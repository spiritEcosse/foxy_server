#include "IClientImpl.h"
#include "fmt/format.h"
#include <string>
#include <random>
#include <openssl/buffer.h>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>
#include <map>
#include <drogon/drogon.h>
#include <fstream>
#include "HttpException.h"
#include <functional>
#include <memory>
#include <uuid/uuid.h>

namespace api::v1 {
    bool IClientImpl::fieldIsMember(const std::string& field,
                                    const cpr::Response& response,
                                    const Json::Value& jsonResponse) {
        if(!jsonResponse.isMember(field)) {
            // Capture JSON parsing error
            sentryHelper(
                std::runtime_error(fmt::format("No data or id in PostType to post. Url: {}, Status: {}, Response: {}.",
                                               response.url.str(),
                                               response.status_code,
                                               response.text)),
                "IClientImpl::fieldIsMember");
            return false;
        }
        return true;
    }

    bool IClientImpl::parseJson(const cpr::Response& response, Json::Value& jsonResponse) {
        if(Json::Reader reader; !reader.parse(response.text, jsonResponse)) {
            // Capture JSON parsing error
            sentryHelper(std::runtime_error(fmt::format("JSON parsing error: {}. Url: {}, Status: {}, Response: {}.",
                                                        reader.getFormattedErrorMessages(),
                                                        response.url.str(),
                                                        response.status_code,
                                                        response.text)),
                         "IClientImpl::parseJson");
            return false;
        }
        return true;
    }

    bool IClientImpl::checkResponses(const std::vector<cpr::Response>& responses, int status_code) {
        if(!std::ranges::all_of(responses, [status_code](const cpr::Response& response) {
               return response.status_code == status_code;
           })) {
            std::vector<std::string> errorMessages;
            std::ranges::transform(responses,
                                   std::back_inserter(errorMessages),
                                   [status_code](const cpr::Response& response) {
                                       return response.status_code != status_code
                                                  ? fmt::format("Url: {}, Status: {}, Response: {}\n",
                                                                response.url.str(),
                                                                response.status_code,
                                                                response.text)
                                                  : "";
                                   });
            sentryHelper(std::runtime_error(fmt::format("{}", fmt::join(errorMessages, ""))),
                         "IClientImpl::checkResponses");
            return false;
        }
        return true;
    }

    std::string IClientImpl::urlEncode(const std::string_view value) {
        std::string escaped;
        escaped.reserve(value.size());
        for(const char c: value) {
            if(isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped += c;
            } else {
                escaped += fmt::format("%{:02X}", static_cast<unsigned int>(static_cast<unsigned char>(c)));
            }
        }
        return escaped;
    }

    std::string IClientImpl::calculateOAuthSignature(const std::string_view httpMethod,
                                                     const std::string_view url,
                                                     const std::map<std::string, std::string, std::less<>>& params,
                                                     const std::string_view consumerSecret,
                                                     const std::string_view tokenSecret) {
        // Step 1-5: Create the signature base string
        std::vector<std::string> encodedParams;
        std::ranges::transform(params, std::back_inserter(encodedParams), [](const auto& pair) {
            return fmt::format("{}={}", urlEncode(pair.first), urlEncode(pair.second));
        });
        const std::string paramString = fmt::to_string(fmt::join(encodedParams, "&"));

        const std::string signatureBaseString =
            fmt::format("{}&{}&{}", urlEncode(httpMethod), urlEncode(url), urlEncode(paramString));

        // Step 6: Generate the signing key
        const std::string signingKey = fmt::format("{}&{}", urlEncode(consumerSecret), urlEncode(tokenSecret));

        // Step 7-8: Sign the base string using HMAC-SHA1 and encode in base64
        unsigned int digest_len;
        const unsigned char* digest = HMAC(EVP_sha1(),
                                           signingKey.c_str(),
                                           static_cast<int>(signingKey.size()),
                                           reinterpret_cast<const unsigned char*>(signatureBaseString.c_str()),
                                           signatureBaseString.length(),
                                           nullptr,
                                           &digest_len);

        // Convert the HMAC digest into base64
        BIO* bmem = BIO_new(BIO_s_mem());
        BIO* b64 = BIO_new(BIO_f_base64());
        bmem = BIO_push(b64, bmem);

        BIO_write(bmem, digest, static_cast<int>(digest_len));
        BIO_flush(bmem);

        BUF_MEM* bptr;
        BIO_get_mem_ptr(bmem, &bptr);

        std::string oauthSignature(bptr->data, bptr->length - 1);  // Exclude the null terminator added by BIO

        BIO_free_all(bmem);

        return oauthSignature;
    }

}
