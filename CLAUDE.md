# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

C++20 REST API server built with the Drogon web framework, using PostgreSQL.

## Build Commands

All commands run from the project root:

```bash
# Configure (Release, without tests)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Configure (with tests enabled)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON

# Build
cmake --build build

# Run tests (requires ENABLE_TESTS=ON and a running PostgreSQL with `foxy_tests` database)
cd build && ctest

# Format code
cmake --build build --target clang-format

# The test binary is built as `foxy_tests` in the build/tests/ directory
```

**Environment variables are required at compile time** — all variables listed in `envs.cmake` must be set before running cmake, or the build will fail. These include API keys for Pinterest, YouTube, Twitter, plus `FOXY_CLIENT`, `FOXY_ADMIN`, `APP_CLOUD_NAME`, `APP_BUCKET_HOST`, `CONFIG_APP_PATH`, and `ENVIRONMENT`.

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
- `src/code/sentry_catcher/` — Sentry error tracking integration
- `tests/` — Google Test suite

## Code Style

- `.clang-format`: LLVM-based, 120 column limit, 4-space indent, always break template declarations
- `.clang-tidy`: warnings as errors, excludes `build/`, `cmake-build-debug/`, `3rdparty/`
- No spaces before parentheses (`SpaceBeforeParens: Never`)
