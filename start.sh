#!/bin/bash

# Ensure the correct number of arguments are provided
if [ "$#" -ne 1 ]; then
    echo "Error: Illegal number of parameters. Please provide one of the following targets:"
    echo " 1. cooja"
    echo " 2. nrf52840"
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

# Check if the database exists, if not create it
if ! mysql -u"$DB_USER" -p"$DB_PASS" -h"$DB_HOST" -e "use $DB_NAME" &>/dev/null; then
    echo "Database $DB_NAME does not exist. Creating it..."
    if ! mysql -u"$DB_USER" -p"$DB_PASS" -h"$DB_HOST" -e "CREATE DATABASE $DB_NAME" &>/dev/null; then
        echo "Error: Failed to create database $DB_NAME"
        exit 1
    fi
    echo "Database $DB_NAME created successfully"
    echo ""
fi



# Truncate iot_nodes table if it exists, otherwise create it
if mysql -u"$DB_USER" -p"$DB_PASS" -h"$DB_HOST" "$DB_NAME" -e "SELECT 1 FROM iot_nodes LIMIT 1" &>/dev/null; then
    echo "Truncating table iot_nodes..."
    if ! mysql -u"$DB_USER" -p"$DB_PASS" -h"$DB_HOST" "$DB_NAME" -e "TRUNCATE TABLE iot_nodes" &>/dev/null; then
        echo "Error: Failed to truncate table iot_nodes"
        exit 1
    fi
    echo "Table iot_nodes truncated successfully"
else
    echo "Table iot_nodes does not exist. Creating it..."
    if ! mysql -u"$DB_USER" -p"$DB_PASS" -h"$DB_HOST" "$DB_NAME" -e "CREATE TABLE iot_nodes (
        ip VARCHAR(50) PRIMARY KEY,
        resource_exposed VARCHAR(255) NOT NULL
    )" &>/dev/null; then
        echo "Error: Failed to create table iot_nodes"
        exit 1
    fi
    echo "Table iot_nodes created successfully"
fi

echo ""

if ! mysql -u"$DB_USER" -p"$DB_PASS" -h"$DB_HOST" "$DB_NAME" -e "SELECT 1 FROM temphum_sensor LIMIT 1" &>/dev/null; then
    echo "Table temphum_sensor does not exist. Creating it..."
    if ! mysql -u"$DB_USER" -p"$DB_PASS" -h"$DB_HOST" "$DB_NAME" -e "CREATE TABLE temphum_sensor (
        id INT AUTO_INCREMENT PRIMARY KEY,
        temperature DOUBLE NOT NULL,
        humidity DOUBLE NOT NULL,
        timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )" &>/dev/null; then
        echo "Error: Failed to create table temphum_sensor"
        exit 1
    fi
    echo "Table temphum_sensor created successfully"
    echo ""
fi



if ! mysql -u"$DB_USER" -p"$DB_PASS" -h"$DB_HOST" "$DB_NAME" -e "SELECT 1 FROM co_sensor LIMIT 1" &>/dev/null; then
    echo "Table co_sensor does not exist. Creating it..."
    if ! mysql -u"$DB_USER" -p"$DB_PASS" -h"$DB_HOST" "$DB_NAME" -e "CREATE TABLE co_sensor (
        id INT AUTO_INCREMENT PRIMARY KEY,
        co DOUBLE NOT NULL,
        timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )" &>/dev/null; then
        echo "Error: Failed to create table co_sensor"
        exit 1
    fi
    echo "Table co_sensor created successfully"
    echo ""
fi



if ! mysql -u"$DB_USER" -p"$DB_PASS" -h"$DB_HOST" "$DB_NAME" -e "SELECT 1 FROM hvac_actuator LIMIT 1" &>/dev/null; then
    echo "Table hvac_actuator does not exist. Creating it..."
    if ! mysql -u"$DB_USER" -p"$DB_PASS" -h"$DB_HOST" "$DB_NAME" -e "CREATE TABLE hvac_actuator (
        id INT AUTO_INCREMENT PRIMARY KEY,
        status BOOLEAN NOT NULL,
        timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )" &>/dev/null; then
        echo "Error: Failed to create table hvac_actuator"
        exit 1
    fi
    echo "Table hvac_actuator created successfully"
    echo ""
fi

echo "------------------------------------------"
echo "Tables are ready in database $DB_NAME"
echo "------------------------------------------"

echo ""

cd Implementation/BorderRouter/

echo "Starting the BorderRouter with target ${TARGET}..."
echo ""

# Start the BorderRouter
if [ "$TARGET" == "nrf52840" ]; then
    if ! make TARGET=nrf52840 BOARD=dongle PORT=/dev/ttyACM0 connect-router; then
        echo "Error: Failed to start the BorderRouter with target ${TARGET}"
        echo ""
        exit 1
    fi
else
    if ! make TARGET=${TARGET} connect-router-cooja; then
        echo "Error: Failed to start the BorderRouter with target ${TARGET}"
        echo ""
        exit 1
    fi
fi

echo ""

cd ../..
