-- DROP TAble if exists  "user";
-- DROP TAble if exists media;
-- DROP TAble if exists item;
-- DROP TAble if exists page;

CREATE OR REPLACE FUNCTION GetValidPage(RequestedPage INT, PageSize INT, TotalCount INT, OUT ValidPage INT)
AS $$
DECLARE
    TotalPages INT;
BEGIN
    -- Calculate the total number of pages
    TotalPages := CEIL(TotalCount::FLOAT / PageSize);

    -- Check if the requested page number is greater than the total number of pages
    IF RequestedPage > TotalPages THEN
        ValidPage := 1; -- Set to the first page
    ELSE
        ValidPage := RequestedPage;
    END IF;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION trigger_set_timestamp()
    RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION do_and_check(query TEXT) RETURNS json AS $$
DECLARE
    rows INTEGER;
    result json;
BEGIN
    EXECUTE query INTO result;

    GET DIAGNOSTICS rows = ROW_COUNT;
    IF rows = 0 THEN
        RAISE EXCEPTION 'not_found';
    END IF;
    RETURN result;
END;
$$ LANGUAGE plpgsql;

create table IF NOT EXISTS item (
                                    id serial primary key,
                                    title varchar(255) NOT NULL,
                                    meta_description TEXT NOT NULL,
                                    description TEXT NOT NULL,
                                    slug varchar(255) NOT NULL unique,
                                    enabled boolean DEFAULT true,
                                    created_at timestamp NOT NULL DEFAULT NOW(),
                                    updated_at timestamp NOT NULL DEFAULT NOW()
    );

create table IF NOT EXISTS "user" (
                                    id serial primary key,
                                    email varchar(255) NOT NULL unique,
                                    password varchar(255) NOT NULL,
                                    created_at timestamp NOT NULL DEFAULT NOW(),
                                    updated_at timestamp NOT NULL DEFAULT NOW()
    );

INSERT INTO "user" (email, password) VALUES ('admin@localhost', '$2b$10$QBLgOdKLG8TdKLFG5UCKQulMDtD43LClVpSNwhC57c3SGjW4Sr.fG');

create table IF NOT EXISTS media (
                                    id serial primary key,
                                     src varchar(255) NOT NULL,
    item_id integer NOT NULL,
    thumb varchar(255),
    sort integer DEFAULT 1,
    created_at timestamp NOT NULL DEFAULT NOW(),
    updated_at timestamp NOT NULL DEFAULT NOW(),
    CONSTRAINT fk_item
    FOREIGN KEY(item_id)
    REFERENCES item(id)
    ON DELETE CASCADE
    );

create table if not EXISTS page (
                                    id serial primary key,
                                    title varchar(255) NOT NULL,
                                    slug varchar(255) NOT NULL unique,
                                    meta_description TEXT NOT NULL,
                                    description TEXT NOT NULL,
                                    enabled boolean DEFAULT true,
                                    canonical_url varchar(255) NOT NULL unique,
                                    created_at timestamp NOT NULL DEFAULT NOW(),
                                    updated_at timestamp NOT NULL DEFAULT NOW()
);

CREATE TRIGGER set_timestamp
    BEFORE UPDATE ON item
    FOR EACH ROW
EXECUTE PROCEDURE trigger_set_timestamp();

CREATE TRIGGER set_timestamp
    BEFORE UPDATE ON media
    FOR EACH ROW
EXECUTE PROCEDURE trigger_set_timestamp();

CREATE TRIGGER set_timestamp
    BEFORE UPDATE ON page
    FOR EACH ROW
EXECUTE PROCEDURE trigger_set_timestamp();

CREATE TRIGGER set_timestamp
    BEFORE UPDATE ON "user"
    FOR EACH ROW
EXECUTE PROCEDURE trigger_set_timestamp();

CREATE OR REPLACE FUNCTION format_src(src TEXT, cloud_name TEXT)
    RETURNS TEXT AS $$
BEGIN
    RETURN 'https://' || cloud_name || '.twic.pics/' || src || '?twic=v1';
END;
$$ LANGUAGE plpgsql;
