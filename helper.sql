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
                                    profile_id INT NOT NULL,
                                    price decimal(10, 2) DEFAULT 0,
                                    slug varchar(255) NOT NULL unique,
                                    enabled boolean DEFAULT true,
                                    created_at timestamp NOT NULL DEFAULT NOW(),
                                    FOREIGN KEY (profile_id) REFERENCES shipping_profile(id) ON DELETE CASCADE,
                                    updated_at timestamp NOT NULL DEFAULT NOW()
    );

CREATE TABLE IF NOT EXISTS country (
    id SERIAL PRIMARY KEY,
    title VARCHAR(255) NOT NULL UNIQUE,
    created_at timestamp NOT NULL DEFAULT NOW(),
    updated_at timestamp NOT NULL DEFAULT NOW()
);

INSERT INTO country (title) VALUES
    ('United States of America'),
    ('Spain');


CREATE TABLE IF NOT EXISTS shipping_profile (
    id SERIAL PRIMARY KEY,
    title VARCHAR(255) NOT NULL,
    country_id INT NOT NULL,
    postal_code VARCHAR(255) NOT NULL,
    processing_time INT NOT NULL,
    shipping_upgrade_cost DECIMAL(10, 2) default 0,
    created_at timestamp NOT NULL DEFAULT NOW(),
    updated_at timestamp NOT NULL DEFAULT NOW(),
    FOREIGN KEY (country_id) REFERENCES country(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS shipping_rate (
    id SERIAL PRIMARY KEY,
    profile_id INT NOT NULL,
    country_id INT,
    delivery_days_min INT NOT NULL,
    delivery_days_max INT NOT NULL,
    created_at timestamp NOT NULL DEFAULT NOW(),
    updated_at timestamp NOT NULL DEFAULT NOW(),
    FOREIGN KEY (profile_id) REFERENCES shipping_profile(id) ON DELETE CASCADE,
    FOREIGN KEY (country_id) REFERENCES country(id) ON DELETE CASCADE
);

create table IF NOT EXISTS "user" (
                                    id serial primary key,
                                    email varchar(255) NOT NULL unique,
                                    password varchar(255) DEFAULT '',
                                    created_at timestamp NOT NULL DEFAULT NOW(),
                                    updated_at timestamp NOT NULL DEFAULT NOW()
    );

create table IF NOT EXISTS media (
                                    id serial primary key,
                                     src varchar(255) NOT NULL,
    item_id integer NOT NULL,
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
    BEFORE UPDATE ON "item"
    FOR EACH ROW
EXECUTE PROCEDURE trigger_set_timestamp();

CREATE TRIGGER set_timestamp
    BEFORE UPDATE ON "media"
    FOR EACH ROW
EXECUTE PROCEDURE trigger_set_timestamp();

CREATE TRIGGER set_timestamp
    BEFORE UPDATE ON "page"
    FOR EACH ROW
EXECUTE PROCEDURE trigger_set_timestamp();

CREATE TRIGGER set_timestamp
    BEFORE UPDATE ON "user"
    FOR EACH ROW
EXECUTE PROCEDURE trigger_set_timestamp();

CREATE TRIGGER set_timestamp
    BEFORE UPDATE ON "shipping_profile"
    FOR EACH ROW
EXECUTE PROCEDURE trigger_set_timestamp();

CREATE TRIGGER set_timestamp
    BEFORE UPDATE ON "shipping_rate"
    FOR EACH ROW
EXECUTE PROCEDURE trigger_set_timestamp();

CREATE TRIGGER set_timestamp
    BEFORE UPDATE ON "country"
    FOR EACH ROW
EXECUTE PROCEDURE trigger_set_timestamp();

CREATE OR REPLACE FUNCTION format_src(src TEXT, cloud_name TEXT)
    RETURNS TEXT AS $$
BEGIN
    RETURN 'https://' || cloud_name || '.twic.pics/' || src;
END;
$$ LANGUAGE plpgsql;
