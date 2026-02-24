# foxy_server

C++20 REST API server built with [Drogon](https://github.com/drogonframework/drogon) and PostgreSQL.

## Deployment

Deployments are fully automated via GitHub Actions. On every push to `main` or `dev`, after the build workflow succeeds:

1. The Docker image is built and pushed to GHCR (`ghcr.io/spiritecosse/foxy_server:<branch>`)
2. The deploy workflow connects to the production server via Docker context SSH
3. Pulls the new image and restarts the corresponding service

```
push to main → build → push image :main → deploy foxy-main (prod)
push to dev  → build → push image :dev  → deploy foxy-dev  (dev)
```

### GitHub Secrets required

| Secret | Description |
|---|---|
| `DEPLOY_SSH_KEY` | SSH private key for prod server access |
| `DEPLOY_SSH_KNOWN_HOSTS` | Output of `ssh-keyscan <prod-host>` run once locally |
| `DEPLOY_HOST` | Production server hostname or IP |
| `DEPLOY_USER` | SSH user on the production server |
| `FOXY_CLIENT` | Allowed CORS origin (client app) |
| `FOXY_ADMIN` | Allowed CORS origin (admin app) |
| `APP_CLOUD_NAME` | Cloudinary cloud name |
| `APP_BUCKET_HOST` | Storage bucket host |
| `ENVIRONMENT` | Runtime environment label |
| `PINTEREST_API_HOST` | Pinterest API base URL |
| `PINTEREST_BOARD_ID` | Pinterest board ID |
| `PINTEREST_CLIENT_ID` | Pinterest OAuth client ID |
| `PINTEREST_CLIENT_SECRET` | Pinterest OAuth client secret |
| `PINTEREST_REFRESH_TOKEN` | Pinterest refresh token |
| `YOUTUBE_CLIENT_ID` | YouTube OAuth client ID |
| `YOUTUBE_CLIENT_SECRET` | YouTube OAuth client secret |
| `YOUTUBE_REFRESH_TOKEN` | YouTube refresh token |
| `TWITTER_API_KEY` | Twitter API key |
| `TWITTER_API_SECRET` | Twitter API secret |
| `TWITTER_ACCESS_TOKEN` | Twitter access token |
| `TWITTER_ACCESS_TOKEN_SECRET` | Twitter access token secret |
| `TWITTER_BEARER_TOKEN` | Twitter bearer token |

### Rollback

```bash
git revert <commit>
git push origin main
```

The revert triggers a new build and deploy automatically.

## Atlas / Database Migrations

Schema changes are managed with [Atlas](https://atlasgo.io) versioned migrations in `migrations/`.

```bash
# Apply pending migrations locally
atlas migrate apply --env local

# Create a new migration
atlas migrate new --env local --name <description>

# Show applied/pending migrations
atlas migrate status --env local
```

## Local Development

See `CLAUDE.md` for build commands, local setup, and architecture overview.
