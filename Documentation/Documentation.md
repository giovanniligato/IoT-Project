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
In the context of an industrial environment, consider a room used for the maintenance and inspection of industrial batteries or fuel cells. These types of rooms, often referred to as battery rooms or fuel cell rooms, are critical due to the potential release of hazardous gases, particularly carbon monoxide (CO), and the heat generated by the batteries or fuel cells during charging and discharging cycles. 

## 1.1. Real Environment: Industrial Battery Room

**Location:** The industrial battery room is located within a larger manufacturing or power generation facility, such as a factory producing electric vehicles, a renewable energy plant using fuel cells, or a data center relying on large battery banks for uninterruptible power supply (UPS).

**Purpose:** The primary function of the room is to house large industrial batteries or fuel cells, which are essential for energy storage and backup power. The room is used for regular maintenance, inspection, and occasional repair of these energy storage systems.

**Potential Hazards:**

1. **High Carbon Monoxide Levels:** During the charging and discharging processes, batteries can emit gases, including hydrogen and carbon monoxide. CO is particularly dangerous as it is colorless and odorless, posing a risk of poisoning to human operators.
2. **High Temperatures:** The chemical reactions within batteries and fuel cells generate significant heat. Without adequate ventilation and cooling, the room can reach high temperatures, making it unsafe for human occupancy.

**Safety Protocols:**

- **Pre-Entry Sensor Activation:** Before any human operator enters the battery room, sensors for evaluating CO levels, temperature, and humidity are activated. These sensors are strategically placed at various points within the room to provide accurate and comprehensive readings.
- **Assessment and Response:** 
  - If the sensors detect CO levels above permissible exposure limits (typically below 35 ppm for continuous exposure), or temperatures exceeding the safe operational range (which can vary but often is around 25-30°C for battery rooms), an automated system activates fans and cooling units.
  - The ventilation system is designed to rapidly expel hazardous gases and reduce the temperature to acceptable levels, ensuring the environment is safe for human entry.
  
**Continuous Monitoring:** Once the operator enters the room, the sensors remain active to continually monitor the conditions. This real-time monitoring ensures that any sudden changes in CO levels or temperature are immediately detected, triggering the ventilation and cooling systems if necessary.

**Maintenance and Inspection Tasks:**

- The operator's tasks might include visual inspections of the batteries or fuel cells, checking for signs of wear or damage, performing diagnostic tests, replacing faulty components, and ensuring all systems are operating efficiently.
- Due to the critical nature of these tasks, it is essential that the room maintains optimal conditions to protect the health of the operator.