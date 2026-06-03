# Inline Size Injection Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the appended "Field size overrides" block in `AiAnalyzeImage` with direct inline injection of size targets into the prompt's JSON-shape placeholders, with per-field min/max validation and silent fallback to default wording.

**Architecture:** The single `DEFAULT_PROMPT` constant is split around its three placeholder lines into a fixed prefix and a fixed suffix. A new `buildPrompt(const SizeOverrides &)` assembles the full default prompt, choosing per field between override wording (`approximately N characters`) and default wording. `readSize` gains per-field `min`/`max` bounds; out-of-range values become `0` (unset). `sizeOverrideBlock()` and the `CLAUDE_ANALYZE_IMAGE_PROMPT` env override are deleted. `extra_prompt` handling is untouched.

**Tech Stack:** C++20, Drogon, fmt, jsoncpp, Google Test. Build via CMake presets (`ninja-debug`), tests via `ctest`. Tests drive the real `analyze()` endpoint through a stub `claude` script on `PATH` that records the exact prompt.

---

## File Structure

- `src/code/controllers/AiAnalyzeImage.cpp` — the only production file changed. The anonymous namespace gains `buildPrompt`, changes `readSize`'s signature, and loses `sizeOverrideBlock`. `runClaudeAndRespond` switches from `getEnv(...) + sizeOverrideBlock(sizes)` to `buildPrompt(sizes)`. The call site in `analyze()` passes per-field bounds to `readSize`.
- `tests/TestAiAnalyzeImage.h` — rewrite the size-override tests to the new inline wording, keep the fallback tests (updated assertions), and add per-field bounds tests.

No new files. No header changes (`SizeOverrides` shape is unchanged; everything stays in the anonymous namespace).

---

## Reference: target wording

When a field **is** overridden (in range), its placeholder line becomes exactly:

```
  "title": "<product title, approximately N characters>",
  "description": "<engaging product description, approximately N characters>",
  "meta_description": "<SEO meta description, approximately N characters>",
```

When a field is **not** overridden, it keeps its original default wording:

```
  "title": "<short product title, 5-12 words>",
  "description": "<2-4 sentence engaging product description>",
  "meta_description": "<SEO meta description, max 160 chars>",
```

Per-field bounds (inclusive):

| Field            | body key                 | min | max  |
|------------------|--------------------------|-----|------|
| title            | `title_size`             | 10  | 60   |
| description      | `description_size`       | 50  | 1000 |
| meta_description | `meta_description_size`  | 50  | 160  |

---

## Task 1: Add per-field bounds to `readSize`

**Files:**
- Test: `tests/TestAiAnalyzeImage.h` (rewrite existing fallback tests + add bounds tests)
- Modify: `src/code/controllers/AiAnalyzeImage.cpp:72-78` (`readSize`) and call site `:323-325`

This task changes `readSize`'s signature and call site but does NOT yet change the prompt wording. To keep the build green between tasks, we change `sizeOverrideBlock` only minimally is NOT needed — instead, this task is done together with Task 2's wording because the tests assert wording. **Therefore Task 1 only writes the bounds tests and the `readSize` change; the wording switch in `buildPrompt` lands in Task 2. The intermediate state will have failing wording tests until Task 2 — that is expected and noted per step.**

> NOTE: Because the existing tests assert the OLD `"Field size overrides"` wording, this plan rewrites the whole `.cpp` and the whole test block as one coherent change across Tasks 1–3, each committed once green. Do the steps in order; do not run the full suite expecting green until the end of Task 3.

- [ ] **Step 1: Change `readSize` to enforce per-field bounds**

Replace the existing `readSize` (lines 70-78) with:

```cpp
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
```

- [ ] **Step 2: Update the call site in `analyze()` to pass bounds**

Replace lines 323-325:

```cpp
    const SizeOverrides sizes{.title = readSize(body, "title_size", 10, 60),
                              .description = readSize(body, "description_size", 50, 1000),
                              .metaDescription = readSize(body, "meta_description_size", 50, 160)};
```

(Proceed directly to Task 2 — do not build yet; `sizeOverrideBlock` still references the old wording the tests will be rewritten away from.)

---

## Task 2: Split `DEFAULT_PROMPT` and add `buildPrompt`; delete `sizeOverrideBlock` and the env override

**Files:**
- Modify: `src/code/controllers/AiAnalyzeImage.cpp` — `DEFAULT_PROMPT` (lines 33-56), `sizeOverrideBlock` (lines 80-97, delete), `runClaudeAndRespond` (lines 248-255)

- [ ] **Step 1: Replace `DEFAULT_PROMPT` with prefix/suffix constants**

Replace the `DEFAULT_PROMPT` definition (lines 33-56) with the split prefix and suffix:

