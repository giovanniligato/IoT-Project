#!/bin/bash

# Ensure the correct number of arguments are provided
if [ "$#" -ne 1 ]; then
    echo "Error: Illegal number of parameters. Please provide one of the following targets:"
    echo " 1. 'cooja'"
    echo " 2. 'nrf52840'"
    exit 1
fi

TARGET=$1

# Validate the provided target
if [ "$TARGET" != "cooja" ] && [ "$TARGET" != "nrf52840" ]; then
    echo "Error: Invalid target specified. Please provide 'cooja' or 'nrf52840'."
    exit 1
fi

# Database credentials
DB_USER="root"
DB_PASS="root"
DB_HOST="localhost"
DB_NAME="iot_voltvault"
TABLE_NAME="iot_nodes"

# Execute the truncate command and handle potential errors
if ! mysql -u"$DB_USER" -p"$DB_PASS" -h"$DB_HOST" "$DB_NAME" -e "TRUNCATE TABLE $TABLE_NAME;"; then
    echo "Error: Failed to truncate table $TABLE_NAME in database $DB_NAME"
    exit 1
fi
echo "Truncate executed on the database"

cd Implementation/BorderRouter/

echo "Starting the BorderRouter with target ${TARGET}"
# Start the BorderRouter
if ! make TARGET=${TARGET} connect-router-cooja; then
    echo "Error: Failed to start the BorderRouter with target ${TARGET}"
    exit 1
fi

cd ../..