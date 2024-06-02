---
header-includes: |
    \usepackage{graphicx}
    \usepackage{seqsplit}
    \usepackage{fvextra}
    \DefineVerbatimEnvironment{Highlighting}{Verbatim}{breaklines,commandchars=\\\{\}}
    \usepackage{caption}
    \usepackage{subcaption}
    \usepackage{xcolor}
    \usepackage{lscape}

    \usepackage{tabularx}
    \usepackage{booktabs}
    \usepackage{caption}
    \usepackage{geometry}
    \usepackage{xltabular}

---


\title{Internet of Things}

\begin{figure}[!htb]
    \centering
    \includegraphics[keepaspectratio=true,scale=0.4]{Resources/"cherub.eps"}
\end{figure}

\begin{center}
    \LARGE{UNIVERSITY OF PISA}
    \vspace{5mm}
    \\ \large{MASTER'S DEGREE IN COMPUTER ENGINEERING}\vspace{3mm}
    \\ \large{Internet of Things}
    \vspace{10mm}
    \\ \huge\textbf{VoltVault}
\end{center}

\vspace{20mm}

\begin{minipage}[t]{0.47\textwidth}
	{\large{Professors:}{\normalsize\vspace{3mm} \bf\\ \large{Giuseppe Anastasi}\vspace{3mm}
 \\ \large{Francesca Righetti}\vspace{3mm}
 \\ \large{Carlo Vallati}}}
\end{minipage}
\hfill
\begin{minipage}[t]{0.47\textwidth}\raggedleft
 {\large{Group Members:}\raggedleft
 {\normalsize\vspace{3mm}
	\bf\\ \large{Giovanni Ligato}\raggedleft
     \normalsize\vspace{3mm}
    	\bf\\ \large{Giuseppe Soriano}\raggedleft}}
\end{minipage}

\vspace{20mm}
\begin{center}
\large{\textbf{GitHub Repository}: \url{https://github.com/giovanniligato/IoT-Project}}
\end{center}
\vspace{15mm}

\hrulefill

\begin{center}
\normalsize{ACADEMIC YEAR 2023/2024}
\end{center}

\pagenumbering{gobble}

\renewcommand*\contentsname{Index}
\tableofcontents


\newpage

\pagenumbering{arabic}


# 1. Introduction
In an industrial setting, rooms dedicated to the maintenance and inspection of industrial batteries or fuel cells are critical. These rooms, known as battery rooms or fuel cell rooms, pose significant risks due to the potential release of *hazardous gases*, particularly carbon monoxide (CO), and the *heat* generated during the charging and discharging cycles of the batteries or fuel cells. 

The industrial battery room is typically situated within larger facilities such as factories producing electric vehicles, renewable energy plants utilizing fuel cells, or data centers relying on large battery banks for uninterruptible power supplies (UPS). For instance, in a factory producing electric vehicles, the battery room is essential for the regular maintenance and inspection of the batteries that power the vehicles. Similarly, in a renewable energy plant, fuel cells are maintained in these rooms to ensure efficient energy storage and conversion. Data centers, which require constant power, also depend on battery rooms to house backup batteries that can be critical during power outages.

The primary function of these rooms is to house large industrial batteries or fuel cells essential for energy storage and backup power. Regular maintenance, inspection, and occasional repair of these energy storage systems take place in this controlled environment. During the charging and discharging processes, batteries can emit gases, including hydrogen and carbon monoxide. CO is particularly dangerous as it is colorless and odorless, posing a significant risk of poisoning to human operators. Furthermore, the chemical reactions within batteries and fuel cells generate substantial heat. Without adequate ventilation and cooling, the room can reach high temperatures, making it unsafe for human occupancy.

**VoltVault** is designed to mitigate these risks implementing stringent safety protocols. Before any human operator enters the battery room, sensors for evaluating CO levels, temperature, and humidity are activated. These sensors are strategically placed to provide accurate and comprehensive readings. If sensors detect CO levels above permissible exposure limits or temperatures exceeding the safe operational range, an automated system activates fans and cooling units. The ventilation system is designed to rapidly expel hazardous gases and reduce the temperature to safe levels, ensuring the environment is safe for human entry.

Once the operator enters the room, the sensors remain active to continuously monitor the conditions. This real-time monitoring ensures that any sudden changes in CO levels or temperature are immediately detected, triggering the ventilation and cooling systems as necessary. The operator's tasks include visual inspections of the batteries or fuel cells, checking for signs of wear or damage, performing diagnostic tests, replacing faulty components, and ensuring all systems are operating efficiently. Given the critical nature of these tasks, it is essential that the room maintains optimal conditions to protect the operator's health.



\newpage 

# 2. Use Case: Industrial Battery Room Monitoring

The VoltVault system (\autoref{fig:use_case}) is designed to ensure the safety and efficiency of an industrial battery room through continuous monitoring and automated responses. This system leverages a variety of sensors and control mechanisms to maintain optimal environmental conditions and protect human operators.

\begin{figure}[!htb]
    \centering
    \includegraphics[width=0.95\textwidth]{Resources/"use_case.png"}
    \caption{The VoltVault system diagram illustrates the integration of various sensors and control mechanisms to ensure the safety and efficiency of an industrial battery room. Key components include movement sensors, a vault status indicator, an automatic door, an HVAC system, CO and temperature/humidity sensors, and a real-time monitoring dashboard.}
    \label{fig:use_case}
\end{figure}

## 2.1. Overview of the System Components
The VoltVault system comprises the following key components:

### 2.1.1. Movement Sensor
The movement sensor is installed at the entrance of the battery room. It detects the presence of personnel approaching the room and initiates the preliminary safety checks before allowing entry.

### 2.1.2. Vault Status Indicator
The vault status indicator is a traffic light system that displays the current status of the battery room:

- **Red Light:** Indicates that the room is unsafe for entry due to high levels of CO or excessive temperature or humidity.
- **Yellow Light:** Indicates that the system is assessing the room conditions before entry is allowed.
- **Green Light:** Indicates that the room is safe for entry. Blinking green indicates that the automatic door is open.

### 2.1.3. Automatic Door
The automatic door is controlled by the VoltVault system. It remains locked when the room conditions are unsafe and automatically unlocks when it is safe for human operators to enter. There is also a manual opening mechanism for triggering manually the movement sensor and for allowing the user to leave the room.

### 2.1.4. HVAC System
The HVAC (Heating, Ventilation, and Air Conditioning) system is integrated to manage the temperature and ventilation within the battery room. It activates automatically in response to readings from the CO sensor and temperature and humidity sensor to maintain safe conditions.

### 2.1.5. Sensors

- **CO Sensor:** Monitors the levels of carbon monoxide within the room. If the CO levels exceed the permissible threshold, the system triggers the HVAC system to increase ventilation.
- **Temperature and Humidity Sensor:** Tracks the temperature and humidity levels inside the battery room. High temperatures trigger the cooling function of the HVAC system, while abnormal humidity levels prompt adjustments to maintain optimal conditions.

Both sensors are by default in *sleeping mode* and activated only when the movement sensor detects the presence of a human operator.

### 2.1.6. Dashboard (Grafana)
The dashboard provides real-time data visualization and monitoring. It displays sensor readings and system status, allowing operators to remotely monitor the conditions within the battery room. It also logs historical data for analysis and reporting purposes.

## 2.2. Operational Workflow

1. **Pre-Entry Check:** When a person approaches the battery room, the movement sensor detects their presence and activates the pre-entry check.
   - The CO sensor and temperature and humidity sensor immediately assess the room conditions.
   - The vault status indicator updates to reflect the current safety status.

2. **Assessment and Response:**
   - If the room conditions are critical, the vault status indicator shows a red light, the HVAC system activates to ventilate and cool the room, and the automatic door remains locked.
   
3. **Safe Entry:**
   - Once the sensors confirm that room conditions are within the safe range, the vault status indicator turns green.
   - The automatic door unlocks (green led blinks), allowing safe entry for maintenance personnel.

4. **Continuous Monitoring:**
   - While the operator is inside the room, the sensors continue to monitor the conditions in real-time.
   - Any sudden changes trigger immediate adjustments by the HVAC system to ensure the environment remains safe.

5. **Maintenance and Inspection:**
   - The operator performs tasks such as visual inspections, diagnostic tests, and component replacements.
   - Throughout this period, the dashboard provides real-time updates and alerts, ensuring the operator is aware of the room conditions.

6. **Post-Exit Monitoring:**
   - After the operator exits, the sensors turn to sleep mode, and the system remains in standby until the next pre-entry check is initiated.

\newpage 

# 3. System Architecture

The VoltVault system employs a sophisticated wireless sensor network (WSN) to ensure the safe operation and monitoring of an industrial battery maintenance room. The system leverages CoAP (Constrained Application Protocol) for efficient communication, utilizing observe relationships to minimize latency and enhance responsiveness. Below is a detailed breakdown of the system components and their interactions (\autoref{fig:architecture}).

\begin{figure}[!htb]
    \centering
    \includegraphics[width=0.95\textwidth]{Resources/"architecture.png"}
    \caption{The VoltVault system architecture diagram illustrates the integration of various sensors and control mechanisms within a wireless sensor network (WSN). Key components include movement sensor, CO sensor, temperature and humidity sensor, an HVAC system with a machine learning model, a border router for internet connectivity, a user application for remote control, a cloud application for data storage and node registration, and Grafana for data visualization.}
    \label{fig:architecture}
\end{figure}

## 3.1. System Components and Interactions

### 3.1.1. Movement Sensor
- **Resource Exposed:** `/movement`
- **Function:** Detects the presence of personnel approaching the battery room.
- **Observations:** Does not observe any other resource.
- **Description:** This sensor activates when it detects motion near the entrance of the room. It plays a crucial role in initiating the safety checks before entry.

### 3.1.2. Vault Status
- **Resource Exposed:** `/vaultstatus`
- **Observations:** Observes `/movement` and `/hvac`.
- **Function:** Indicates the safety status of the battery room.
- **Description:** The vault status system updates based on inputs from the movement sensor and the HVAC system. It displays a traffic light indicator (red, yellow, green) to communicate the room’s status to operators.

### 3.1.3. CO Sensor
- **Resource Exposed:** `/co`
- **Observations:** Observes `/vaultstatus`.
- **Function:** Monitors carbon monoxide levels in the room.
- **Description:** This sensor ensures that CO levels remain within safe limits by tracking CO concentrations and alerting the system if levels exceed predefined thresholds.

### 3.1.4. Temperature and Humidity Sensor
- **Resource Exposed:** `/temperatureandhumidity`
- **Observations:** Observes `/vaultstatus`.
- **Function:** Measures temperature and humidity levels within the room.
- **Description:** This sensor provides essential environmental data to maintain safe and optimal conditions for battery maintenance operations.

### 3.1.5. HVAC System
- **Resource Exposed:** `/hvac`
- **Observations:** Observes `/co` and `/temperatureandhumidity`.
- **Function:** Regulates the environmental conditions in the room.
- **Description:** The HVAC system employs a *Machine Learning* model (Section **5. Machine Learning Model**) to evaluate data from CO, temperature, and humidity sensors. Based on this data, it classifies the room as *habitable* or *not* and adjusts ventilation and cooling systems accordingly.

### 3.1.6. Border Router
- **Function:** Provides connectivity between the WSN and the internet.
- **Description:** The border router facilitates remote access and control of the system, enabling seamless communication between local sensors and cloud-based applications.

### 3.1.7. User Application
- **Function:** Allows remote interaction with the system.
- **Description:** This application enables users to remotely trigger the movement sensor, simulating an operator’s entry to assess room conditions from a distance.

### 3.1.8. Cloud Application
- **Components:** CoAP Server for Registration, User Application
- **Function:** Manages node registration and stores sensor data.
- **Description:** The cloud application handles the initial registration of sensors and actuators as they join the network. It collects and stores data from temperature, humidity, and CO sensors, as well as the HVAC system status, in a MySQL database.

### 3.1.9. Grafana
- **Function:** Visualizes system data.
- **Description:** Grafana reads data from the MySQL database and displays it in real-time dashboards. This visualization tool allows operators to monitor current conditions and review historical data through various graphs and charts.

# 4. Data Encoding

In the VoltVault system, we use JSON (JavaScript Object Notation) for data encoding, adhering specifically to the SenML (Sensor Measurement Lists) standard. The choice of JSON is driven by its simplicity, human readability, and widespread adoption, making it an ideal format for data interchange in IoT environments. By conforming to the SenML standard, our solution ensures interoperability and can be seamlessly integrated with third-party applications, promoting a global and interoperable vision.

## 4.1. Why JSON and SenML?

### 4.1.1. Simplicity and Human Readability
JSON is a lightweight data interchange format that is easy to read and write for humans, and easy to parse and generate for machines. This makes it an excellent choice for encoding sensor data, which often needs to be reviewed and debugged by developers and operators.

### 4.1.2. Interoperability
Adhering to the SenML standard allows our system to produce data that is compatible with a wide range of third-party applications. SenML provides a standardized way to represent sensor measurements, metadata, and other relevant information, facilitating seamless data exchange and integration across different platforms and systems.

### 4.1.3. Global Vision
By using a standard like SenML, we ensure that the sensors programmed within our system can be utilized in a broader context. This aligns with the IoT paradigm, where devices from different manufacturers and ecosystems need to work together harmoniously.

## 4.2. SenML Structure

SenML represents data using a series of records, each containing one or more measurements or metadata entries. Below is an example of a SenML JSON record used in the VoltVault system:

```json
    {
        "e":[
            {"n":"temperature","v":2700000,"u":"Cel"},
            {"n":"humidity","v":7356000,"u":"%RH"}
        ],
        "bn":"urn:dev:mac:916B08BC215E:",
        "bt":0,
        "ver":1
    }
```

### 4.2.1. Breakdown of SenML Fields:
- **Base Name (bn):** Provides a common prefix for the names of all measurements in the record. This can be used to identify the device or source.
- **Base Time (bt):** Represents the base time for all measurements. In our system, we set this value to 0 to indicate that the measurement corresponds to the current instant. This is due to the lightweight nature of our sensors, which do not have the concept of absolute time. On the server side, this will be stored as the current time, as high precision is not required.
- **Entries (e):** An array of entries where each entry represents a measurement.
  - **Name (n):** The name of the measurement (e.g., temperature, humidity, CO).
  - **Unit (u):** The unit of measurement (e.g., Celsius for temperature, %RH for humidity, ppm for CO).
  - **Value (v):** The value of the measurement.

## 4.3. Handling Floating Point Values

Due to the limitations of the Nordic nRF52840 microcontrollers, we encode floating-point values as integers by multiplying them by 100,000. This approach maintains a precision of five decimal places, ensuring that the sensor data remains accurate and reliable.

### 4.3.1. Example:
If the temperature sensor reads 27°C, it will be reported as 2700000 in the JSON payload. When interpreting these values, users and applications must divide by 100,000 to retrieve the original measurement.

```json
    {
        "e":[
            {"n":"temperature","v":2700000,"u":"Cel"},
            {"n":"humidity","v":7356000,"u":"%RH"}
        ],
        "bn":"urn:dev:mac:916B08BC215E:",
        "bt":0,
        "ver":1
    }
```

**Note:** This encoding technique ensures that the system remains compliant with the limitations of the nRF52840 microcontrollers while providing accurate sensor data.

\newpage 

# 5. Machine Learning Model

The VoltVault system employs a machine learning model to assess the environmental conditions of the industrial battery room. This model is implemented within the HVAC actuator, which plays a crucial role in maintaining the room's safety and optimal conditions. The model used is a **RandomForestClassifier**, known for its robustness and accuracy.

## 5.1. Integration in HVAC Actuator

The HVAC actuator utilizes the machine learning model to process real-time sensor data, such as CO levels, temperature, and humidity. Based on these inputs, the model classifies the room's condition as habitable or non-habitable, enabling the HVAC system to take appropriate actions, like adjusting ventilation or cooling.

### 5.1.1. Dataset

The dataset used for training the model is sourced from [Kaggle - Environmental Sensor Data](https://www.kaggle.com/datasets/garystafford/environmental-sensor-data-132k). This dataset contains 132,000 records of environmental sensor data, including temperature, humidity, and CO levels. It provides a comprehensive set of features for training robust machine learning models aimed at environmental monitoring.

### 5.1.2. Training the Model

The training of the RandomForestClassifier involves several key steps:

```python
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier

# Load and prepare data
data = pd.read_csv('sensor_data.csv')
X = data.drop(columns=['habitable'])
y = data['habitable']

# Split the dataset into training and testing sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Train the RandomForest model
model = RandomForestClassifier(n_estimators=100, random_state=42)
model.fit(X_train, y_train)
```

1. **Data Preparation:** The dataset is loaded, and irrelevant columns are removed. The features (`X`) and labels (`y`) are defined.
2. **Dataset Splitting:** The data is divided into training and testing sets to ensure the model's performance can be evaluated accurately.
3. **Model Training:** The `RandomForestClassifier` is trained on the training set, learning the relationships between the sensor readings and the room's habitability.

### 5.1.3. Deployment

After training, the model is converted into C code using emlearn, allowing it to run efficiently on the HVAC actuator's hardware:

```python
import emlearn

# Convert the trained model to C code
path = 'machine_learning.h'
cmodel = emlearn.convert(model, method='inline')
cmodel.save(file=path, name='machine_learning')

print('Wrote model to', path)
```

Using **emlearn**, the trained model is converted into efficient C code, allowing it to run effectively on the actuator's hardware. This ensures that the HVAC system can respond quickly to changing environmental conditions, maintaining a safe and optimal environment in the battery room.

By embedding the machine learning model within the HVAC actuator, the VoltVault system provides dynamic and real-time control, significantly enhancing the safety and operational efficiency of the industrial battery room.

# 6. MySQL Database Schema

The VoltVault system uses a MySQL database to store sensor data and actuator statuses. This database is critical for maintaining historical records and enabling real-time monitoring and analysis via Grafana. Below are the descriptions of the key tables in the database.

## 6.1. Table: `co_sensor`

The `co_sensor` table stores data from the CO sensor, which monitors the concentration of carbon monoxide in the battery room.

\begin{table}[h]
    \begin{tabularx}{\textwidth}{XXXX}
        \toprule
        Field       & Type      & Default           & Extra             \\
        \midrule
        \textbf{id} (PK) & int       & NULL              & auto\_increment    \\
        co          & double    & NULL              &                   \\
        timestamp   & timestamp & Current\_Timestamp & Default\_Generated \\
        \bottomrule
    \end{tabularx}
\end{table}


### 6.1.1. Description:
- **id:** Unique identifier for each record.
- **co:** The measured CO concentration in parts per million (ppm).
- **timestamp:** The time at which the measurement was taken. This field uses the current timestamp by default, as the lightweight sensors do not have an absolute time concept.

## 6.2. Table: `hvac_actuator`

The `hvac_actuator` table records the status of the HVAC system, which is responsible for maintaining the room’s environmental conditions.

\begin{table}[h]
    \begin{tabularx}{\textwidth}{XXXX}
        \toprule
        Field       & Type       & Default           & Extra             \\
        \midrule
        \textbf{id} (PK) & int        & NULL              & auto\_increment    \\
        status      & tinyint(1) & NULL              &                   \\
        timestamp   & timestamp  & Current\_Timestamp & Default\_Generated \\
        \bottomrule
    \end{tabularx}
\end{table}

### 6.2.1. Description:
- **id:** Unique identifier for each record.
- **status:** The operational status of the HVAC system (e.g., on/off).
- **timestamp:** The time at which the status was recorded, defaulting to the current timestamp.

## 6.3. Table: `temphum_sensor`

The `temphum_sensor` table stores data from the temperature and humidity sensors.

\begin{table}[h]
    \begin{tabularx}{\textwidth}{XXXX}
        \toprule
        Field       & Type      & Default            & Extra             \\
        \midrule
        \textbf{id} (PK) & int       & NULL          & auto\_increment    \\
        temperature & double    & NULL               &                   \\
        humidity    & double    & NULL               &                   \\
        timestamp   & timestamp & Current\_Timestamp & Default\_Generated \\
        \bottomrule
    \end{tabularx}
\end{table} 

### 6.3.1. Description:
- **id:** Unique identifier for each record.
- **temperature:** The measured temperature in degrees Celsius. 
- **humidity:** The measured humidity as a percentage.
- **timestamp:** The time at which the measurement was taken, with the default being the current timestamp.

## 6.4. Table: `iot_nodes`

The `iot_nodes` table keeps track of the IP addresses and resources exposed by the various IoT nodes within the system.

\begin{table}[h]
    \begin{tabularx}{\textwidth}{XXXX}
        \toprule
        Field & Type & Default & Extra \\
        \midrule
        \textbf{ip} (PK) & varchar(50)  & NULL & \\
        resource\_exposed & varchar(255) & NULL & \\
        \bottomrule
    \end{tabularx}
\end{table}

### 6.4.1. Description:
- **ip:** The IP address of the IoT node.
- **resource_exposed:** The resource or endpoint exposed by the IoT node, such as `co`, `temperatureandhumidity`, or `hvac`.


\newpage 

# 7. Grafana Dashboard

To effectively monitor and analyze the environmental conditions within the industrial battery room, we utilize Grafana for data visualization. Below are some screenshots demonstrating the real-time dashboards and historical data analysis capabilities provided by Grafana. These visualizations enable operators to maintain optimal conditions and promptly respond to any anomalies.

\begin{figure}[htb!]
  \centering
  \ContinuedFloat
  \begin{subfigure}{0.68\linewidth}
    \centering
    \includegraphics[width=\linewidth]{Resources/grafana_screen_1.png}
    \caption{HVAC status history, indicating the operational state of the HVAC system and its response to environmental changes.}
    \label{fig:grafana_screen_1}
  \end{subfigure}
\end{figure}

\begin{figure}[htb!]
  \centering
  \ContinuedFloat
  \begin{subfigure}{0.68\linewidth}
    \centering
    \includegraphics[width=\linewidth]{Resources/grafana_screen_2.png}
    \caption{Temperature readings over time, showing a detailed trend of temperature changes within the room.}
    \label{fig:grafana_screen_2}
  \end{subfigure}
\end{figure}

\newpage

\begin{figure}[htb!]
  \centering
  \ContinuedFloat
  \begin{subfigure}{0.68\linewidth}
    \centering
    \includegraphics[width=\linewidth]{Resources/grafana_screen_3.png}
    \caption{Humidity levels over time, illustrating the fluctuations in relative humidity.}
    \label{fig:grafana_screen_3}
  \end{subfigure}
\end{figure}

\begin{figure}[htb!]
  \centering
  \ContinuedFloat
  \begin{subfigure}{0.68\linewidth}
    \centering
    \includegraphics[width=\linewidth]{Resources/grafana_screen_4.png}
    \caption{CO levels over time, providing insight into the concentration of carbon monoxide.}
    \label{fig:grafana_screen_4}
  \end{subfigure}
  \caption{Grafana dashboard}
\end{figure}


\newpage

# 8. Conclusion and Future Work

The VoltVault system successfully integrates a machine learning model within the HVAC actuator to monitor and manage the environmental conditions of industrial battery rooms. Utilizing a **RandomForestClassifier** and **emlearn** for model conversion, the system ensures real-time assessment and control, enhancing safety and operational efficiency.

While effective, VoltVault can be further improved and expanded. Future versions could incorporate additional sensors to monitor other hazardous gases and battery health metrics, providing a more comprehensive overview. By incorporating predictive analytics, the system could anticipate potential issues, allowing for proactive maintenance and reducing downtime.

Exploring advanced machine learning models could improve the detection of complex patterns and anomalies, enhancing accuracy and reliability. Additionally, developing a more intuitive interface for Grafana dashboards and user applications would improve usability and decision-making.

Finally, designing VoltVault to be more scalable and adaptable would allow for application in various industrial settings, from small storage rooms to large-scale facilities. In summary, the VoltVault system offers a robust solution for monitoring and managing the environmental conditions of industrial battery rooms, with significant potential for future enhancements to further advance industrial safety and efficiency.