# CORS Headers on 404/Error Responses Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ensure CORS headers (`Access-Control-Allow-Origin`) are present on all Drogon-generated error responses (404, 405, 500, etc.) and on OPTIONS preflights to non-existent routes, so browsers surface the real HTTP status instead of a misleading CORS error.

**Architecture:** Replace `registerPostHandlingAdvice` (which only fires for matched routes) with `setCustomErrorHandler` to inject CORS headers on framework-generated errors. Add a catch-all OPTIONS handler via `registerSyncAdvice` at the pre-routing join point to respond to preflight requests before routing, returning 204 with the full CORS preflight headers. Add a standalone test file `TestCors.h` that hits non-existent paths and asserts headers.

**Tech Stack:** C++20, Drogon 1.9.x, Google Test

---

## File Map

- **Modify:** `src/main.cpp` — replace `registerPostHandlingAdvice` with `setCustomErrorHandler`; add sync advice for OPTIONS preflight
- **Modify:** `tests/main.cpp` — add listener on port 18080 and include `TestCors.h`
- **Create:** `tests/TestCors.h` — tests for CORS headers on 404 POST and OPTIONS preflight, using `drogon::HttpClient` against the test listener

---

### Task 1: Write failing tests for CORS on 404

The existing test suite calls controller methods directly, bypassing Drogon's routing pipeline. CORS headers are injected at the pipeline level (sync advice, custom error handler), so tests must go through the full HTTP stack. The test app needs a listener on port 18080 and tests use `drogon::HttpClient`.

**Files:**
- Create: `tests/TestCors.h`
- Modify: `tests/main.cpp`

- [ ] **Step 1: Add listener to `tests/main.cpp`**

In the `DrogonTestEnvironment::SetUp()` method, add a listener before `drogon::app().run()` is called. Find the line `drogon::app().createDbClient(...)` and add after it:

```cpp
drogon::app().addListener("127.0.0.1", 18080);
```

The full block after modification:
```cpp
drogon::app()
    .createDbClient("postgresql", "/var/run/postgresql", 5432, pgDb, pgUser, "", 1, "", "default", true);
drogon::app().addListener("127.0.0.1", 18080);
```

- [ ] **Step 2: Create `tests/TestCors.h`**

```cpp
#pragma once

#include "drogon/drogon.h"
#include "drogon/HttpClient.h"
#include "utils/config.h"
#include <gtest/gtest.h>
#include <future>

class CorsTest : public ::testing::Test {
protected:
    void SetUp() override {
        foxyClient = api::v1::getEnv("FOXY_CLIENT", "http://localhost:5173");
        client = drogon::HttpClient::newHttpClient("http://127.0.0.1:18080");
    }
    std::string foxyClient;
    drogon::HttpClientPtr client;

    std::function<void(drogon::ReqResult, const drogon::HttpResponsePtr&)>
    makeCallback(std::shared_ptr<std::promise<void>> promise,
                 drogon::HttpStatusCode expectedStatus,
                 bool expectCorsHeader,
                 bool expectPreflightHeaders = false) {
        return [this, promise, expectedStatus, expectCorsHeader, expectPreflightHeaders]
               (drogon::ReqResult result, const drogon::HttpResponsePtr& resp) {
            try {
                ASSERT_EQ(result, drogon::ReqResult::Ok);
                EXPECT_EQ(resp->getStatusCode(), expectedStatus);
                if(expectCorsHeader) {
                    EXPECT_EQ(resp->getHeader("Access-Control-Allow-Origin"), foxyClient);
                } else {
                    EXPECT_TRUE(resp->getHeader("Access-Control-Allow-Origin").empty());
                }
                if(expectPreflightHeaders) {
                    EXPECT_FALSE(resp->getHeader("Access-Control-Allow-Methods").empty());
                    EXPECT_FALSE(resp->getHeader("Access-Control-Allow-Headers").empty());
                }
                promise->set_value();
            } catch(...) {
                promise->set_exception(std::current_exception());
            }
        };
    }
};

TEST_F(CorsTest, NotFoundPostHasCorsHeader) {
    auto promise = std::make_shared<std::promise<void>>();
    auto future = promise->get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/v1/does-not-exist");
    req->addHeader("Origin", foxyClient);

    client->sendRequest(req, makeCallback(promise, drogon::k404NotFound, true));
    future.get();
}

TEST_F(CorsTest, OptionsPreflightNotFoundReturns204WithCorsHeaders) {
    auto promise = std::make_shared<std::promise<void>>();
    auto future = promise->get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Options);
    req->setPath("/api/v1/does-not-exist");
    req->addHeader("Origin", foxyClient);

    client->sendRequest(req, makeCallback(promise, drogon::k204NoContent, true, true));
    future.get();
}

TEST_F(CorsTest, UnknownOriginGetsNoCorsHeader) {
    auto promise = std::make_shared<std::promise<void>>();
    auto future = promise->get_future();

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/v1/does-not-exist");
    req->addHeader("Origin", "http://evil.example.com");

    client->sendRequest(req, makeCallback(promise, drogon::k404NotFound, false));
    future.get();
}
```

