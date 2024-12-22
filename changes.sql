BEGIN;

CREATE INDEX idx_item_id ON social_media (item_id);
CREATE INDEX idx_item_id_media ON media (item_id);
CREATE INDEX idx_item_id_tag ON tag (item_id);

CREATE INDEX idx_item_media
    ON media (item_id, type);

-- Update table statistics
ANALYZE item;
ANALYZE social_media;
ANALYZE media;

ALTER TABLE media
    ADD COLUMN content_type VARCHAR(20) DEFAULT '' not null;
ALTER TABLE media
    ALTER COLUMN content_type DROP DEFAULT;
COMMIT;
