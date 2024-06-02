#!/bin/bash

# Define paths for each sensor/actuator
declare -A paths
paths["co"]="Implementation/Sensors/CO/"
paths["temperatureandhumidity"]="Implementation/Sensors/TemperatureAndHumidity/"
paths["movement"]="Implementation/Sensors/Movement/"
paths["hvac"]="Implementation/Actuators/HVAC/"
paths["vaultstatus"]="Implementation/Actuators/VaultStatus/"

# Check if the "distclean" option is passed
distclean=false
if [ "$1" == "distclean" ]; then
    echo "[DISTCLEAN] enabled"
    echo ""
    distclean=true
    shift # Remove the first argument so the rest of the script can process normally
fi

# Prompt the user for the number of sensors
echo "How many sensors do you want to flash? (Enter 'all' to flash all sensors)"
read num_sensors

# Initialize an array to store sensor names
sensor_names=()

# Function to flash a sensor
flash_sensor() {
    local sensor_name=$1
    local path=${paths[$sensor_name]}
    
    if [ -n "$path" ]; then
        echo "Flashing $sensor_name sensor..."
        cd $path
        if [ "$distclean" = true ]; then
            echo "Running make distclean for $sensor_name..."
            make distclean TARGET=nrf52840
        fi
        make TARGET=nrf52840 BOARD=dongle ${sensor_name}.dfu-upload PORT=/dev/ttyACM0
        cd - > /dev/null
    else
        echo "Invalid sensor name: $sensor_name"
    fi
}

# Function to flash all sensors
flash_all() {
    for sensor_name in "${!paths[@]}"; do
        read -p "Press Enter to flash the next sensor (${sensor_name})..."
        flash_sensor $sensor_name
    done
}

# Prompt the user to enter sensor names
if [ "$num_sensors" == "all" ]; then
    flash_all
else
    for ((i=1; i<=num_sensors; i++)); do
        echo "Enter the name of sensor $i:"
        read sensor_name
        sensor_names+=($sensor_name)
    done

    # Flash each sensor entered by the user
    for sensor_name in "${sensor_names[@]}"; do
        read -p "Press Enter to flash the next sensor (${sensor_name})..."
        flash_sensor $sensor_name
    done
fi