```cpp
    // The default prompt is split around its three placeholder lines (title,
    // description, meta_description) so buildPrompt() can inject per-field size
    // targets inline. PREFIX ends just before the JSON shape's opening brace
    // body; SUFFIX begins right after the meta_description line.
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
```

- [ ] **Step 2: Add `buildPrompt` after the `SizeOverrides` struct**

Add this function immediately after the `SizeOverrides` struct (after line 68, before `readSize`). It emits the three placeholder lines between prefix and suffix, choosing override vs default wording per field:

```cpp
    // Assembles the default prompt with per-field size targets injected inline
    // into the three JSON-shape placeholders. A field with size > 0 uses the
    // "approximately N characters" target wording (enabling shrink AND expand);
    // an unset field (0) keeps its natural default wording.
    std::string buildPrompt(const SizeOverrides &sizes) {
        const std::string title = sizes.title > 0
            ? fmt::format(R"(  "title": "<product title, approximately {} characters>",)", sizes.title)
            : R"(  "title": "<short product title, 5-12 words>",)";
        const std::string description = sizes.description > 0
            ? fmt::format(R"(  "description": "<engaging product description, approximately {} characters>",)",
                          sizes.description)
            : R"(  "description": "<2-4 sentence engaging product description>",)";
        const std::string metaDescription = sizes.metaDescription > 0
            ? fmt::format(R"(  "meta_description": "<SEO meta description, approximately {} characters>",)",
                          sizes.metaDescription)
            : R"(  "meta_description": "<SEO meta description, max 160 chars>",)";

        return fmt::format("{}{}\n{}\n{}\n{}", PROMPT_PREFIX, title, description, metaDescription, PROMPT_SUFFIX);
    }
```

- [ ] **Step 3: Delete `sizeOverrideBlock`**

Delete the entire `sizeOverrideBlock` function (originally lines 80-97, including its leading comment lines 80-81).

- [ ] **Step 4: Rewrite `runClaudeAndRespond`'s prompt assembly**

Replace lines 248-255 (the `getEnv(...)` line through the `fmt::format(...)` that built `fullPrompt`) with:

```cpp
        const std::string fullPrompt = fmt::format(
            "{}{}\n\nUse the Read tool to load the image at {} and then analyze it. Return ONLY the JSON object — "
            "no prose, no markdown fences, no explanation.",
            buildPrompt(sizes),
            extraPromptBlock(extraPrompt),
            imagePath);
```

(Note: the old format string had three `{}` for `prompt`, `sizeOverrideBlock(sizes)`, `extraPromptBlock(extraPrompt)`. The new one has two: `buildPrompt(sizes)` and `extraPromptBlock(extraPrompt)`.)

- [ ] **Step 5: Verify the `<string_view>` include and `getEnv` usage**

`getEnv` is still used by `writeTempImage` (`FOXY_AI_WORKDIR`), so the `utils/config.h` include stays. `std::string_view` is still used (`PROMPT_PREFIX`/`PROMPT_SUFFIX`, `shellQuote`, `extensionFor`), so its include stays. No include changes needed. Confirm by grep:

Run: `grep -n "getEnv\|string_view" src/code/controllers/AiAnalyzeImage.cpp`
Expected: `getEnv` appears in `writeTempImage`; `string_view` appears in includes and several functions. No reference to `CLAUDE_ANALYZE_IMAGE_PROMPT` remains.

Run: `grep -n "CLAUDE_ANALYZE_IMAGE_PROMPT\|sizeOverrideBlock\|DEFAULT_PROMPT" src/code/controllers/AiAnalyzeImage.cpp`
Expected: no output (all three are gone).

---

## Task 3: Rewrite and extend the tests, then build and run green

**Files:**
- Modify: `tests/TestAiAnalyzeImage.h` — rewrite the four size tests, update the two fallback tests, add bounds tests.

- [ ] **Step 1: Rewrite `NoSizeOverridesOmitsBlock`**

Replace the test at lines 176-180 with (rename to reflect new behavior):

```cpp
TEST_F(AiAnalyzeImageTest, NoSizeOverridesKeepsDefaultWording) {
    const std::string prompt = promptFor(validBody());
    EXPECT_FALSE(prompt.empty());
    EXPECT_EQ(prompt.find("approximately"), std::string::npos);
    EXPECT_NE(prompt.find("short product title, 5-12 words"), std::string::npos);
    EXPECT_NE(prompt.find("2-4 sentence engaging product description"), std::string::npos);
    EXPECT_NE(prompt.find("SEO meta description, max 160 chars"), std::string::npos);
}
```

- [ ] **Step 2: Rewrite `DescriptionSizeOverrideInjected`**

Replace the test at lines 182-191 with:

