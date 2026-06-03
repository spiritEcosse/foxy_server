#include "controllers/AiAnalyzeImage.h"
#include "utils/Base64.h"
#include "utils/config.h"
#include "sentry_catcher/sentryHelper.h"

#include <fmt/format.h>
#include <json/json.h>

#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <system_error>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <utility>

using namespace api::v1;
using namespace drogon;

namespace {
    // The default prompt is split around its three placeholder lines (title,
    // description, meta_description) so buildPrompt() can inject per-field size
    // targets inline. PREFIX ends just after the JSON shape's opening brace;
    // SUFFIX begins right after the meta_description line.
    constexpr std::string_view PROMPT_PREFIX =
        R"(You are an assistant that generates social-media-ready metadata for product images.

Look at the attached image and return ONLY a JSON object (no prose, no markdown) with this exact shape:
{
)";

    constexpr std::string_view PROMPT_SUFFIX =
        R"(  "tags": [
    { "title": "<TagName>", "social_media": ["Instagram","Pinterest","Twitter","Facebook","TikTok","YouTube"] }
  ]
}

Tag rules per platform:
- Instagram: up to 30 tags total may include Instagram in social_media
- Pinterest: 5-7 tags may include Pinterest
- Twitter: 3-5 tags may include Twitter
- Facebook: 5-10 tags may include Facebook
- TikTok: 3-8 tags may include TikTok
- YouTube: 5-15 tags may include YouTube

