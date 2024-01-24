CREATE OR REPLACE FUNCTION GetValidPage(RequestedPage INT, PageSize INT, OUT ValidPage INT)
AS $$
DECLARE
TotalPages INT;
BEGIN
    -- Calculate the total number of pages
SELECT CEIL(COUNT(*)::FLOAT / PageSize) INTO TotalPages
FROM item;

-- Check if the requested page number is greater than the total number of pages
IF RequestedPage > TotalPages THEN
        ValidPage := 1; -- Set to the first page
ELSE
        ValidPage := RequestedPage;
END IF;
END
$$ LANGUAGE PLPGSQL;

CREATE OR REPLACE FUNCTION trigger_set_timestamp()
    RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

create table IF NOT EXISTS item (
                                    id serial primary key,
                                    title varchar(255) NOT NULL,
                                    meta_description TEXT NOT NULL,
                                    description TEXT NOT NULL,
                                    slug varchar(255) NOT NULL unique,
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
