commit;
ALTER table "user"
    add column first_name varchar(255);
ALTER table "user"
    add column last_name varchar(255);
ALTER table "user"
    add column birthday DATE;

-- UPDATE "user"
-- SET first_name = 'John',
--     last_name  = 'Doe',
--     birthday   = '1990-01-01';

CREATE UNIQUE INDEX idx_basket_item_unique
    ON "basket_item" (item_id, basket_id);

ALTER table "user"
    alter column password set default '';

ALTER table "user"
    alter column first_name set not null;
ALTER table "user"
    alter column last_name set not null;
ALTER TABLE "user"
    ALTER COLUMN birthday DROP NOT NULL;
ALTER table "user"
    alter column birthday set default null;
Alter table "user"
    add column has_newsletter BOOLEAN NOT NULL DEFAULT false;

CREATE TABLE IF NOT EXISTS review
(
    id
               SERIAL
        PRIMARY
            KEY,
    status
               VARCHAR(255) NOT NULL,
    user_id    INT          NOT NULL,
    item_id    INT          NOT NULL,
    comment    TEXT         NOT NULL,
    created_at TIMESTAMP    NOT NULL DEFAULT NOW
                                             (
                                             ),
    updated_at TIMESTAMP    NOT NULL DEFAULT NOW
                                             (
                                             ),
    FOREIGN KEY
        (
         user_id
            ) REFERENCES "user"
        (
         id
            ),
    FOREIGN KEY
        (
         item_id
            ) REFERENCES item
        (
         id
            )
);
Alter table "order"
    add column returned BOOLEAN NOT NULL DEFAULT false;
commit;

-- truncate table "country", "shipping_profile", "shipping_rate", "address", "order";
truncate table "country";
INSERT INTO country (title, code)
VALUES ('Vietnam', 'VN'),
       ('United Kingdom', 'GB'),
       ('Ukraine', 'UA'),
       ('Türkiye', 'TR'),
       ('Thailand', 'TH'),
       ('Switzerland', 'CH'),
       ('Sweden', 'SE'),
       ('South Africa', 'ZA'),
       ('Slovenia', 'SI'),
       ('Slovakia', 'SK'),
       ('Singapore', 'SG'),
       ('Romania', 'RO'),
       ('Portugal', 'PT'),
       ('Poland', 'PL'),
       ('Philippines', 'PH'),
       ('Peru', 'PE'),
       ('Norway', 'NO'),
       ('New Zealand', 'NZ'),
       ('Netherlands', 'NL'),
       ('Morocco', 'MA'),
       ('Mexico', 'MX'),
       ('Malta', 'MT'),
       ('Malaysia', 'MY'),
       ('Luxembourg', 'LU'),
       ('Lithuania', 'LT'),
       ('Latvia', 'LV'),
       ('Japan', 'JP'),
       ('Italy', 'IT'),
       ('Israel', 'IL'),
       ('Ireland', 'IE'),
       ('Indonesia', 'ID'),
       ('India', 'IN'),
       ('Hungary', 'HU'),
       ('Hong Kong', 'HK'),
       ('Greece', 'GR'),
       ('Germany', 'DE'),
       ('France', 'FR'),
       ('Finland', 'FI'),
       ('Estonia', 'EE'),
       ('Denmark', 'DK'),
       ('Czech Republic', 'CZ'),
       ('Cyprus', 'CY'),
       ('Croatia', 'HR'),
       ('China', 'CN'),
       ('Chile', 'CL'),
       ('Canada', 'CA'),
       ('Bulgaria', 'BG'),
       ('Belgium', 'BE'),
       ('Austria', 'AT'),
       ('Australia', 'AU'),
       ('Argentina', 'AR'),
       ('Spain', 'ES'),
       ('United States', 'US');

UPDATE countries_ips
SET country_id = country.id
FROM country
WHERE countries_ips.country_code = country.code;

alter table basket
    add column in_use BOOLEAN NOT NULL DEFAULT true;

ALTER table "address"
    drop column avatar;
ALTER table "address"
    drop column state_abbr;

ALTER TABLE "address"
    ADD COLUMN country_id INT NULL,
    ADD FOREIGN KEY (country_id) REFERENCES "country" (id) ON DELETE CASCADE;

UPDATE "address"
SET country_id = 1;
ALTER table "address"
    alter column country_id set not null;

do
$$
    BEGIN
        CREATE OR REPLACE FUNCTION format_src(src TEXT, cloud_name TEXT)
            RETURNS TEXT AS
        $body$
        BEGIN
            RETURN 'https://' || cloud_name || '/' || src;
        END;
        $body$ LANGUAGE plpgsql;
    END
$$;