- [ ] **Step 3: Add include to `tests/main.cpp`**

Add `#include "TestCors.h"` after `#include "TestAiAnalyzeImage.h"`:

```cpp
#include "TestCors.h"
```

- [ ] **Step 4: Build and confirm tests fail**

```bash
cmake --preset ninja-debug && cmake --build --preset ninja-debug 2>&1 | tail -20
```

Expected: build succeeds, then run tests:

```bash
ctest --preset ninja-debug --output-on-failure -R CorsTest 2>&1 | tail -30
```

Expected: `CorsTest.NotFoundPostHasCorsHeader` FAILS (no `Access-Control-Allow-Origin` header), `CorsTest.OptionsPreflightNotFoundReturns204WithCorsHeaders` FAILS (gets 404 not 204).

---

### Task 2: Implement CORS on error responses and OPTIONS preflight

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Replace CORS logic in `src/main.cpp`**

Replace the entire `addCorsHeader` function and `registerRoutes()` in `src/main.cpp`. The full updated anonymous namespace block (everything between `namespace {` and `}  // namespace`) should look like:

```cpp
namespace {
    void sendJson(std::function<void(const HttpResponsePtr &)> &&callback, Json::Value &&json) {
        auto resp = HttpResponse::newHttpJsonResponse(std::move(json));
        auto callbackPtr = std::make_shared<std::function<void(const HttpResponsePtr &)>>(std::move(callback));
        (*callbackPtr)(resp);
    }

    void rootHandler(const HttpRequestPtr &, std::function<void(const HttpResponsePtr &)> &&callback) {
        Json::Value json;
        json["result"] = "ok";
        json["message"] = "hello,world!";
        sendJson(std::move(callback), std::move(json));
    }

    void testHandler(const HttpRequestPtr &,
                     std::function<void(const HttpResponsePtr &)> &&callback,
                     const std::string &name) {
        Json::Value json;
        json["result"] = "ok";
        json["message"] = "hello," + name;
        sendJson(std::move(callback), std::move(json));
    }

#if defined(SENTRY_DSN)
    void sentryHandler(const HttpRequestPtr &, std::function<void(const HttpResponsePtr &)> &&callback) {
        sentryHelper("It works!", "custom");
        Json::Value json;
        json["result"] = "ok";
        json["message"] = "sentry, checked! (SENTRY_LEVEL_INFO, custom, It works!)";
        sendJson(std::move(callback), std::move(json));
    }

    void initSentry() {
        sentry_options_t *options = sentry_options_new();
        sentry_options_set_dsn(options, SENTRY_DSN);
        sentry_options_set_handler_path(
            options,
            fmt::format("{}/_deps/sentry-build/crashpad_build/handler/crashpad_handler", CMAKE_BINARY_DIR).c_str());
        sentry_options_set_release(options, "faithfishart-server@0.0.1");
        sentry_options_set_debug(options, 1);
        sentry_init(options);
    }
#endif

    bool isAllowedOrigin(const std::string &origin) {
        return !origin.empty() &&
               (origin == api::v1::getEnv("FOXY_CLIENT", "") || origin == api::v1::getEnv("FOXY_ADMIN", ""));
    }

    void attachCorsHeaders(const HttpRequestPtr &req, const HttpResponsePtr &resp) {
        const auto origin = req->getHeader("Origin");
        if(isAllowedOrigin(origin))
            resp->addHeader("Access-Control-Allow-Origin", origin);
    }

    void registerRoutes() {
        app().registerHandler("/", &rootHandler);
        app().registerHandler("/test?username={name}", &testHandler);
#if defined(SENTRY_DSN)
        app().registerHandler("/sentry", &sentryHandler);
#endif

        // Inject CORS on matched-route responses
        app().registerPostHandlingAdvice(&attachCorsHeaders);

        // Inject CORS on framework-generated errors (404, 405, 500, …)
        app().setCustomErrorHandler([](HttpStatusCode code, const HttpRequestPtr &req) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(code);
            attachCorsHeaders(req, resp);
            return resp;
        });

        // Handle OPTIONS preflight for any path (including non-existent routes)
        // Sync advice fires before routing, so it intercepts OPTIONS before a 404 is generated.
        app().registerSyncAdvice([](const HttpRequestPtr &req) -> HttpResponsePtr {
            if(req->getMethod() != HttpMethod::Options)
                return nullptr;
            const auto origin = req->getHeader("Origin");
            if(!isAllowedOrigin(origin))
                return nullptr;
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k204NoContent);
            resp->addHeader("Access-Control-Allow-Origin", origin);
            resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS");
            resp->addHeader("Access-Control-Allow-Headers", "Authorization, Content-Type");
            resp->addHeader("Access-Control-Max-Age", "86400");
            return resp;
        });
    }

    void setupDb() {
        const auto pgDb = api::v1::getEnv("PG_DB", "foxy");
        const auto pgUser = api::v1::getEnv("PG_USER", "foxy");
        app().createDbClient("postgresql", "/var/run/postgresql", 5432, pgDb, pgUser, "", 1, "", "default", true);
    }
}  // namespace
```

