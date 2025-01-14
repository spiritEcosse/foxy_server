#!/bin/bash
set -ex

PS4='Line ${LINENO}: '

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

HELPER_SQL_FILE="../helper.sql"
FIXTURES_SQL_FILE="../fixtures.sql"

# Database connection parameters - replace with your actual values
DB_NAME="foxy_tests"
DB_USER="foxy"
export PGPASSWORD=foxy
if [[ -z ${DB_HOST+x} ]]; then
    DB_HOST="localhost"
fi
if [[ -z ${DB_PORT+x} ]]; then
    DB_PORT="5432"
fi

# Function to execute SQL command with error handling
execute_sql() {
    local command=$1
    echo -e "${YELLOW}Executing: $command${NC}"
    psql -h $DB_HOST -U $DB_USER -p $DB_PORT -d $DB_NAME -c "$command"
}

# Main script
echo "Starting database setup..."

# List of tables to process
TABLES=($(grep -i "CREATE TABLE" "$HELPER_SQL_FILE" | \
         sed -n 's/.*CREATE TABLE\s\+\(IF NOT EXISTS\s\+\)\?\([\"]\?\([^\"\ ]\+\)[\"]\?\).*/\2/pi' | \
         sort -u))

# 1. Drop existing tables
DROP_COMMAND="DROP TABLE IF EXISTS $(IFS=,; echo "${TABLES[*]}") CASCADE;"
execute_sql "$DROP_COMMAND"

# 2. Execute helper.sql
echo -e "\nExecuting helper.sql..."
if [ -f "$HELPER_SQL_FILE" ]; then
    psql -h $DB_HOST -U $DB_USER -p $DB_PORT -d $DB_NAME -f "$HELPER_SQL_FILE"
else
    echo -e "${RED}$HELPER_SQL_FILE file not found${NC}"
    return 1
fi

# 3. Execute fixtures.sql
echo -e "\nExecuting fixtures.sql..."
if [ -f "$FIXTURES_SQL_FILE" ]; then
    psql -h $DB_HOST -U $DB_USER -p $DB_PORT -d $DB_NAME -f "$FIXTURES_SQL_FILE"
else
    echo -e "${RED}$FIXTURES_SQL_FILE file not found${NC}"
    return 1
fi

echo -e "${GREEN}Database setup completed successfully${NC}"