```cpp
TEST_F(AiAnalyzeImageTest, DescriptionSizeOverrideInjected) {
    Json::Value body = validBody();
    body["description_size"] = 300;
    const std::string prompt = promptFor(body);
    EXPECT_NE(prompt.find("engaging product description, approximately 300 characters"), std::string::npos);
    // Other fields keep their default wording.
    EXPECT_NE(prompt.find("short product title, 5-12 words"), std::string::npos);
    EXPECT_NE(prompt.find("SEO meta description, max 160 chars"), std::string::npos);
}
```

- [ ] **Step 3: Rewrite `AllSizeOverridesInjected`**

Replace the test at lines 193-202 with (note `description_size` 300 stays in [50,1000]; `meta_description_size` lowered to 150 to stay in [50,160]):

```cpp
TEST_F(AiAnalyzeImageTest, AllSizeOverridesInjected) {
    Json::Value body = validBody();
    body["title_size"] = 60;
    body["description_size"] = 300;
    body["meta_description_size"] = 150;
    const std::string prompt = promptFor(body);
    EXPECT_NE(prompt.find("product title, approximately 60 characters"), std::string::npos);
    EXPECT_NE(prompt.find("engaging product description, approximately 300 characters"), std::string::npos);
    EXPECT_NE(prompt.find("SEO meta description, approximately 150 characters"), std::string::npos);
    // No default wording remains for the overridden fields.
    EXPECT_EQ(prompt.find("5-12 words"), std::string::npos);
    EXPECT_EQ(prompt.find("2-4 sentence"), std::string::npos);
    EXPECT_EQ(prompt.find("max 160 chars"), std::string::npos);
}
```

- [ ] **Step 4: Update `NonPositiveSizeOverridesIgnored`**

Replace the test at lines 204-210 with:

```cpp
TEST_F(AiAnalyzeImageTest, NonPositiveSizeOverridesIgnored) {
    Json::Value body = validBody();
    body["title_size"] = 0;
    body["description_size"] = -10;
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("approximately"), std::string::npos);
    EXPECT_NE(prompt.find("short product title, 5-12 words"), std::string::npos);
    EXPECT_NE(prompt.find("2-4 sentence engaging product description"), std::string::npos);
}
```

- [ ] **Step 5: Update `NonNumericSizeOverrideIgnored`**

Replace the test at lines 212-217 with:

```cpp
TEST_F(AiAnalyzeImageTest, NonNumericSizeOverrideIgnored) {
    Json::Value body = validBody();
    body["description_size"] = "not a number";
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("approximately"), std::string::npos);
    EXPECT_NE(prompt.find("2-4 sentence engaging product description"), std::string::npos);
}
```

- [ ] **Step 6: Rework `ExtraPromptAppearsAfterSizeOverrides`**

Replace the test at lines 247-257 with (now anchors on the inline override text instead of the deleted block):

```cpp
TEST_F(AiAnalyzeImageTest, ExtraPromptAppearsAfterSizeOverrides) {
    Json::Value body = validBody();
    body["title_size"] = 60;
    body["extra_prompt"] = "Emphasize seasonal colors.";
    const std::string prompt = promptFor(body);
    const auto sizePos = prompt.find("product title, approximately 60 characters");
    const auto extraPos = prompt.find("Additional instructions from the request");
    ASSERT_NE(sizePos, std::string::npos);
    ASSERT_NE(extraPos, std::string::npos);
    EXPECT_LT(sizePos, extraPos);
}
```

- [ ] **Step 7: Add per-field above-max bounds tests**

Add after the `AllSizeOverridesInjected` test:

```cpp
TEST_F(AiAnalyzeImageTest, TitleSizeAboveMaxFallsBack) {
    Json::Value body = validBody();
    body["title_size"] = 200;  // > 60
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("title, approximately"), std::string::npos);
    EXPECT_NE(prompt.find("short product title, 5-12 words"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, DescriptionSizeAboveMaxFallsBack) {
    Json::Value body = validBody();
    body["description_size"] = 5000;  // > 1000
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("description, approximately"), std::string::npos);
    EXPECT_NE(prompt.find("2-4 sentence engaging product description"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, MetaDescriptionSizeAboveMaxFallsBack) {
    Json::Value body = validBody();
    body["meta_description_size"] = 500;  // > 160
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("meta description, approximately"), std::string::npos);
    EXPECT_NE(prompt.find("SEO meta description, max 160 chars"), std::string::npos);
}
```

- [ ] **Step 8: Add per-field below-min bounds tests**

Add after the above-max tests:

