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


create table IF NOT EXISTS item (
                                    item_id serial primary key,
                                    title varchar(255) NOT NULL,
                                    meta_description TEXT NOT NULL,
                                    description TEXT NOT NULL
    );

create table IF NOT EXISTS media (
                                     media_id serial primary key,
                                     src varchar(255) NOT NULL,
    item_id integer NOT NULL,
    thumb varchar(255),
    sort integer DEFAULT 1,
    CONSTRAINT fk_item
    FOREIGN KEY(item_id)
    REFERENCES item(item_id)
    ON DELETE CASCADE
    );

create table if not EXISTS page (
                                    page_id serial primary key,
                                    title varchar(255) NOT NULL,
                                    meta_description TEXT NOT NULL,
                                    description TEXT NOT NULL,
                                    canonical_url varchar(255) NOT NULL
);
