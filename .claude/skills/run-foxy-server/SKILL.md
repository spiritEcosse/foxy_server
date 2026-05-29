---
description: Launch and smoke-test the foxy_server C++ API server locally
---

# Run foxy_server

## Prerequisites

- PostgreSQL running on unix socket `/var/run/postgresql` with DB `foxy`, user `foxy`
- `.env` file present in project root with API keys and CORS origins
- Debug binary built: `cmake --preset ninja-debug && cmake --build --preset ninja-debug`

## Check if already running

```bash
pgrep -a foxy_server
```

If already running on port 8080, skip to **Verify**.

## Run

Export env vars and start in the background:

```bash
export $(grep -v '^#' .env | grep -v '^$' | xargs)
./build/debug/foxy_server &> /tmp/foxy_server.log &
SERVER_PID=$!
```

Wait for readiness:

```bash
for i in {1..20}; do
  curl -sf http://localhost:8080/api/v1/item > /dev/null && break
  sleep 0.5
done
```

## Verify

```bash
curl -s 'http://localhost:8080/api/v1/item?limit=3'
# → {"_page":null,"data":[...],"total":N}
```

## Stop

```bash
kill $SERVER_PID
# or if PID is lost:
pkill -f foxy_server
```

Logs are at `/tmp/foxy_server.log`.

## Environment

| Variable | Required | Notes |
|---|---|---|
| `FOXY_CLIENT` | Yes | Allowed CORS origin for the client app |
| `FOXY_ADMIN` | Yes | Allowed CORS origin for the admin app |
| `PINTEREST_*` | No | Pinterest API credentials |
| `YOUTUBE_*` | No | YouTube API credentials |
| `TWITTER_*` | No | Twitter API credentials |
| `APP_CLOUD_NAME` | Yes | S3-compatible storage hostname |
| `APP_BUCKET_HOST` | Yes | S3 bucket host |

## Port

Always **8080**. Not configurable via env var — set in `config.json`.
