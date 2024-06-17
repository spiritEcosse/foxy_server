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

ALTER table "user"
    alter column first_name set not null;
ALTER table "user"
    alter column last_name set not null;
ALTER table "user"
    alter column birthday set not null;
Alter table "user"
    add column has_newsletter BOOLEAN NOT NULL DEFAULT false;

CREATE TABLE IF NOT EXISTS review
(
    id
    SERIAL
    PRIMARY
    KEY,
    status
    VARCHAR
(
    255
) NOT NULL,
    user_id INT NOT NULL,
    item_id INT NOT NULL,
    comment TEXT NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW
(
),
    updated_at TIMESTAMP NOT NULL DEFAULT NOW
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
