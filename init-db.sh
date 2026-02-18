#!/bin/bash
set -e

DATABASES="foxy_prod foxy_dev"

for db in $DATABASES; do
    echo "Creating user '$db' and database '$db'..."
    psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" <<-EOSQL
        CREATE USER ${db} WITH PASSWORD '${db}';
        CREATE DATABASE ${db} OWNER ${db};
EOSQL

    echo "Running schema on '$db'..."
    psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$db" \
        < /docker-entrypoint-initdb.d/helper.sql

    echo "Done: $db"
done