```cpp
TEST_F(AiAnalyzeImageTest, TitleSizeBelowMinFallsBack) {
    Json::Value body = validBody();
    body["title_size"] = 3;  // < 10
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("title, approximately"), std::string::npos);
    EXPECT_NE(prompt.find("short product title, 5-12 words"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, DescriptionSizeBelowMinFallsBack) {
    Json::Value body = validBody();
    body["description_size"] = 10;  // < 50
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("description, approximately"), std::string::npos);
    EXPECT_NE(prompt.find("2-4 sentence engaging product description"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, MetaDescriptionSizeBelowMinFallsBack) {
    Json::Value body = validBody();
    body["meta_description_size"] = 10;  // < 50
    const std::string prompt = promptFor(body);
    EXPECT_EQ(prompt.find("meta description, approximately"), std::string::npos);
    EXPECT_NE(prompt.find("SEO meta description, max 160 chars"), std::string::npos);
}
```

- [ ] **Step 9: Add boundary tests (exact min/max inject; just-outside fall back)**

Add after the below-min tests:

```cpp
TEST_F(AiAnalyzeImageTest, TitleSizeBoundariesInject) {
    Json::Value max = validBody();
    max["title_size"] = 60;  // == max
    EXPECT_NE(promptFor(max).find("product title, approximately 60 characters"), std::string::npos);

    Json::Value min = validBody();
    min["title_size"] = 10;  // == min
    EXPECT_NE(promptFor(min).find("product title, approximately 10 characters"), std::string::npos);
}

TEST_F(AiAnalyzeImageTest, TitleSizeJustOutsideBoundariesFallBack) {
    Json::Value over = validBody();
    over["title_size"] = 61;  // max + 1
    EXPECT_EQ(promptFor(over).find("title, approximately"), std::string::npos);

    Json::Value under = validBody();
    under["title_size"] = 9;  // min - 1
    EXPECT_EQ(promptFor(under).find("title, approximately"), std::string::npos);
}
```

- [ ] **Step 10: Add a mixed in-range / out-of-range / unset test**

Add after the boundary tests:

```cpp
TEST_F(AiAnalyzeImageTest, MixedSizeOverridesInjectOnlyInRange) {
    Json::Value body = validBody();
    body["title_size"] = 50;            // in range → inject
    body["description_size"] = 5000;    // out of range → fall back
    // meta_description_size unset → fall back
    const std::string prompt = promptFor(body);
    EXPECT_NE(prompt.find("product title, approximately 50 characters"), std::string::npos);
    EXPECT_NE(prompt.find("2-4 sentence engaging product description"), std::string::npos);
    EXPECT_NE(prompt.find("SEO meta description, max 160 chars"), std::string::npos);
    EXPECT_EQ(prompt.find("description, approximately"), std::string::npos);
    EXPECT_EQ(prompt.find("meta description, approximately"), std::string::npos);
}
```

- [ ] **Step 11: Build**

Run: `cmake --preset ninja-debug && cmake --build --preset ninja-debug 2>&1 | tail -20`
Expected: build succeeds, no warnings-as-errors failures.

- [ ] **Step 12: Run the AiAnalyzeImage tests**

Run: `ctest --preset ninja-debug -R AiAnalyzeImage --output-on-failure`
Expected: all `AiAnalyzeImageTest.*` tests PASS (the existing four endpoint tests plus all rewritten/new prompt tests).

- [ ] **Step 13: Run the full test suite to confirm no regressions**

Run: `ctest --preset ninja-debug --output-on-failure`
Expected: all tests PASS.

- [ ] **Step 14: Format**

Run: `cmake --build build/debug --target clang-format`
Expected: no diff, or only formatting of the touched files.

- [ ] **Step 15: Commit**

```bash
git add src/code/controllers/AiAnalyzeImage.cpp tests/TestAiAnalyzeImage.h docs/superpowers/plans/2026-06-03-inline-size-injection.md
git commit -m "feat: inline size injection for AiAnalyzeImage (#104)"
```

---

## Self-Review notes

- **Spec coverage:** Decision 1 (drop env override) → Task 2 Step 4/5. Decision 2 (per-field validation) → Task 1. Decision 3/4 (chars unit, "approximately N characters") → Task 2 Step 2. Decision 5 (`extra_prompt` unchanged) → untouched; verified by retained extra-prompt tests. Decision 6 (delete `sizeOverrideBlock`) → Task 2 Step 3. Per-field ranges table → Task 1 Step 2 + Task 3 bounds tests. Testing section's Rewrite/Keep/Add lists → Task 3 Steps 1-10.
- **Type consistency:** `readSize(body, key, min, max)` signature defined in Task 1 Step 1, used in Task 1 Step 2. `buildPrompt(const SizeOverrides &)` defined in Task 2 Step 2, used in Task 2 Step 4. `SizeOverrides` shape unchanged.
- **Wording consistency:** every test's substring (`product title, approximately N characters`, `engaging product description, approximately N characters`, `SEO meta description, approximately N characters`) matches exactly what `buildPrompt` emits in Task 2 Step 2. The `title, approximately` / `description, approximately` / `meta description, approximately` negative-match substrings are unique prefixes of those override strings, so they correctly detect injection per field.
