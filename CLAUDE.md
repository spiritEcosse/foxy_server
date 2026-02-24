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

**Use a transaction for every migration.** PostgreSQL supports transactional DDL — if the migration fails mid-way, the transaction rolls back cleanly with no partial state.

```sql
BEGIN;

ALTER TABLE item ADD COLUMN weight_kg NUMERIC(8,3);
ALTER TABLE item ALTER COLUMN price SET NOT NULL;

COMMIT;
```

**Rules:**
- Always wrap statements in `BEGIN; ... COMMIT;` — Atlas applies each file atomically in PostgreSQL by default, but being explicit makes intent clear.
- One logical change per file (e.g. "add column" or "create table", not both in the same file unless they are tightly coupled).
- Never edit an already-committed migration file. If you need to fix something, create a new migration that corrects it.
- Do not use `IF NOT EXISTS` / `IF EXISTS` guards — Atlas will never re-apply a migration it has already recorded.
- Do not use the `DO $$ BEGIN ... END $$;` wrapper from the baseline — that was needed for the old idempotent single-file approach. Plain SQL in a transaction is the correct pattern here.

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

Before implementing any feature or fix: if a GitHub issue exists for it, create a branch named `issue-<number>-<short-description>` and switch to it before writing any code.

```bash
git checkout -b issue-42-add-shipping-rates
```

## Code Style

- `.clang-format`: LLVM-based, 120 column limit, 4-space indent, always break template declarations
- `.clang-tidy`: warnings as errors, excludes `build/`, `cmake-build-debug/`
- No spaces before parentheses (`SpaceBeforeParens: Never`)
