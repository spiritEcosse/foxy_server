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

## Code Style

- `.clang-format`: LLVM-based, 120 column limit, 4-space indent, always break template declarations
- `.clang-tidy`: warnings as errors, excludes `build/`, `cmake-build-debug/`
- No spaces before parentheses (`SpaceBeforeParens: Never`)
