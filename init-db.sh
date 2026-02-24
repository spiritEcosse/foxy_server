#!/bin/bash
set -e

echo "Creating user '${DB_NAME}' and database '${DB_NAME}'..."
psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" <<-EOSQL
    CREATE USER ${DB_NAME} WITH PASSWORD '${DB_NAME}';
    CREATE DATABASE ${DB_NAME} OWNER ${DB_NAME};
EOSQL

echo "Done: ${DB_NAME} — schema will be applied by Atlas"