A single tag can be reused for multiple platforms (list every platform it suits in its social_media array).
Tag titles must be a single CamelCase or PascalCase word without spaces or punctuation.
Return strictly valid JSON. Do not wrap in code fences.)";

    // Optional per-request character-count limits for the generated fields.
    // A value of 0 means "not set" → fall back to the default prompt wording.
    struct SizeOverrides {
        int title = 0;
        int description = 0;
        int metaDescription = 0;
    };

    // Assembles the default prompt with per-field size targets injected inline
    // into the three JSON-shape placeholders. A field with size > 0 uses the
    // "approximately N characters" target wording (enabling shrink AND expand);
    // an unset field (0) keeps its natural default wording.
    std::string buildPrompt(const SizeOverrides &sizes) {
        const std::string title =
            sizes.title > 0 ? fmt::format(R"(  "title": "<product title, approximately {} characters>",)", sizes.title)
                            : std::string(R"(  "title": "<short product title, 5-12 words>",)");
        const std::string description =
            sizes.description > 0
                ? fmt::format(R"(  "description": "<engaging product description, approximately {} characters>",)",
                              sizes.description)
                : std::string(R"(  "description": "<2-4 sentence engaging product description>",)");
        const std::string metaDescription =
            sizes.metaDescription > 0
                ? fmt::format(R"(  "meta_description": "<SEO meta description, approximately {} characters>",)",
                              sizes.metaDescription)
                : std::string(R"(  "meta_description": "<SEO meta description, max 160 chars>",)");

        return fmt::format("{}{}\n{}\n{}\n{}", PROMPT_PREFIX, title, description, metaDescription, PROMPT_SUFFIX);
    }

    // Reads an optional character-count override from the body and validates it
    // against the field's [min, max] range. Non-integral, below-min, or above-max
    // values are treated as unset (return 0 → default prompt wording for the field).
    int readSize(const Json::Value &body, const char *key, int min, int max) {
        const Json::Value &v = body[key];
        if(!v.isIntegral())
            return 0;
        const int n = v.asInt();
        return (n >= min && n <= max) ? n : 0;
    }

    // Reads the optional user-supplied prompt from the body. Non-string or empty
    // values yield an empty string (treated as unset).
    std::string readExtraPrompt(const Json::Value &body) {
        const Json::Value &v = body["extra_prompt"];
        return v.isString() ? v.asString() : std::string{};
    }

    // Builds the block for the caller-supplied prompt, appended after the size
    // overrides. Returns an empty string when no extra prompt was provided.
    std::string extraPromptBlock(const std::string &extraPrompt) {
        if(extraPrompt.empty())
            return {};
        return fmt::format("\n\nAdditional instructions from the request: {}", extraPrompt);
    }

    Json::Value errorJson(const std::string &error, const std::string &detail = {}) {
        Json::Value json;
        json["error"] = error;
        if(!detail.empty())
            json["detail"] = detail;
        return json;
    }

    HttpResponsePtr makeError(HttpStatusCode code, const std::string &error, const std::string &detail = {}) {
        auto resp = HttpResponse::newHttpJsonResponse(errorJson(error, detail));
        resp->setStatusCode(code);
        return resp;
    }

    struct StringHash {
        using is_transparent = void;

        std::size_t operator()(std::string_view s) const noexcept {
            return std::hash<std::string_view>{}(s);
        }
    };

    std::string extensionFor(std::string_view mimeType) {
        static const std::unordered_map<std::string, std::string, StringHash, std::equal_to<>> map{
            {"image/jpeg", "jpg"},
            {"image/jpg", "jpg"},
            {"image/png", "png"},
            {"image/webp", "webp"}};
        if(const auto it = map.find(mimeType); it != map.end())
            return it->second;
        return {};
    }

    // Single-quote shell escape: wrap in '...' and turn embedded ' into '\''.
    std::string shellQuote(std::string_view in) {
        std::string out;
        out.reserve(in.size() + 2);
        out.push_back('\'');
        for(const char c: in) {
            if(c == '\'')
                out.append("'\\''");
            else
                out.push_back(c);
        }
        out.push_back('\'');
        return out;
    }

    // Owns a per-request temp directory; removes it (and the image inside) on destruction.
    // Per-request isolation: claude --add-dir sees only this directory, not all of /tmp,
    // so concurrent requests cannot access each other's images.
    class TempDirGuard {
    public:
        TempDirGuard() = default;

        explicit TempDirGuard(std::string dir) : dir_(std::move(dir)) {}

        TempDirGuard(const TempDirGuard &) = delete;
        TempDirGuard &operator=(const TempDirGuard &) = delete;

        TempDirGuard(TempDirGuard &&other) noexcept : dir_(std::move(other.dir_)) {
            other.dir_.clear();
        }

        TempDirGuard &operator=(TempDirGuard &&other) noexcept {
            if(this != &other) {
                cleanup();
                dir_ = std::move(other.dir_);
                other.dir_.clear();
            }
            return *this;
        }

        ~TempDirGuard() {
            cleanup();
        }

        [[nodiscard]] const std::string &dir() const noexcept {
            return dir_;
        }

    private:
        std::string dir_;

        void cleanup() const noexcept {
            if(!dir_.empty()) {
                std::error_code ec;
                std::filesystem::remove_all(dir_, ec);
            }
        }
    };

    struct TempImage {
        TempDirGuard guard;
        std::string filePath;
    };

    // Create a unique per-request directory under the configured working dir, write
    // the decoded image inside it. Returns an empty TempImage on failure. The directory
    // is created via mkdtemp with 0700 perms, so even on a shared base path, only this
    // process's user can read the contents. The base is overridable via the
    // FOXY_AI_WORKDIR env var so deployments can point at a dedicated private directory
    // (e.g. /var/lib/foxy/tmp) instead of the world-writable /tmp default.
    TempImage writeTempImage(std::string_view bytes, std::string_view ext) {
        // S5443: /tmp default is acceptable here — mkdtemp() below creates a 0700
        // subdir, so the publicly-writable base does not grant access to the image.
        const std::string base = getEnv("FOXY_AI_WORKDIR", "/tmp");  // NOSONAR(cpp:S5443)
        std::string templ = fmt::format("{}/foxy_ai_XXXXXX", base);
        const char *dirPath = ::mkdtemp(templ.data());
        if(!dirPath)
            return {};

        TempDirGuard guard(dirPath);
        const std::string filePath = fmt::format("{}/image.{}", dirPath, ext);
        const int fd = ::open(filePath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
        if(fd == -1)
            return {};

        const auto want = static_cast<ssize_t>(bytes.size());
        const ssize_t wrote = ::write(fd, bytes.data(), bytes.size());
        ::close(fd);
        if(wrote != want)
            return {};

        return TempImage{std::move(guard), filePath};
    }

    void runClaudeAndRespond(const std::shared_ptr<std::function<void(const HttpResponsePtr &)>> &callbackPtr,
                             TempDirGuard guard,
                             const std::string &imagePath,
                             const SizeOverrides &sizes,
                             const std::string &extraPrompt) {
        const std::string &dir = guard.dir();

        const std::string fullPrompt = fmt::format(
            "{}{}\n\nUse the Read tool to load the image at {} and then analyze it. Return ONLY the JSON object — "
            "no prose, no markdown fences, no explanation.",
            buildPrompt(sizes),
            extraPromptBlock(extraPrompt),
            imagePath);

        const std::string cmd =
            fmt::format("claude --add-dir {} --print {} 2>&1", shellQuote(dir), shellQuote(fullPrompt));

        FILE *pipe = ::popen(cmd.c_str(), "r");
        if(!pipe) {
            sentryHelper(std::runtime_error("popen failed for claude CLI"), "AiAnalyzeImage::analyze");
            (*callbackPtr)(makeError(k502BadGateway, "claude CLI failed", "popen failed"));
            return;
        }

        std::string output;
        std::array<char, 4096> buf{};
        while(const auto n = std::fread(buf.data(), 1, buf.size(), pipe))
            output.append(buf.data(), n);

        const int status = ::pclose(pipe);
        const int exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

        if(exitCode != 0 || output.empty()) {
            const std::string tail = output.size() > 500 ? output.substr(output.size() - 500) : output;
            sentryHelper(std::runtime_error(fmt::format("claude CLI exit {}: {}", exitCode, tail)),
                         "AiAnalyzeImage::analyze");
            (*callbackPtr)(makeError(k502BadGateway, "claude CLI failed", fmt::format("exit {}: {}", exitCode, tail)));
            return;
        }

        Json::Value parsed;
        if(Json::Reader reader; !reader.parse(output, parsed)) {
            const std::string head = output.size() > 500 ? output.substr(0, 500) : output;
            (*callbackPtr)(makeError(k502BadGateway, "Invalid JSON from claude", head));
            return;
        }

        auto resp = HttpResponse::newHttpJsonResponse(std::move(parsed));
        resp->setStatusCode(k200OK);
        (*callbackPtr)(resp);
    }
}  // namespace

