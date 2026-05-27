# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

C++20 REST API server built with the Drogon web framework, using PostgreSQL.

## Build Commands

All commands run from the project root. **No env vars required for cmake** — all config is read at runtime.

```bash
# Debug (with tests)
cmake --preset ninja-debug && cmake --build --preset ninja-debug
ctest --preset ninja-debug

# Release (with tests — for dev/CI)
cmake --preset ninja-release && cmake --build --preset ninja-release
ctest --preset ninja-release

# Production (no tests)
cmake --preset ninja-prod && cmake --build --preset ninja-prod

# Format code
cmake --build build/release --target clang-format
```

## Running Locally

The server needs runtime env vars for API keys and CORS origins. DB connects via unix socket with defaults (`foxy`/`foxy`).

```bash
# Option 1: direnv (recommended — auto-loads on cd)
direnv allow   # once, after installing direnv

# Option 2: manual export
export $(grep -v '^#' .env | grep -v '^$' | xargs)

# Then run
./build/debug/foxy_server
```

**SENTRY_DSN** is the one exception — it must be set *before* cmake if Sentry integration is needed, because it controls conditional compilation (`#if defined(SENTRY_DSN)`).

## Database

Connects via PostgreSQL unix socket (`/var/run/postgresql`). Hardcoded in the binary. DB name and user are runtime:

| Context | PG_DB | PG_USER |
|---------|-------|---------|
| Local (default) | `foxy` | `foxy` |
| Docker dev | `foxy_dev` | `foxy_dev` |
| Docker prod | `foxy` | `foxy` |

Docker containers get `PG_DB`/`PG_USER` from `docker-compose.yml`.

## Database Migrations (Atlas)

