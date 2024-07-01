DO
$$
    BEGIN
        DROP TAble if exists "review";
        DROP TAble if exists "order";
        DROP TAble if exists "address";
        DROP TAble if exists "basket_item";
        DROP TAble if exists "basket";
        DROP TAble if exists "user";
        DROP TAble if exists "media";
        DROP TAble if exists "item";
        DROP TAble if exists "page";
        DROP TAble if exists "shipping_rate";
        DROP TAble if exists "shipping_profile";
        DROP TAble if exists "countries_ips";
        DROP TAble if exists "country";

        CREATE OR REPLACE FUNCTION GetValidPage(RequestedPage INT, PageSize INT, TotalCount INT, OUT ValidPage INT)
        AS
        $body$
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
        $body$ LANGUAGE plpgsql;

        CREATE OR REPLACE FUNCTION trigger_set_timestamp()
            RETURNS TRIGGER AS
        $body$
        BEGIN
            NEW.updated_at = NOW();
            RETURN NEW;
        END;
        $body$ LANGUAGE plpgsql;

        CREATE OR REPLACE FUNCTION do_and_check(query TEXT) RETURNS json AS
        $body$
        DECLARE
            rows   INTEGER;
            result json;
        BEGIN
            EXECUTE query INTO result;

            GET DIAGNOSTICS rows = ROW_COUNT;
            IF rows = 0 THEN
                RAISE EXCEPTION 'not_found';
            END IF;
            RETURN result;
        END;
        $body$ LANGUAGE plpgsql;

        CREATE TABLE IF NOT EXISTS country
        (
            id         SERIAL PRIMARY KEY,
            title      VARCHAR(255) NOT NULL UNIQUE,
            code       VARCHAR(255) NOT NULL UNIQUE,
            created_at timestamp    NOT NULL DEFAULT NOW(),
            updated_at timestamp    NOT NULL DEFAULT NOW()
        );

        CREATE TABLE IF NOT EXISTS shipping_profile
        (
            id                    SERIAL PRIMARY KEY,
            title                 VARCHAR(255) NOT NULL,
            country_id            INT          NOT NULL,
            postal_code           VARCHAR(255) NOT NULL,
            processing_time       INT          NOT NULL,
            shipping_upgrade_cost DECIMAL(10, 2)        default 0,
            created_at            timestamp    NOT NULL DEFAULT NOW(),
            updated_at            timestamp    NOT NULL DEFAULT NOW(),
            FOREIGN KEY (country_id) REFERENCES country (id) ON DELETE CASCADE
        );

        create table IF NOT EXISTS item
        (
            id                  serial primary key,
            title               varchar(255) NOT NULL,
            meta_description    TEXT         NOT NULL,
            description         TEXT         NOT NULL,
            shipping_profile_id INT          NOT NULL,
            price               decimal(10, 2)        DEFAULT 0,
            slug                varchar(255) NOT NULL unique,
            enabled             boolean               DEFAULT true,
            created_at          timestamp    NOT NULL DEFAULT NOW(),
            FOREIGN KEY (shipping_profile_id) REFERENCES shipping_profile (id) ON DELETE CASCADE,
            updated_at          timestamp    NOT NULL DEFAULT NOW()
        );

        INSERT INTO country (title, code)
        VALUES ('United States of America', 'US'),
               ('Spain', 'ES');

        CREATE TABLE IF NOT EXISTS shipping_rate
        (
            id                  SERIAL PRIMARY KEY,
            shipping_profile_id INT       NOT NULL,
            country_id          INT,
            delivery_days_min   INT       NOT NULL,
            delivery_days_max   INT       NOT NULL,
            created_at          timestamp NOT NULL DEFAULT NOW(),
            updated_at          timestamp NOT NULL DEFAULT NOW(),
            FOREIGN KEY (shipping_profile_id) REFERENCES shipping_profile (id) ON DELETE CASCADE,
            FOREIGN KEY (country_id) REFERENCES country (id) ON DELETE CASCADE
        );

        create table IF NOT EXISTS "user"
        (
            id             serial primary key,
            email          varchar(255) NOT NULL unique,
            password       varchar(255)          DEFAULT '',
            first_name     VARCHAR(255) NOT NULL,
            last_name      VARCHAR(255) NOT NULL,
            birthday       DATE                  DEFAULT NULL,
            has_newsletter BOOLEAN      NOT NULL DEFAULT false,
            created_at     timestamp    NOT NULL DEFAULT NOW(),
            updated_at     timestamp    NOT NULL DEFAULT NOW()
        );

        CREATE TABLE IF NOT EXISTS "address"
        (
            id         SERIAL PRIMARY KEY,
            address    VARCHAR(255) NOT NULL,
            state_abbr VARCHAR(255) NOT NULL,
            city       VARCHAR(255) NOT NULL,
            zipcode    VARCHAR(255) NOT NULL,
            avatar     VARCHAR(255) NOT NULL,
            user_id    INT          NOT NULL,
            created_at timestamp    NOT NULL DEFAULT NOW(),
            updated_at timestamp    NOT NULL DEFAULT NOW(),
            FOREIGN KEY (user_id) REFERENCES "user" (id) ON DELETE CASCADE
        );


        CREATE TABLE IF NOT EXISTS review
        (
            id         SERIAL PRIMARY KEY,
            status     VARCHAR(255) NOT NULL,
            user_id    INT          NOT NULL,
            item_id    INT          NOT NULL,
            comment    TEXT         NOT NULL,
            created_at TIMESTAMP    NOT NULL DEFAULT NOW(),
            updated_at TIMESTAMP    NOT NULL DEFAULT NOW(),
            FOREIGN KEY (user_id) REFERENCES "user" (id) ON DELETE CASCADE,
            FOREIGN KEY (item_id) REFERENCES item (id) ON DELETE CASCADE
        );

        create table IF NOT EXISTS media
        (
            id         serial primary key,
            src        varchar(255) NOT NULL,
            item_id    integer      NOT NULL,
            sort       integer               DEFAULT 1,
            created_at timestamp    NOT NULL DEFAULT NOW(),
            updated_at timestamp    NOT NULL DEFAULT NOW(),
            CONSTRAINT fk_item
                FOREIGN KEY (item_id)
                    REFERENCES item (id)
                    ON DELETE CASCADE
        );

        create table if not EXISTS page
        (
            id               serial primary key,
            title            varchar(255) NOT NULL,
            slug             varchar(255) NOT NULL unique,
            meta_description TEXT         NOT NULL,
            description      TEXT         NOT NULL,
            enabled          boolean               DEFAULT true,
            canonical_url    varchar(255) NOT NULL unique,
            created_at       timestamp    NOT NULL DEFAULT NOW(),
            updated_at       timestamp    NOT NULL DEFAULT NOW()
        );

        CREATE TABLE IF NOT EXISTS "basket"
        (
            id         SERIAL PRIMARY KEY,
            user_id    INT       NOT NULL,
            created_at timestamp NOT NULL DEFAULT NOW(),
            updated_at timestamp NOT NULL DEFAULT NOW(),
            FOREIGN KEY (user_id) REFERENCES "user" (id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS "basket_item"
        (
            id         SERIAL PRIMARY KEY,
            item_id    INT            NOT NULL,
            quantity   INT            NOT NULL,
            basket_id  INT            NOT NULL,
            price      decimal(10, 2) NOT NULL,
            created_at timestamp      NOT NULL DEFAULT NOW(),
            updated_at timestamp      NOT NULL DEFAULT NOW(),
            FOREIGN KEY (item_id) REFERENCES item (id) ON DELETE CASCADE,
            FOREIGN KEY (basket_id) REFERENCES basket (id) ON DELETE CASCADE,
            UNIQUE (item_id, basket_id)
        );

        CREATE TABLE IF NOT EXISTS "order"
        (
            id             SERIAL PRIMARY KEY,
            status         VARCHAR(255)   NOT NULL,
            basket_id      INT            NOT NULL UNIQUE,
            total          DECIMAL(10, 2) NOT NULL,
            total_ex_taxes DECIMAL(10, 2) NOT NULL,
            delivery_fees  DECIMAL(10, 2) NOT NULL,
            tax_rate       DECIMAL(10, 2) NOT NULL,
            taxes          DECIMAL(10, 2) NOT NULL,
            user_id        INT            NOT NULL,
            reference      VARCHAR(255)   NOT NULL,
            created_at     timestamp      NOT NULL DEFAULT NOW(),
            updated_at     timestamp      NOT NULL DEFAULT NOW(),
            address_id     INT            NOT NULL,
            returned       BOOLEAN        NOT NULL DEFAULT false,
            FOREIGN KEY (basket_id) REFERENCES "basket" (id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES "user" (id) ON DELETE CASCADE,
            FOREIGN KEY (address_id) REFERENCES address (id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS countries_ips
        (
            start_range  bigint NOT NULL,
            end_range    bigint NOT NULL,
            country_code text   NOT NULL,
            country_name text   NOT NULL,
            country_id   INT,
            PRIMARY KEY (start_range, end_range),
            FOREIGN KEY (country_id) REFERENCES country (id) ON DELETE CASCADE
        );

        CREATE INDEX idx_media_item_id_sort ON media (item_id, sort ASC);
        CREATE INDEX idx_item_slug ON item (slug);
        CREATE INDEX idx_shipping_rate_shipping_profile_id_country_id ON shipping_rate (shipping_profile_id, country_id);
        CREATE INDEX idx_shipping_rate_country_id ON shipping_rate (country_id);
        CREATE INDEX idx_item_shipping_profile_id ON item (shipping_profile_id);
        CREATE INDEX idx_countries_ips_country_id ON countries_ips (country_id);
        CREATE INDEX end_range_with_include_idx ON countries_ips USING btree (end_range ASC NULLS LAST) INCLUDE (start_range, country_id);

        CREATE INDEX idx_item_updated_at ON item (updated_at);
        CREATE INDEX idx_item_enabled ON item (enabled);
        CREATE INDEX idx_order_status ON "order" (status);

        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "item"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "media"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "page"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "user"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "shipping_profile"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "shipping_rate"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "country"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "order"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "basket_item"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "basket"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "address"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "review"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "countries_ips"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE OR REPLACE FUNCTION format_src(src TEXT, cloud_name TEXT)
            RETURNS TEXT AS
        $body$
        BEGIN
            RETURN 'https://' || cloud_name || '/' || src;
        END;
        $body$ LANGUAGE plpgsql;
    END
$$;