- [ ] **Step 2: Verify `registerSyncAdvice` exists in the Drogon API**

```bash
grep -n "registerSyncAdvice" /home/ihor/projects/foxy_server/foxy_server_dev/.cache/CPM/drogon/fd852dfd97e65bafa7b071babd59d67301f60a06/lib/inc/drogon/HttpAppFramework.h
```

Expected: a line showing `registerSyncAdvice` declaration. If it does NOT appear, use the pre-routing advice alternative — see Note below.

> **Note (fallback if `registerSyncAdvice` is unavailable):** Use `registerPreRoutingAdvice` instead:
> ```cpp
> app().registerPreRoutingAdvice([](const HttpRequestPtr &req,
>                                   AdviceCallback &&stop,
>                                   AdviceChainCallback &&next) {
>     if(req->getMethod() != HttpMethod::Options) return next();
>     const auto origin = req->getHeader("Origin");
>     if(!isAllowedOrigin(origin)) return next();
>     auto resp = HttpResponse::newHttpResponse();
>     resp->setStatusCode(k204NoContent);
>     resp->addHeader("Access-Control-Allow-Origin", origin);
>     resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS");
>     resp->addHeader("Access-Control-Allow-Headers", "Authorization, Content-Type");
>     resp->addHeader("Access-Control-Max-Age", "86400");
>     stop(resp);
> });
> ```
> The signature for `registerPreRoutingAdvice` is:
> `void(const HttpRequestPtr&, AdviceCallback&&, AdviceChainCallback&&)`

- [ ] **Step 3: Build**

```bash
cmake --preset ninja-debug && cmake --build --preset ninja-debug 2>&1 | tail -20
```

Expected: build completes with no errors.

---

### Task 3: Run tests and verify

**Files:** none (read-only verification step)

- [ ] **Step 1: Run CORS tests**

```bash
ctest --preset ninja-debug --output-on-failure -R CorsTest 2>&1 | tail -30
```

Expected: all three `CorsTest` tests PASS.

- [ ] **Step 2: Run full test suite to check for regressions**

```bash
ctest --preset ninja-debug --output-on-failure 2>&1 | tail -40
```

Expected: all previously passing tests still pass.

- [ ] **Step 3: Smoke-test with curl against a running server**

```bash
# Rebuild and restart the server, then:
curl -i -X OPTIONS -H "Origin: http://localhost:5173" http://localhost:8080/api/v1/does-not-exist 2>&1 | grep -E "HTTP|Access-Control"
curl -i -X POST   -H "Origin: http://localhost:5173" http://localhost:8080/api/v1/does-not-exist 2>&1 | grep -E "HTTP|Access-Control"
```

Expected first command:
```
HTTP/1.1 204 No Content
Access-Control-Allow-Origin: http://localhost:5173
Access-Control-Allow-Methods: GET, POST, PUT, PATCH, DELETE, OPTIONS
Access-Control-Allow-Headers: Authorization, Content-Type
```

Expected second command:
```
HTTP/1.1 404 Not Found
Access-Control-Allow-Origin: http://localhost:5173
```

---

### Task 4: Commit and create PR

**Files:** none (git operations)

- [ ] **Step 1: Create branch for issue #94**

```bash
git checkout -b issue-94-cors-on-404-errors
```

- [ ] **Step 2: Stage and commit**

```bash
git add src/main.cpp tests/TestCors.h tests/main.cpp
git commit -m "$(cat <<'EOF'
fix: add CORS headers to 404/error responses and OPTIONS preflights

Fixes a debugging trap where unmatched routes caused browsers to show a
CORS error instead of the real 404. Uses setCustomErrorHandler for
framework-generated errors and a pre-routing sync advice for OPTIONS
preflights on non-existent paths.

Closes #94
EOF
)"
```

- [ ] **Step 3: Push and create PR**

```bash
git push -u origin issue-94-cors-on-404-errors
gh pr create --base dev --title "fix: CORS headers on 404/error responses and OPTIONS preflights" --body "$(cat <<'EOF'
## Summary

- Replace bare `registerPostHandlingAdvice` with `setCustomErrorHandler` so CORS headers are injected on all framework-generated error responses (404, 405, 500, …)
- Add a sync pre-routing advice that intercepts `OPTIONS` requests before routing — returns `204 No Content` with full preflight headers for known origins
- Extract `isAllowedOrigin()` helper to avoid repeating the env-var check
- Add `TestCors.h` with three tests: 404 POST gets origin header, OPTIONS preflight to non-existent route gets 204 + headers, unknown origin gets no header

Closes #94
EOF
)"
```