void AiAnalyzeImage::analyze(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) const {
    if(!req->bodyLength()) {
        callback(makeError(k400BadRequest, "Empty body"));
        return;
    }

    const auto jsonPtr = req->getJsonObject();
    if(!jsonPtr) {
        callback(makeError(k400BadRequest, "Invalid JSON body"));
        return;
    }

    const Json::Value &body = *jsonPtr;
    const std::string image = body.get("image", "").asString();
    const std::string mimeType = body.get("mimeType", "").asString();

    if(image.empty() || mimeType.empty()) {
        callback(makeError(k400BadRequest, "Missing image or mimeType"));
        return;
    }

    const std::string ext = extensionFor(mimeType);
    if(ext.empty()) {
        callback(makeError(k400BadRequest, "Unsupported mimeType", mimeType));
        return;
    }

    const SizeOverrides sizes{.title = readSize(body, "title_size", 10, 60),
                              .description = readSize(body, "description_size", 50, 1000),
                              .metaDescription = readSize(body, "meta_description_size", 50, 160)};
    const std::string extraPrompt = readExtraPrompt(body);

    const std::string decoded = Base64::Decode(image);
    if(decoded.empty()) {
        callback(makeError(k400BadRequest, "Failed to decode base64 image"));
        return;
    }

    TempImage temp = writeTempImage(decoded, ext);
    if(temp.filePath.empty()) {
        callback(makeError(k500InternalServerError, "Failed to write temp file", std::strerror(errno)));
        return;
    }

    auto callbackPtr = std::make_shared<std::function<void(const HttpResponsePtr &)>>(std::move(callback));

    // Run blocking popen call off the event loop. Class-static jthread list
    // keeps threads owned (not detached → satisfies cpp:S5962). Statics are
    // class members (in the header) so dynamic init in this TU also forces
    // HttpController<T>::registrator_ to initialize at startup.
    std::lock_guard lock(workerMutex);
    workerThreads.emplace_back(
        [callbackPtr, guard = std::move(temp.guard), path = temp.filePath, sizes, extraPrompt]() mutable {
            runClaudeAndRespond(callbackPtr, std::move(guard), path, sizes, extraPrompt);
        });
}
