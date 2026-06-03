# Inline size injection for AiAnalyzeImage

**Date:** 2026-06-03
**File touched:** `src/code/controllers/AiAnalyzeImage.cpp`

## Problem

`AiAnalyzeImage` lets callers pass per-field character-size overrides
(`title_size`, `description_size`, `meta_description_size`). Today these are
expressed as a **separate block appended** to the end of the prompt:

```
Field size overrides (use these exact limits, in characters): title max 60; description max 200.
```

This conflicts with the inline placeholder wording that still lives in the JSON
shape (`<short product title, 5-12 words>`, `<SEO meta description, max 160
chars>`). Two instructions fight:

- The inline placeholder says "be short / max 160".
- The appended block says "use these exact limits".

Consequences:
- **Shrinking** works (a `max` below default genuinely constrains).
- **Expanding** is unreliable — `max N` is read as a ceiling, not a target, and
  the "short / 2-4 sentence" inline wording still pulls the model down.

## Goal

Replace the append-a-block approach with **direct inline injection** into the
prompt's JSON-shape placeholders, so each field carries a single,
non-conflicting instruction. This enables both shrinking and expanding.
Validate sizes per field; out-of-range values silently fall back to the default
wording for that field.

## Decisions (from brainstorming)

1. **Drop the `CLAUDE_ANALYZE_IMAGE_PROMPT` env override entirely.** Always use
   the templated default prompt. Nothing else in the repo references this var.
2. **Per-field min/max validation.** Each field has its own sane character range.
   Out-of-range (or non-positive / non-integral) → treat as unset → default
   wording for that field. No error is returned; it just falls back.
3. **Override unit is always characters.** Matches the `*_size` field names.
   When a field is overridden, its placeholder switches to
   `approximately N characters`. When a field is **not** overridden, it keeps its
   natural default wording (title in words, description in sentences, meta in
   chars).
4. **Wording when set:** `approximately N characters` (a target, encouraging the
   model to fill toward N — enabling expansion).
5. **`extra_prompt` is unchanged** — it stays appended via `extraPromptBlock()`.
   Only the size handling moves from append to inline injection.
6. **Delete `sizeOverrideBlock()`** outright.

## Per-field ranges and fallback wording

| Field            | `*_size` body key        | Unit  | min | max  | Default wording when not overridden            |
|------------------|--------------------------|-------|-----|------|------------------------------------------------|
| title            | `title_size`             | chars | 10  | 60   | `<short product title, 5-12 words>`            |
| description      | `description_size`       | chars | 50  | 1000 | `<2-4 sentence engaging product description>`  |
| meta_description | `meta_description_size`  | chars | 50  | 160  | `<SEO meta description, max 160 chars>`         |

The `title` max is capped at **60** to match Google's SERP title truncation
(~50–60 chars / ~600px desktop), since the title doubles as the SEO page title.
The `description` field is product *content* (not a meta tag), so Google imposes
no length rule — its 50–1000 range is just sane bounds.

The `meta_description` max is capped at **160** to match Google's SERP snippet
truncation point (~155–160 chars desktop). Google has no official required
length and may rewrite snippets, but ~160 is the established SEO-safe target, so
allowing longer overrides would just get truncated in results.

When overridden and in range, the placeholder becomes:

```
"title":            "<product title, approximately N characters>"
"description":      "<engaging product description, approximately N characters>"
"meta_description": "<SEO meta description, approximately N characters>"
```

## Implementation

All changes are in `src/code/controllers/AiAnalyzeImage.cpp`.

### 1. Make the prompt a template

`DEFAULT_PROMPT` currently is a `constexpr std::string_view`. The fixed prefix
("You are an assistant...") and the fixed suffix (tag rules, etc.) stay constant.
The three placeholder lines become substitution points filled by a builder
function.

Approach: keep the constant text split around the three placeholders, and have a
function `buildPrompt(const SizeOverrides &sizes)` assemble the full default
prompt, choosing per field between the override wording and the default wording.

### 2. Per-field validation

Replace the single `readSize(body, key)` (which only rejects non-positive) with a
helper that also enforces per-field bounds, e.g.:

```cpp
int readSize(const Json::Value &body, const char *key, int min, int max);
```

Returns `0` (unset) when the value is non-integral, `< min`, or `> max`. The
`SizeOverrides` struct is unchanged in shape; `0` continues to mean "unset →
default wording".

Call sites pass the bounds:

```cpp
const SizeOverrides sizes{
    .title           = readSize(body, "title_size", 10, 60),
    .description     = readSize(body, "description_size", 50, 1000),
    .metaDescription = readSize(body, "meta_description_size", 50, 160)};
```

### 3. Remove `sizeOverrideBlock()`

Delete the function and its call in `runClaudeAndRespond`. The size info is now
carried inline by `buildPrompt(sizes)`.

### 4. Remove the env override

Delete the `getEnv("CLAUDE_ANALYZE_IMAGE_PROMPT", ...)` line in
`runClaudeAndRespond`; build the prompt via `buildPrompt(sizes)` instead.

### 5. `extra_prompt` unchanged

`readExtraPrompt` / `extraPromptBlock` stay. `extraPromptBlock(extraPrompt)`
remains appended after the (now inline-templated) default prompt.

## Resulting `fullPrompt` structure

```
<templated default prompt — sizes injected inline into the 3 placeholders>
<extraPromptBlock — appended, unchanged>

Use the Read tool to load the image at <path> and then analyze it. Return ONLY
the JSON object — no prose, no markdown fences, no explanation.
```

## Data flow (shape unchanged)

`getJsonObject` → read the three `*_size` keys with per-field bounds → validate →
`SizeOverrides` → captured into the worker thread → `runClaudeAndRespond` builds
the templated prompt via `buildPrompt(sizes)`. The Claude response is still parsed
by JSON **keys** (`title`, `description`, ...), not by placeholder text, so there
is no downstream impact.

## Testing

The valuable, deterministic coverage is the prompt-building / validation logic,
which is pure (no network, no `popen`):

- `readSize` bounds: for each field, assert below-min, in-range, above-max,
  non-integral, and missing → correct `0`-or-value result.
- `buildPrompt`: for each field, assert that an in-range override produces
  `approximately N characters` and that an unset field keeps its default wording.
  Spot-check a mixed case (one field overridden, two default).

The `popen`/CLI execution path is left as-is (unchanged behavior, hard to unit
test without invoking the CLI).

**Testability note:** `buildPrompt` and `readSize` currently live in an anonymous
namespace (`namespace {` at the top of the TU), so they are not linkable from a
test translation unit. To unit-test them, they must be exposed — either moved out
of the anonymous namespace into `api::v1` (and declared in a header), or the pure
logic extracted to a small testable helper. The implementation plan must pick one;
the recommendation is to move both into `api::v1` with a header declaration,
matching how other testable helpers in the project are organized. Confirm this
against an existing controller's test setup before finalizing.

## Out of scope

- Changing the `extra_prompt` mechanism.
- Adding new overridable fields (e.g. tag counts).
- Rewriting the default (non-overridden) wording units.
