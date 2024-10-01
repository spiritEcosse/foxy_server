BEGIN;

ALTER TABLE item
    DROP COLUMN IF EXISTS "tags";

CREATE TABLE IF NOT EXISTS tag
(
    id           SERIAL PRIMARY KEY,
    title        VARCHAR(255) NOT NULL,
    social_media social_media_type[],
    item_id      INT          NOT NULL,
    created_at   TIMESTAMP    NOT NULL DEFAULT NOW(),
    updated_at   TIMESTAMP    NOT NULL DEFAULT NOW(),
    UNIQUE (title, item_id),
    FOREIGN KEY (item_id) REFERENCES item (id) ON DELETE CASCADE
);

DROP TRIGGER IF EXISTS set_timestamp ON "tag";
CREATE TRIGGER set_timestamp
    BEFORE UPDATE
    ON "tag"
    FOR EACH ROW
EXECUTE PROCEDURE trigger_set_timestamp();

ALTER TYPE social_media_type ADD VALUE 'Snapchat';
ALTER TYPE social_media_type ADD VALUE 'TikTok';
ALTER TYPE social_media_type ADD VALUE 'LinkedIn';

COMMIT;
