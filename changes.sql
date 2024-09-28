BEGIN;

ALTER TABLE item
    DROP COLUMN IF EXISTS "tags";

CREATE TABLE IF NOT EXISTS tags
(
    id           SERIAL PRIMARY KEY,
    title        VARCHAR(255) NOT NULL,
    social_media social_media_type[],
    item_id      INT          NOT NULL,
    created_at   TIMESTAMP    NOT NULL DEFAULT NOW(),
    updated_at   TIMESTAMP    NOT NULL DEFAULT NOW(),
    FOREIGN KEY (item_id) REFERENCES item (id) ON DELETE CASCADE
);

COMMIT;
