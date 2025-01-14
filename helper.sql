DO
$$
    BEGIN


        CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

        CREATE OR REPLACE FUNCTION GetValidPage(RequestedPage INT, PageSize INT, TotalCount INT, OUT ValidPage INT)
        AS
        $body$
        DECLARE
            TotalPages INT;
        BEGIN
            TotalPages := CEIL(TotalCount::FLOAT / PageSize);
            IF RequestedPage > TotalPages THEN
                ValidPage := 1; -- Set to the first page
            ELSE
                ValidPage := RequestedPage;
            END IF;
        END;
        $body$
            LANGUAGE plpgsql;

        CREATE OR REPLACE FUNCTION trigger_set_timestamp()
            RETURNS TRIGGER AS
        $body$
        BEGIN
            NEW.updated_at = NOW();
            RETURN NEW;
        END;
        $body$
            LANGUAGE plpgsql;

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
        $body$
            LANGUAGE plpgsql;

        CREATE TABLE IF NOT EXISTS country
        (
            id         SERIAL PRIMARY KEY,
            title      VARCHAR(255) NOT NULL UNIQUE,
            code       VARCHAR(255) NOT NULL UNIQUE,
            created_at TIMESTAMP    NOT NULL DEFAULT NOW(),
            updated_at TIMESTAMP    NOT NULL DEFAULT NOW()
        );

        CREATE TABLE IF NOT EXISTS shipping_profile
        (
            id                    SERIAL PRIMARY KEY,
            title                 VARCHAR(255) NOT NULL,
            country_id            INT          NOT NULL,
            postal_code           VARCHAR(255) NOT NULL,
            processing_time       INT          NOT NULL,
            shipping_upgrade_cost DECIMAL(10, 2)        DEFAULT 0,
            created_at            TIMESTAMP    NOT NULL DEFAULT NOW(),
            updated_at            TIMESTAMP    NOT NULL DEFAULT NOW(),
            FOREIGN KEY (country_id) REFERENCES country (id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS item
        (
            id                  SERIAL PRIMARY KEY,
            title               VARCHAR(255) NOT NULL,
            meta_description    TEXT         NOT NULL,
            description         TEXT         NOT NULL,
            shipping_profile_id INT          NOT NULL,
            price               DECIMAL(10, 2)        DEFAULT 0,
            slug                VARCHAR(255) NOT NULL UNIQUE,
            enabled             BOOLEAN               DEFAULT true,
            created_at          TIMESTAMP    NOT NULL DEFAULT NOW(),
            updated_at          TIMESTAMP    NOT NULL DEFAULT NOW(),
            FOREIGN KEY (shipping_profile_id) REFERENCES shipping_profile (id) ON DELETE CASCADE
        );

        IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'social_media_type') THEN
            CREATE TYPE social_media_type AS ENUM
                (
                    'Facebook',
                    'Twitter',
                    'Instagram',
                    'Pinterest',
                    'Quora',
                    'YouTube',
                    'LinkedIn',
                    'TikTok',
                    'Snapchat'
                    );
        END IF;

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

        --         INSERT INTO country (title, code)
--         VALUES ('United States of America', 'US'),
--                ('Spain', 'ES');

        CREATE TABLE IF NOT EXISTS shipping_rate
        (
            id                  SERIAL PRIMARY KEY,
            shipping_profile_id INT       NOT NULL,
            country_id          INT,
            delivery_days_min   INT       NOT NULL,
            delivery_days_max   INT       NOT NULL,
            created_at          TIMESTAMP NOT NULL DEFAULT NOW(),
            updated_at          TIMESTAMP NOT NULL DEFAULT NOW(),
            FOREIGN KEY (shipping_profile_id) REFERENCES shipping_profile (id) ON DELETE CASCADE,
            FOREIGN KEY (country_id) REFERENCES country (id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS "user"
        (
            id             SERIAL PRIMARY KEY,
            email          VARCHAR(255) NOT NULL UNIQUE,
            password       VARCHAR(255)          DEFAULT '',
            first_name     VARCHAR(255) NOT NULL,
            last_name      VARCHAR(255) NOT NULL,
            is_admin       BOOLEAN      NOT NULL DEFAULT false,
            birthday       DATE                  DEFAULT NULL,
            has_newsletter BOOLEAN      NOT NULL DEFAULT false,
            created_at     TIMESTAMP    NOT NULL DEFAULT NOW(),
            updated_at     TIMESTAMP    NOT NULL DEFAULT NOW()
        );

        CREATE TABLE IF NOT EXISTS "address"
        (
            id         SERIAL PRIMARY KEY,
            address    VARCHAR(255) NOT NULL,
            city       VARCHAR(255) NOT NULL,
            zipcode    VARCHAR(255) NOT NULL,
            user_id    INT          NOT NULL,
            country_id INT          NOT NULL,
            created_at TIMESTAMP    NOT NULL DEFAULT NOW(),
            updated_at TIMESTAMP    NOT NULL DEFAULT NOW(),
            FOREIGN KEY (user_id) REFERENCES "user" (id) ON DELETE CASCADE,
            FOREIGN KEY (country_id) REFERENCES "country" (id) ON DELETE CASCADE
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

        IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'media_type') THEN
            CREATE TYPE media_type AS ENUM
                (
                    'video',
                    'image'
                    );
        END IF;

        CREATE TABLE IF NOT EXISTS media
        (
            id           SERIAL PRIMARY KEY,
            src          VARCHAR(255) NOT NULL,
            type         media_type   NOT NULL,
            item_id      INT          NOT NULL,
            sort         INT                   DEFAULT 1,
            content_type VARCHAR(20)  NOT NULL,
            created_at   TIMESTAMP    NOT NULL DEFAULT NOW(),
            updated_at   TIMESTAMP    NOT NULL DEFAULT NOW(),
            FOREIGN KEY (item_id) REFERENCES item (id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS page
        (
            id               SERIAL PRIMARY KEY,
            title            VARCHAR(255) NOT NULL,
            slug             VARCHAR(255) NOT NULL UNIQUE,
            meta_description TEXT         NOT NULL,
            description      TEXT         NOT NULL,
            enabled          BOOLEAN               DEFAULT true,
            canonical_url    VARCHAR(255) NOT NULL UNIQUE,
            created_at       TIMESTAMP    NOT NULL DEFAULT NOW(),
            updated_at       TIMESTAMP    NOT NULL DEFAULT NOW()
        );

        CREATE TABLE IF NOT EXISTS "basket"
        (
            id         SERIAL PRIMARY KEY,
            user_id    INT,
            in_use     BOOLEAN   NOT NULL DEFAULT true,
            created_at TIMESTAMP NOT NULL DEFAULT NOW(),
            updated_at TIMESTAMP NOT NULL DEFAULT NOW(),
            FOREIGN KEY (user_id) REFERENCES "user" (id) ON DELETE SET NULL
        );

        CREATE TABLE IF NOT EXISTS "basket_item"
        (
            id         SERIAL PRIMARY KEY,
            item_id    INT            NOT NULL,
            quantity   INT            NOT NULL,
            basket_id  INT            NOT NULL,
            price      DECIMAL(10, 2) NOT NULL,
            created_at TIMESTAMP      NOT NULL DEFAULT NOW(),
            updated_at TIMESTAMP      NOT NULL DEFAULT NOW(),
            FOREIGN KEY (item_id) REFERENCES item (id) ON DELETE CASCADE,
            FOREIGN KEY (basket_id) REFERENCES basket (id) ON DELETE CASCADE,
            UNIQUE (item_id, basket_id)
        );

        CREATE UNIQUE INDEX IF NOT EXISTS idx_basket_item_unique ON "basket_item" (item_id, basket_id);

        IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'order_status_type') THEN
            CREATE TYPE order_status_type AS ENUM
                ('Completed', 'Ordered', 'Processing', 'Shipped', 'Delivered', 'Returned', 'Cancelled', 'Refunded');
        END IF;

        CREATE TABLE IF NOT EXISTS "order"
        (
            id             SERIAL PRIMARY KEY,
            status         order_status_type NOT NULL,
            basket_id      INT               NOT NULL UNIQUE,
            total          DECIMAL(10, 2)    NOT NULL,
            total_ex_taxes DECIMAL(10, 2)    NOT NULL,
            tax_rate       DECIMAL(10, 2)    NOT NULL,
            taxes          DECIMAL(10, 2)    NOT NULL,
            user_id        INT,
            reference      UUID              NOT NULL DEFAULT uuid_generate_v4() UNIQUE,
            created_at     TIMESTAMP         NOT NULL DEFAULT NOW(),
            updated_at     TIMESTAMP         NOT NULL DEFAULT NOW(),
            address_id     INT,
            returned       BOOLEAN           NOT NULL DEFAULT false,
            FOREIGN KEY (basket_id) REFERENCES "basket" (id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES "user" (id) ON DELETE SET NULL,
            FOREIGN KEY (address_id) REFERENCES address (id) ON DELETE SET NULL
        );

        CREATE TABLE IF NOT EXISTS countries_ips
        (
            start_range  BIGINT NOT NULL,
            end_range    BIGINT NOT NULL,
            country_code TEXT   NOT NULL,
            country_name TEXT   NOT NULL,
            country_id   INT,
            PRIMARY KEY (start_range, end_range),
            FOREIGN KEY (country_id) REFERENCES country (id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS social_media
        (
            id          SERIAL PRIMARY KEY,
            title       social_media_type NOT NULL,
            item_id     INT               NOT NULL,
            external_id VARCHAR(255)      NOT NULL,
            created_at  TIMESTAMP         NOT NULL DEFAULT NOW(),
            updated_at  TIMESTAMP         NOT NULL DEFAULT NOW(),
            FOREIGN KEY (item_id) REFERENCES item (id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS "financial_details"
        (
            id                  SERIAL PRIMARY KEY,
            tax_rate            DECIMAL(10, 2) NOT NULL,
            gateway             VARCHAR(255)   NOT NULL,
            gateway_merchant_id VARCHAR(255)   NOT NULL,
            merchant_id         VARCHAR(255)   NOT NULL,
            merchant_name       VARCHAR(255)   NOT NULL,
            created_at          TIMESTAMP      NOT NULL DEFAULT NOW(),
            updated_at          TIMESTAMP      NOT NULL DEFAULT NOW()
        );

        CREATE INDEX IF NOT EXISTS idx_media_item_id_sort ON media (item_id, sort ASC);
        CREATE INDEX IF NOT EXISTS idx_item_slug ON item (slug);
        CREATE INDEX IF NOT EXISTS idx_shipping_rate_shipping_profile_id_country_id ON shipping_rate (shipping_profile_id, country_id);
        CREATE INDEX IF NOT EXISTS idx_shipping_rate_country_id ON shipping_rate (country_id);
        CREATE INDEX IF NOT EXISTS idx_item_shipping_profile_id ON item (shipping_profile_id);
        CREATE INDEX IF NOT EXISTS idx_countries_ips_country_id ON countries_ips (country_id);
        CREATE INDEX IF NOT EXISTS end_range_with_include_idx ON countries_ips USING btree (end_range ASC NULLS LAST) INCLUDE (start_range, country_id);
        CREATE INDEX IF NOT EXISTS idx_item_updated_at ON item (updated_at);
        CREATE INDEX IF NOT EXISTS idx_item_enabled ON item (enabled);
        CREATE INDEX IF NOT EXISTS idx_order_status ON "order" (status);
        CREATE INDEX IF NOT EXISTS idx_item_id ON social_media (item_id);
        CREATE INDEX IF NOT EXISTS idx_item_id_media ON media (item_id);
        CREATE INDEX IF NOT EXISTS idx_item_id_tag ON tag (item_id);
        CREATE INDEX IF NOT EXISTS idx_item_media ON media (item_id, type);

        DROP TRIGGER IF EXISTS set_timestamp ON "item";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "item"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "media";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "media"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "page";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "page"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "user";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "user"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "shipping_profile";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "shipping_profile"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "shipping_rate";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "shipping_rate"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "country";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "country"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "order";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "order"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "basket_item";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "basket_item"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "basket";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "basket"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "address";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "address"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "review";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "review"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "countries_ips";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "countries_ips"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "financial_details";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "financial_details"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        DROP TRIGGER IF EXISTS set_timestamp ON "tag";
        CREATE TRIGGER set_timestamp
            BEFORE UPDATE
            ON "tag"
            FOR EACH ROW
        EXECUTE PROCEDURE trigger_set_timestamp();

        CREATE OR REPLACE FUNCTION format_src(src TEXT, cloud_name TEXT)
            RETURNS TEXT AS
        $body$
        BEGIN
            RETURN 'https://' || cloud_name || '/' || src;
        END;
        $body$
            LANGUAGE plpgsql;

        CREATE OR REPLACE FUNCTION format_social_url(external_id TEXT, social_media_title TEXT)
            RETURNS TEXT AS
        $body$
        DECLARE
            base_url TEXT;
        BEGIN
            CASE social_media_title
                WHEN 'Twitter' THEN base_url := 'https://x.com/faithfishart/status/';
                ELSE base_url := ''; -- Default or error case
                END CASE;
            RETURN base_url || external_id;
        END;
        $body$
            LANGUAGE plpgsql;

    END

$$;
