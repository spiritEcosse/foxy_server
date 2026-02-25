CREATE TABLE pinterest_token
(
    id                       SERIAL PRIMARY KEY,
    singleton                BOOLEAN     NOT NULL DEFAULT TRUE UNIQUE CHECK (singleton = TRUE),
    access_token             TEXT        NOT NULL,
    access_token_expires_at  TIMESTAMPTZ NOT NULL,
    refresh_token            TEXT        NOT NULL,
    refresh_token_expires_at TIMESTAMPTZ NOT NULL,
    scope                    TEXT        NOT NULL,
    created_at               TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at               TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TRIGGER set_pinterest_token_timestamp
    BEFORE UPDATE ON pinterest_token
    FOR EACH ROW EXECUTE FUNCTION trigger_set_timestamp();
