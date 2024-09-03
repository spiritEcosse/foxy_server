BEGIN;

CREATE TYPE media_type AS ENUM ('video', 'image');

Alter table media
    ADD COLUMN type media_type NOT NULL DEFAULT 'image';

COMMIT;