Schema changes are managed with [Atlas](https://atlasgo.io) versioned migrations stored in `migrations/`.

### Apply pending migrations locally
```bash
atlas migrate apply --env local
```

### Create a new migration
```bash
# 1. Create a timestamped file
atlas migrate new --env local --name <short_description>
# e.g. → migrations/20260301120000_add_item_weight.sql

# 2. Write the SQL in that file, then recompute the checksum
atlas migrate hash

# 3. Apply and verify
atlas migrate apply --env local

# 4. Commit migrations/<new_file>.sql + updated migrations/atlas.sum
```

### Check migration status
```bash
atlas migrate status --env local
```

Atlas tracks applied migrations in the `atlas_schema_revisions` table. On the next `docker-compose up`, `migrate-main`/`migrate-dev` automatically apply any new files.

### How to write migration SQL files

**Do not add `BEGIN`/`COMMIT` to migration files.** Atlas wraps every migration in its own transaction automatically (`txmode=file`). Adding explicit `BEGIN/COMMIT` causes Atlas to exit with `"unexpected transaction status idle"` because the inner `COMMIT` ends Atlas's outer transaction before it records the revision — breaking CI/CD.

```sql
ALTER TABLE item ADD COLUMN weight_kg NUMERIC(8,3);
ALTER TABLE item ALTER COLUMN price SET NOT NULL;
```

**Rules:**
- Do not wrap statements in `BEGIN; ... COMMIT;` — Atlas handles the transaction.
- One logical change per file (e.g. "add column" or "create table", not both in the same file unless they are tightly coupled).
- Never edit an already-committed migration file. If you need to fix something, create a new migration that corrects it.
- Do not use `IF NOT EXISTS` / `IF EXISTS` guards — Atlas will never re-apply a migration it has already recorded.
- Do not use the `DO $$ BEGIN ... END $$;` wrapper from the baseline — that was needed for the old idempotent single-file approach. Plain SQL is the correct pattern here.

**Non-transactional operations** (`CREATE INDEX CONCURRENTLY`, `VACUUM`, etc.) cannot run inside a transaction. Use the Atlas directive to opt out per-file:

```sql
-- atlas:txmode off

CREATE INDEX CONCURRENTLY idx_item_price ON item (price);
```

Put non-transactional files alone — never mix transactional and non-transactional DDL in the same file.

## Architecture

### Namespace
All application code is in `namespace api::v1`.

### Models (`src/code/models/`)
Each model extends `BaseModel<T>` (CRTP pattern). Models define:
- A static `tableName`
- A nested `Field` struct that registers database columns via `registerFields()`
- Auto-generated SQL for INSERT, UPDATE, DELETE, SELECT operations
- JSON serialization/deserialization from `Json::Value`
- A `SetMapFieldTypes` vector mapping fields to variant values for SQL generation

### Controllers (`src/code/controllers/`)
Controllers extend `BaseCRUD<T, R>` where T is the model and R is the controller. BaseCRUD provides standard REST operations: `getOne`, `getList`, `createItem`, `createItems`, `updateItem`, `updateItems`, `deleteItem`, `deleteItems`. All handlers use Drogon's async callback pattern with `std::shared_ptr<std::function<void(const HttpResponsePtr&)>>`.

### ORM (`src/code/orm/`)
Custom query builder — not Drogon's built-in ORM:
- `QuerySet<T>` — chainable SQL query builder (select, filter, join, pagination)
- `WhereClause` — builds WHERE conditions
- `Function` — SQL functions (COUNT, etc.)
- `BaseField` — represents a database column with table qualification

### Key Directories
- `src/code/auth/` — authentication/JWT handling
- `src/code/filters/` — Drogon request filters
- `src/code/clients/` — external API clients (Pinterest, YouTube, Twitter)
- `src/code/utils/` — database helpers, JWT, request parsing, exceptions
- `src/code/utils/config.h` — runtime `getEnv()` utility (replaces compile-time macros)
- `src/code/sentry_catcher/` — Sentry error tracking integration
- `tests/` — Google Test suite
- `cmake/` — modular cmake includes (cpm, dependencies, tests, format, sanitizers, sentry)

## Workflow

### Branching Rules

- **GitHub Issue work**: Create and link a feature branch using `gh issue develop <N> --name issue/<N>-<short-description> --base dev --checkout` — this creates the branch, links it to the issue in GitHub, and checks it out in one step.
- **Create pull request**: After pushing a feature branch, ALWAYS create a PR with `gh pr create --base dev` including `Closes #<N>` in the body to auto-link and auto-close the issue on merge.
- **After creating a PR**: Wait 30 seconds, then run `./scripts/sonarcloud-check.sh`. If there are **zero issues** on new code, check CI jobs with `gh pr checks <PR#> --watch`. Only merge after **both** SonarCloud gate AND all CI jobs (ninja-debug, ninja-asan-ubsan, ninja-tsan) pass (`gh pr merge --squash`). If ANY SonarCloud issues or CI failures, fix them all, push, and re-check until clean.
- **Close issue after merge**: After merging a feature branch into `dev`, ALWAYS close the issue with `gh issue close <N>`. Do NOT wait to be asked.
- **Switch to dev after merge**: After merging and closing the issue, ALWAYS run `git checkout dev && git pull` to return to the main branch.
- **Ad-hoc fixes** (no GitHub Issue): Work directly on `dev`.

Before implementing any feature or fix: if a GitHub issue exists for it, create a branch using `gh issue develop` as described above.

When creating a pull request, always include `Closes #<issue-number>` in the PR body. This links the PR to the issue in GitHub's UI and automatically closes the issue when the PR is merged.

```bash
gh pr create --base dev --title "..." --body "$(cat <<'EOF'
## Summary
...

Closes #42
EOF
)"
```

## SonarCloud Quality Checks

Use `scripts/sonarcloud-check.sh` to inspect code quality. Requires `SONAR_TOKEN` env var.

```bash
# Check current branch (auto-detects dev/main → branch mode, feature → PR mode)
./scripts/sonarcloud-check.sh

# Force branch mode for dev
./scripts/sonarcloud-check.sh --branch dev

# Check a specific PR
./scripts/sonarcloud-check.sh --pr 42
```

### Fix-and-verify workflow

After pushing fixes, SonarCloud takes a few minutes to re-analyze. Follow this loop:

1. Run the check — note all CRITICAL, MAJOR, MINOR issues
2. Fix every issue in code, build to confirm (`cmake --build --preset ninja-release`)
3. Commit and push
4. Wait ~3–5 minutes, then re-run the check
5. Repeat until no code smell issues remain

```bash
# Full loop (fix → build → commit → push → wait → verify)
cmake --build --preset ninja-release 2>&1 | tail -5   # confirm build
git add <files> && git commit -m "fix: ..."
git push origin dev
sleep 180  # wait ~3 min for SonarCloud CI to finish
./scripts/sonarcloud-check.sh --branch dev
```

### Common fix patterns (from project history)

| Rule | Pattern | Fix |
|------|---------|-----|
| cpp:S1181 | `catch(std::runtime_error&)` or `catch(std::exception&)` around GTest/CPR ops that don't throw | Remove the dead try/catch entirely |
| cpp:S5421 | Mutable global `static` variables | Move into the class that owns them |
| cpp:S3776 | Cognitive complexity too high | Extract a private helper method for complex conditions |
| cpp:S1192 | String literal repeated 3+ times in PL/pgSQL | Add `DECLARE … CONSTANT text := '…'` in the `DO $$` block |
| cpp:S1481 | `catch(const T& e)` where `e` is never used | Drop the variable name: `catch(const T&)` |
| cpp:S2738 | `catch(...)` | Replace with `catch(const std::exception&)` |
| cpp:S6004 | Variable declared before `if`, used only in condition | Use C++17 if-init: `if(auto val = …; val)` |
| cpp:S6177 | Verbose `EnumType::VALUE` in switch cases | Add `using enum EnumType;` at top of switch |
| cpp:S6009 | `const std::string&` lambda param used only for comparison | Use `std::string_view` instead |
| docker:S7031 | Consecutive `RUN` instructions in Dockerfile | Merge with `&& \` |
| plsql:S1192 | Duplicate string literal 3+ times in SQL | Declare a PL/pgSQL `CONSTANT` variable |

### Quality gate
The gate fails on `new_security_hotspots_reviewed: 0%` — this requires manually reviewing/acknowledging hotspots in the SonarCloud UI (not fixable in code).

## Code Style

- `.clang-format`: LLVM-based, 120 column limit, 4-space indent, always break template declarations
- `.clang-tidy`: warnings as errors, excludes `build/`, `cmake-build-debug/`
- No spaces before parentheses (`SpaceBeforeParens: Never`)
