CREATE OR REPLACE FUNCTION format_social_url(external_id TEXT, social_media_title TEXT)
    RETURNS TEXT AS
$body$
DECLARE
    base_url TEXT;
BEGIN
    CASE social_media_title
        WHEN 'Twitter' THEN base_url := 'https://x.com/faithfishart/status/';
        WHEN 'YouTube' THEN base_url := 'https://www.youtube.com/watch?v=';
        WHEN 'Pinterest' THEN base_url := 'https://pinterest.com/pin/';
        ELSE base_url := ''; -- Default or error case
        END CASE;

    RETURN base_url || external_id;
END;
$body$
    LANGUAGE plpgsql;

