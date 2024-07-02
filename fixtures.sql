DO
$$
    BEGIN
        -- Mock data for country
        INSERT INTO country (title, code)
        VALUES ('Country1', 'C1'),
               ('Country2', 'C2');

-- Mock data for shipping_profile
        INSERT INTO shipping_profile (title, country_id, postal_code, processing_time, shipping_upgrade_cost)
        VALUES ('Profile1', 1, '12345', 2, 10.00),
               ('Profile2', 2, '67890', 3, 15.00);

-- Mock data for item
        INSERT INTO item (title, meta_description, description, shipping_profile_id, price, slug, enabled)
        VALUES ('Item1', 'Meta1', 'Description1', 1, 100.00, 'item1', true),
               ('Item2', 'Meta2', 'Description2', 2, 200.00, 'item2', false);

-- Mock data for user
        INSERT INTO "user" (email, password, first_name, last_name, birthday, has_newsletter)
        VALUES ('user1@example.com', 'password1', 'User', 'One', '2000-01-01', true),
               ('user2@example.com', 'password2', 'User', 'Two', '2000-02-02', false);

-- Mock data for address
        INSERT INTO "address" (address, country_id, city, zipcode, user_id)
        VALUES ('Address1', 1, 'City1', '12345', 1),
               ('Address2', 2, 'City2', '67890', 2);

-- Mock data for review
        INSERT INTO review (status, user_id, item_id, comment)
        VALUES ('Approved', 1, 1, 'Great product!'),
               ('Pending', 2, 2, 'Good quality.');

-- Mock data for media
        INSERT INTO media (src, item_id, sort)
        VALUES ('media1.png', 1, 1),
               ('media2.png', 2, 2);

-- Mock data for page
        INSERT INTO page (title, slug, meta_description, description, enabled, canonical_url)
        VALUES ('Page1', 'page1', 'Meta1', 'Description1', true, 'www.example.com/page1'),
               ('Page2', 'page2', 'Meta2', 'Description2', false, 'www.example.com/page2');

-- Mock data for shipping_rate
        INSERT INTO shipping_rate (shipping_profile_id, country_id, delivery_days_min, delivery_days_max)
        VALUES (1, 1, 1, 5),
               (2, 2, 2, 6);

-- Mock data for basket
        INSERT INTO "basket" (user_id)
        VALUES (1),
               (2);

-- Mock data for basket_item
        INSERT INTO "basket_item" (item_id, quantity, basket_id, price)
        VALUES (1, 1, 1, 100.00),
               (2, 2, 2, 200.00);

-- Mock data for order
        INSERT INTO "order" (status, basket_id, total, total_ex_taxes, delivery_fees, tax_rate, taxes, user_id,
                             reference,
                             address_id, returned)
        VALUES ('Completed', 1, 100.00, 90.00, 10.00, 0.10, 10.00, 1, 'REF1', 1, false),
               ('Pending', 2, 200.00, 180.00, 20.00, 0.10, 20.00, 2, 'REF2', 2, false);

-- Mock data for countries_ips
        INSERT INTO countries_ips (start_range, end_range, country_code, country_name, country_id)
        VALUES (1, 100, 'US', 'United States of America', 1),
               (101, 200, 'ES', 'Spain', 2);
    END
$$;
