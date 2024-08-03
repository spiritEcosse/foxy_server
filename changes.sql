BEGIN;

ALTER table "user"
    add column first_name varchar(255);
ALTER table "user"
    add column last_name varchar(255);
ALTER table "user"
    add column birthday DATE default null;

-- UPDATE "user"
-- SET first_name = 'John',
--     last_name  = 'Doe',
--     birthday   = '1990-01-01';

ALTER table "user"
    drop column password;

ALTER table "user"
    alter column first_name set not null;
ALTER table "user"
    alter column last_name set not null;
Alter table "user"
    add column has_newsletter BOOLEAN NOT NULL DEFAULT false;
ALTER TABLE "user"
    ADD column is_admin BOOLEAN NOT NULL DEFAULT false;


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
       ('Argentina', 'AR');

UPDATE country
SET updated_at = now()
WHERE id = 2
   OR id = 1;

UPDATE countries_ips
SET country_id = country.id
FROM country
WHERE countries_ips.country_code = country.code;

COMMIT;
