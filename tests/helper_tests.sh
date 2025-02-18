#!/bin/bash
set -e  # Stop on any error
set -o pipefail  # Ensure errors in pipelines stop execution

PS4='Line ${LINENO}: '

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

HELPER_SQL_FILE="../helper.sql"
FIXTURES_SQL_FILE="../fixtures.sql"

# Database connection parameters
DB_NAME="foxy_tests"
DB_USER="foxy"
export PGPASSWORD=foxy
DB_HOST="${DB_HOST:-localhost}"
DB_PORT="${DB_PORT:-5432}"

# Function to execute SQL commands
execute_sql() {
    local sql="$1"
    echo -e "${YELLOW}Executing: $sql${NC}"
    echo "$sql" | psql -h "$DB_HOST" -U "$DB_USER" -p "$DB_PORT" -d "$DB_NAME"
}

# Function to execute SQL files
execute_sql_file() {
    local file="$1"
    echo -e "${YELLOW}Executing file: $file${NC}"
    psql -h "$DB_HOST" -U "$DB_USER" -p "$DB_PORT" -d "$DB_NAME" --set ON_ERROR_STOP=1 -f "$file"
}

echo "Starting database setup..."

# Get tables from helper.sql and drop them
TABLES=($(grep -i "CREATE TABLE" "$HELPER_SQL_FILE" | sed -n 's/.*CREATE TABLE\s\+\(IF NOT EXISTS\s\+\)\?\([\"]\?\([^\"\ ]\+\)[\"]\?\).*/\2/pi' | sort -u))
[[ ${#TABLES[@]} -gt 0 ]] && execute_sql "DROP TABLE IF EXISTS $(IFS=,; echo "${TABLES[*]}") CASCADE;"

# Execute SQL files
execute_sql_file "$HELPER_SQL_FILE"
execute_sql_file "$FIXTURES_SQL_FILE"

echo -e "${GREEN}Database setup completed successfully${NC}"
