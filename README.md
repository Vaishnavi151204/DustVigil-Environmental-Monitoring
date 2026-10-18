DustVigil — Autonomous Air Pollution Control Pod

An embedded systems project for monitoring local air-quality indicators and controlling dust-mitigation and air-purification components for construction-site environments.

Project Overview

Construction sites generate airborne dust and other pollutants that can affect workers, nearby residents, and the surrounding environment. Conventional dust-control methods may require continuous manual operation and may not respond effectively to changing site conditions.

DustVigil is a prototype of an autonomous air-pollution control pod that combines sensor-based monitoring with automated control of fans, a spray mechanism, and water-level management. Its goal is to support responsive dust mitigation through embedded controllers and a local web dashboard.

The broader research concept explores multi-stage filtration, mist-assisted dust suppression, and microalgae-based carbon dioxide sequestration. The repository currently focuses on the supplied ESP8266 controller sketches and their monitoring dashboards.

Project Objectives

Monitor sensor readings using embedded hardware.

Automate fan and spray control using configurable thresholds.

Monitor water level and control spray and refill mechanisms.

Provide a local web dashboard for viewing readings and controlling outputs.

Explore sustainable, modular air-pollution mitigation for construction sites.

Prototype Gallery

1. Hardware Prototype



Front view of the DustVigil prototype.

2. Circuit and Hardware Connections



Circuit and component connections used in the prototype.

3. Controller Dashboard — Parts 1 and 2





Dashboard views for the first and second controller sections.

4. Controller Dashboard — Part 3





Additional dashboard views from the prototype.

Key Features

Sensor monitoring: Reads an analog value from an MQ-series sensor and obtains raw readings from an HX710B pressure-sensor module.

Automatic fan control: Uses threshold-based logic to control the primary fan.

Pressure-based control: Uses a configured raw pressure reading threshold to control the secondary fan.

Water-level monitoring: Reads an analog water-level sensor.

Spray and refill control: Supports automatic threshold-based operation and dashboard controls.

Local web dashboard: Displays readings and provides Auto/Manual modes and output controls.

Embedded control: Uses ESP8266-based controller sketches with Wi-Fi connectivity.

System Architecture

The prototype is organised into two controller sketches.

1. Air Purification and Filter-Monitoring Controller

The first ESP8266 sketch handles:

MQ-series analog sensor readings.

Raw HX710B module readings.

Primary fan control based on the configured sensor threshold.

Secondary fan control based on the configured pressure threshold.

Spray output control.

Manual controls and monitoring through a local web dashboard.

2. Water-Level and Spray Controller

The second ESP8266 sketch handles:

Analog water-level sensor readings.

High- and low-level threshold checks.

Spray and refill relay control.

Auto/Manual mode selection.

Water-level monitoring through a local web dashboard.

The temperature value displayed by this sketch is simulated in software, not measured by a physical temperature sensor in the supplied code.

Hardware Components

The components represented in the supplied sketches include:

ESP8266 development boards

MQ-series analog gas sensor module

HX710B pressure-sensor interface module

Analog water-level sensor

Relay modules

Fan units

Spray pump or misting mechanism

Water refill pump or mechanism

Connecting wires and supporting hardware

Additional filtration and microalgae components belong to the broader DustVigil research concept and should be documented separately according to the hardware actually assembled and tested.

Software and Technologies

Programming language: Arduino C/C++

Embedded platform: ESP8266

Development environment: Arduino IDE

Connectivity: Wi-Fi

Web server: ESPAsyncWebServer

Networking library: ESPAsyncTCP

Dashboard: HTML, CSS, and JavaScript served by the controller

Control method: Threshold-based logic

Version control: Git and GitHub

Repository Structure

Dustvigil/
├── Arduino_Code/
│   ├── Air_Purification_Controller/
│   │   └── Air_Purification_Controller.ino
│   └── Water_Level_Controller/
│       └── Water_Level_Controller.ino
├── images/
│   ├── circuits.jpeg
│   ├── front_view.jpeg
│   ├── part1&part2_dashboard.jpeg
│   ├── part1&part2_dashboard2.jpeg
│   ├── part3_dashboard.jpeg
│   └── part3_dashboard2.jpeg
└── README.md

The folder and sketch names above are examples. Adjust them to match your actual filenames and folders.

How It Works

The controller reads sensor values at regular intervals.

In automatic mode, the controller compares readings with configured thresholds.

Based on the logic in the first sketch, the primary fan and spray output are controlled using the MQ sensor reading, while the secondary fan is controlled using the HX710B raw reading.

The water-level controller compares the analog water-level reading with configured low and high thresholds to operate the spray and refill outputs.

The local dashboard displays available readings and provides controls for changing modes and operating outputs.

Implementation Status

Implemented in the supplied sketches

ESP8266-based sensor-reading logic.

Threshold-based fan and water-level control.

Relay output control for fans, spray, and refill mechanisms.

Local web dashboards with monitoring and manual-control interfaces.

Wi-Fi connectivity for dashboard access.

Limitations and validation needed

The supplied sketches do not directly measure PM2.5 or PM10 concentrations using dedicated particulate-matter sensors.

The MQ sensor's analog reading should not be presented as a calibrated PM2.5 or PM10 measurement.

The HX710B reading is handled as a raw value; calibrated pressure units require additional calibration.

The temperature value in the water-level sketch is simulated.

Filtration efficiency, pollutant reduction, water savings, and CO₂ sequestration require documented experiments and appropriate measurement equipment.

Microalgae-based treatment, fuzzy-logic control, and mobile-app integration should be described as implemented only when their corresponding hardware and software have been verified.

Future Scope

Integrate calibrated PM2.5 and PM10 sensors.

Improve sensor calibration and logging of environmental measurements.

Add historical data storage and dashboard visualisation.

Integrate and evaluate the proposed microalgae-based CO₂ treatment subsystem.

Investigate energy-efficient operation and renewable power options.

Conduct controlled experiments to evaluate purification performance.

Explore mobile monitoring and remote notifications.

Research Context

DustVigil is associated with the research concept:

“Autonomous Air Pollution Control Pod for Construction Sites using Multi-Stage Filtration and Algae-Based Sequestration.”

The broader concept explores combining mechanical filtration, mist-assisted dust suppression, environmental monitoring, and microalgae-based treatment in a modular pollution-control system.

Author

Vaishnavi Dhankutkar

GitHub: Vaishnavi151204

Disclaimer

DustVigil is a prototype and research project. Its actual pollution-control performance depends on hardware configuration, sensor calibration, airflow, environmental conditions, and experimental validation. The current repository should not be interpreted as proof of certified air-purification performance.

