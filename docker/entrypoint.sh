#!/bin/bash
set -e

DB_NAME=$(hostname)

cat > /app/config.json <<EOF
{
  "db_clients": [
    {
      "name": "default",
      "rdbms": "postgresql",
      "host": "/var/run/postgresql",
      "port": 5432,
      "dbname": "${DB_NAME}",
      "user": "${DB_NAME}",
      "passwd": "",
      "is_fast": true,
      "connection_number": 4
    },
    {
      "name": "default_not_fast",
      "rdbms": "postgresql",
      "host": "/var/run/postgresql",
      "port": 5432,
      "dbname": "${DB_NAME}",
      "user": "${DB_NAME}",
      "passwd": "",
      "is_fast": false,
      "connection_number": 4
    }
  ]
}
EOF

exec "$@"
