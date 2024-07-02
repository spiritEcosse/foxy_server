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

INSERT INTO country (title, code)
VALUES ('Zimbabwe', 'ZW'),
       ('Zambia', 'ZM'),
       ('Yemen', 'YE'),
       ('Vietnam', 'VN'),
       ('Venezuela', 'VE'),
       ('Vatican City', 'VA'),
       ('Vanuatu', 'VU'),
       ('Uzbekistan', 'UZ'),
       ('Uruguay', 'UY'),
       ('United Kingdom', 'GB'),
       ('United Arab Emirates', 'AE'),
       ('Ukraine', 'UA'),
       ('Uganda', 'UG'),
       ('Tuvalu', 'TV'),
       ('Turkmenistan', 'TM'),
       ('Turkey', 'TR'),
       ('Tunisia', 'TN'),
       ('Trinidad and Tobago', 'TT'),
       ('Tonga', 'TO'),
       ('Togo', 'TG'),
       ('Timor-Leste', 'TL'),
       ('Thailand', 'TH'),
       ('Tanzania', 'TZ'),
       ('Tajikistan', 'TJ'),
       ('Taiwan', 'TW'),
       ('Syria', 'SY'),
       ('Switzerland', 'CH'),
       ('Sweden', 'SE'),
       ('Suriname', 'SR'),
       ('Sudan', 'SD'),
       ('Sri Lanka', 'LK'),
       ('South Sudan', 'SS'),
       ('South Africa', 'ZA'),
       ('Somalia', 'SO'),
       ('Solomon Islands', 'SB'),
       ('Slovenia', 'SI'),
       ('Slovakia', 'SK'),
       ('Singapore', 'SG'),
       ('Sierra Leone', 'SL'),
       ('Seychelles', 'SC'),
       ('Serbia', 'RS'),
       ('Senegal', 'SN'),
       ('Saudi Arabia', 'SA'),
       ('Sao Tome and Principe', 'ST'),
       ('San Marino', 'SM'),
       ('Samoa', 'WS'),
       ('Saint Vincent and the Grenadines', 'VC'),
       ('Saint Lucia', 'LC'),
       ('Saint Kitts and Nevis', 'KN'),
       ('Rwanda', 'RW'),
       ('Russia', 'RU'),
       ('Romania', 'RO'),
       ('Qatar', 'QA'),
       ('Portugal', 'PT'),
       ('Poland', 'PL'),
       ('Philippines', 'PH'),
       ('Peru', 'PE'),
       ('Paraguay', 'PY'),
       ('Papua New Guinea', 'PG'),
       ('Panama', 'PA'),
       ('Palestine', 'PS'),
       ('Palau', 'PW'),
       ('Pakistan', 'PK'),
       ('Oman', 'OM'),
       ('Norway', 'NO'),
       ('North Macedonia', 'MK'),
       ('Nigeria', 'NG'),
       ('Niger', 'NE'),
       ('Nicaragua', 'NI'),
       ('New Zealand', 'NZ'),
       ('Netherlands', 'NL'),
       ('Nepal', 'NP'),
       ('Nauru', 'NR'),
       ('Namibia', 'NA'),
       ('Myanmar', 'MM'),
       ('Mozambique', 'MZ'),
       ('Morocco', 'MA'),
       ('Montenegro', 'ME'),
       ('Mongolia', 'MN'),
       ('Monaco', 'MC'),
       ('Moldova', 'MD'),
       ('Micronesia', 'FM'),
       ('Mexico', 'MX'),
       ('Mauritius', 'MU'),
       ('Mauritania', 'MR'),
       ('Marshall Islands', 'MH'),
       ('Malta', 'MT'),
       ('Mali', 'ML'),
       ('Maldives', 'MV'),
       ('Malaysia', 'MY'),
       ('Malawi', 'MW'),
       ('Madagascar', 'MG'),
       ('Luxembourg', 'LU'),
       ('Lithuania', 'LT'),
       ('Liechtenstein', 'LI'),
       ('Libya', 'LY'),
       ('Liberia', 'LR'),
       ('Lesotho', 'LS'),
       ('Lebanon', 'LB'),
       ('Latvia', 'LV'),
       ('Laos', 'LA'),
       ('Kyrgyzstan', 'KG'),
       ('Kuwait', 'KW'),
       ('Kosovo', 'XK'),
       ('Korea, South', 'KR'),
       ('Korea, North', 'KP'),
       ('Kiribati', 'KI'),
       ('Kenya', 'KE'),
       ('Kazakhstan', 'KZ'),
       ('Jordan', 'JO'),
       ('Japan', 'JP'),
       ('Jamaica', 'JM'),
       ('Italy', 'IT'),
       ('Israel', 'IL'),
       ('Ireland', 'IE'),
       ('Iraq', 'IQ'),
       ('Iran', 'IR'),
       ('Indonesia', 'ID'),
       ('India', 'IN'),
       ('Iceland', 'IS'),
       ('Hungary', 'HU'),
       ('Honduras', 'HN'),
       ('Haiti', 'HT'),
       ('Guyana', 'GY'),
       ('Guinea-Bissau', 'GW'),
       ('Guinea', 'GN'),
       ('Guatemala', 'GT'),
       ('Grenada', 'GD'),
       ('Greece', 'GR'),
       ('Ghana', 'GH'),
       ('Germany', 'DE'),
       ('Georgia', 'GE'),
       ('Gambia', 'GM'),
       ('Gabon', 'GA'),
       ('France', 'FR'),
       ('Finland', 'FI'),
       ('Fiji', 'FJ'),
       ('Ethiopia', 'ET'),
       ('Eswatini', 'SZ'),
       ('Estonia', 'EE'),
       ('Eritrea', 'ER'),
       ('Equatorial Guinea', 'GQ'),
       ('El Salvador', 'SV'),
       ('Egypt', 'EG'),
       ('Ecuador', 'EC'),
       ('Dominican Republic', 'DO'),
       ('Dominica', 'DM'),
       ('Djibouti', 'DJ'),
       ('Denmark', 'DK'),
       ('Czech Republic', 'CZ'),
       ('Cyprus', 'CY'),
       ('Cuba', 'CU'),
       ('Croatia', 'HR'),
       ('Cote d''Ivoire', 'CI'),
       ('Costa Rica', 'CR'),
       ('Congo, Republic of the', 'CG'),
       ('Congo, Democratic Republic of the', 'CD'),
       ('Comoros', 'KM'),
       ('Colombia', 'CO'),
       ('China', 'CN'),
       ('Chile', 'CL'),
       ('Chad', 'TD'),
       ('Central African Republic', 'CF'),
       ('Canada', 'CA'),
       ('Cameroon', 'CM'),
       ('Cambodia', 'KH'),
       ('Cabo Verde', 'CV'),
       ('Burundi', 'BI'),
       ('Burkina Faso', 'BF'),
       ('Bulgaria', 'BG'),
       ('Brunei', 'BN'),
       ('Brazil', 'BR'),
       ('Botswana', 'BW'),
       ('Bosnia and Herzegovina', 'BA'),
       ('Bolivia', 'BO'),
       ('Bhutan', 'BT'),
       ('Benin', 'BJ'),
       ('Belize', 'BZ'),
       ('Belgium', 'BE'),
       ('Belarus', 'BY'),
       ('Barbados', 'BB'),
       ('Bangladesh', 'BD'),
       ('Bahrain', 'BH'),
       ('Bahamas', 'BS'),
       ('Azerbaijan', 'AZ'),
       ('Austria', 'AT'),
       ('Australia', 'AU'),
       ('Armenia', 'AM'),
       ('Argentina', 'AR'),
       ('Antigua and Barbuda', 'AG'),
       ('Angola', 'AO'),
       ('Andorra', 'AD'),
       ('Algeria', 'DZ'),
       ('Albania', 'AL'),
       ('Afghanistan', 'AF'),
       ('Spain', 'ES'),
       ('United States', 'US');

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
