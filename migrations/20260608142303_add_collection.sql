CREATE TABLE collection
(
    id               SERIAL PRIMARY KEY,
    title            VARCHAR(255) NOT NULL,
    slug             VARCHAR(255) NOT NULL UNIQUE,
    description      TEXT         NOT NULL,
    meta_description VARCHAR(255) NOT NULL,
    enabled          BOOLEAN      NOT NULL DEFAULT FALSE,
    created_at       TIMESTAMPTZ  NOT NULL DEFAULT NOW(),
    updated_at       TIMESTAMPTZ  NOT NULL DEFAULT NOW()
);

CREATE INDEX idx_collection_enabled ON collection (enabled);

CREATE TRIGGER set_collection_timestamp
    BEFORE UPDATE ON collection
    FOR EACH ROW EXECUTE FUNCTION trigger_set_timestamp();
