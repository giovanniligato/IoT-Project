# IoT Project - VoltVault

## Overview

This repository contains the source code, configuration files, and resources for the VoltVault IoT project. VoltVault is designed to ensure the safety and efficiency of an industrial battery room through continuous monitoring and automated responses. This README will guide you through the structure of the repository and provide instructions on how to get started.

## Project Structure

The project is organized into the following directories:

- `Implementation/`: Contains the implementation of sensors, actuators, simulation scripts, and utility tools.
  - `Sensors/`: Source code and configuration files for various sensors.
    - `CO/`: Carbon Monoxide sensor.
      - `co.c`: Main source file for the CO sensor.
      - `Makefile`: Build configuration file.
      - `project-conf.h`: Project-specific configuration header.
      - `resources/res-co.c`: Resource file for the CO sensor.
    - `Movement/`: Movement sensor.
      - `movement.c`: Main source file for the movement sensor.
      - `Makefile`: Build configuration file.
      - `project-conf.h`: Project-specific configuration header.
      - `resources/res-movement.c`: Resource file for the movement sensor.
    - `TemperatureAndHumidity/`: Temperature and Humidity sensor.
      - `temperatureandhumidity.c`: Main source file for the temperature and humidity sensor.
      - `Makefile`: Build configuration file.
      - `project-conf.h`: Project-specific configuration header.
      - `resources/res-temperatureandhumidity.c`: Resource file for the temperature and humidity sensor.
  - `Actuators/`: Source code for various actuators.
    - `VaultStatus/`: Outputs the status of the battery room.
      - `vaultstatus.c`: Main source file for the automatic door actuator.
      - `Makefile`: Build configuration file.
      - `project-conf.h`: Project-specific configuration header.
      - `resources/res-vaultstatus.c`: Resource file for the automatic door actuator.
    - `HVAC/`: Controls the HVAC system.
      - `hvac.c`: Main source file for the HVAC actuator.
      - `Makefile`: Build configuration file.
      - `project-conf.h`: Project-specific configuration header.
      - `resources/res-hvac.c`: Resource file for the HVAC actuator.
  - `Simulation/`: Simulation configuration and scripts.
    - `simulation.csc`: Cooja simulation script.
  - `Utility/`: Utility tools including debug tools and JSON SenML library.
    - `Debug/`: Debugging utilities.
      - `debug_sleep.c`: Source file for debug sleep functionality.
      - `debug_sleep.h`: Header file for debug sleep functionality.
    - `JSON_SenML/`: JSON SenML library.
      - `json-senml.c`: Source file for JSON SenML functionality.
      - `json-senml.h`: Header file for JSON SenML functionality.
    - `RandomNumberGenerator/`: Random number generator utility.
      - `random-number-generator.c`: Source file for random number generator.
      - `random-number-generator.h`: Header file for random number generator.
- `MachineLearning/`: Machine learning scripts and data.
  - `iot_telemetry_data.csv`: Dataset for IoT telemetry data.
  - `machine_learning.h`: Header file for machine learning functionality.
  - `training.ipynb`: Jupyter notebook for training machine learning models.
- `JavaApplication/`: Contains the Java application for user interaction and data processing.
  - `src/`: Source code for the Java application.
  - `pom.xml`: Maven configuration file for the Java application.
- `BorderRouter/`: Configuration and scripts for the border router.
  - `border-router.c`: Source file for the border router.
  - `Makefile`: Build configuration file.

## Installation

1. Clone the repository alongside Contiki-NG:
    ```bash
    git clone https://github.com/yourusername/IoT-Project.git
    git clone https://github.com/contiki-ng/contiki-ng.git
    cd IoT-Project
    ```

## Usage

### Simulation

To run the simulation, navigate to the `Simulation` directory and use Cooja:

1. Open Cooja from the Contiki-NG tools:
    ```bash
    cd contiki-ng/tools/cooja
    ./gradlew run
    ```
2. Load the simulation script.
    ```bash
    File -> Open simulation -> Select `simulation.csc`
    ```

### Flashing to nRF52840

To flash the project to the nRF52840 dongle:

1. Use the `flash.sh` script:
    ```bash
    ./flash.sh
    ```
2. Use the `login.sh` script to connect to the device:
    ```bash
    ./login.sh
    ```

## Border Router

The Border Router provides connectivity between the wireless sensor network (WSN) and the internet, enabling remote access and control of the system. To start the Border Router, use the `start.sh` script located in the root directory of the repository. This script also handles database creation and table setup.

This script ensures the correct setup of the MySQL database and starts the Border Router with the specified target (cooja or nrf52840). Ensure that the database credentials match your local configuration (predefined ones are `root` for username and `root` for password).

## Sensors

### CO Sensor
- **File**: `co.c`
- **Functionality**: Monitors the level of carbon monoxide and triggers alerts when dangerous levels are detected.
- **Resources**: `res-co.c` provides RESTful API access to sensor data.

### Movement Sensor
- **File**: `movement.c`
- **Functionality**: Detects movement and logs activity.
- **Resources**: `res-movement.c` provides RESTful API access to sensor data.

### Temperature and Humidity Sensor
- **File**: `temperatureandhumidity.c`
- **Functionality**: Monitors temperature and humidity levels.
- **Resources**: `res-temperatureandhumidity.c` provides RESTful API access to sensor data.

## Actuators

### Vault Status
- **File**: `vaultstatus.c`
- **Functionality**: Controls the automatic door, locking and unlocking based on room safety status.

### HVAC System
- **File**: `hvac.c`
- **Functionality**: Regulates the environmental conditions in the room based on sensor inputs.

## Utility

### JSON SenML Library
- **Files**: `json-senml.c`, `json-senml.h`
- **Functionality**: Implements JSON SenML format for sensor data representation.

### Random Number Generator
- **Files**: `random-number-generator.c`, `random-number-generator.h`
- **Functionality**: Provides a utility for generating random numbers.

## Machine Learning

The `MachineLearning` directory contains scripts and data for training machine learning models on IoT telemetry data. To run the Jupyter notebook for training:

1. Install Jupyter Notebook if not already installed:
    ```bash
    pip install notebook
    ```
2. Navigate to the MachineLearning directory:
    ```bash
    cd MachineLearning
    ```
3. Start the Jupyter Notebook:
    ```bash
    jupyter notebook training.ipynb
    ```

## Grafana

The Grafana dashboard provides real-time data visualization and monitoring. It displays sensor readings and system status, allowing operators to remotely monitor the conditions within the battery room. To set up Grafana:

1. Install Grafana following the instructions on the [official website](https://grafana.com/get).
2. Configure Grafana to read data from the MySQL database used by this project.
3. Import the provided dashboard configuration to visualize real-time data.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.
