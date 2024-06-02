#!/bin/bash

# Define paths for each sensor/actuator
declare -A paths
paths["co"]="Implementation/Sensors/CO/"
paths["temperatureandhumidity"]="Implementation/Sensors/TemperatureAndHumidity/"
paths["movement"]="Implementation/Sensors/Movement/"
paths["hvac"]="Implementation/Actuators/HVAC/"
paths["vaultstatus"]="Implementation/Actuators/VaultStatus/"

# Array of port numbers to be used
port_numbers=(1 2 3 4 5)

# Function to open a new terminal and execute the make login command
open_terminal() {
    local sensor_name=$1
    local port_number=$2
    local path=${paths[$sensor_name]}
    
    if [ -n "$path" ]; then
        echo "Opening terminal for $sensor_name on PORT=/dev/ttyACM${port_number}..."
        gnome-terminal -- bash -c "cd $path; make login TARGET=nrf52840 BOARD=dongle PORT=/dev/ttyACM${port_number}; exec bash"
    else
        echo "Invalid sensor name: $sensor_name"
    fi
}

# Iterate over each sensor and open a new terminal with a different port number
index=0
for sensor_name in "${!paths[@]}"; do
    open_terminal $sensor_name ${port_numbers[$index]}
    index=$((index + 1))
    # If there are more than 5 sensors, reset the index to reuse port numbers (optional based on requirements)
    if [ $index -ge ${#port_numbers[@]} ]; then
        index=0
    fi
done
