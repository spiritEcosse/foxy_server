CREATE TABLE collection_item
(
    id            SERIAL PRIMARY KEY,
    collection_id INT         NOT NULL,
    item_id       INT         NOT NULL,
    created_at    TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at    TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    FOREIGN KEY (collection_id) REFERENCES collection (id) ON DELETE CASCADE,
    FOREIGN KEY (item_id) REFERENCES item (id) ON DELETE CASCADE,
    UNIQUE (collection_id, item_id)
);

CREATE INDEX idx_collection_item_item_id ON collection_item (item_id);

CREATE TRIGGER set_collection_item_timestamp
    BEFORE UPDATE ON collection_item
    FOR EACH ROW EXECUTE FUNCTION trigger_set_timestamp();
