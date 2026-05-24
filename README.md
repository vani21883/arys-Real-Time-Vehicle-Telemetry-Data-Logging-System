# Real-Time Vehicle Telemetry & Data Logging System

## Overview

This project implements a real-time embedded telemetry and vehicle performance monitoring system using the ESP32 microcontroller and FreeRTOS under the ESP-IDF v6.0.1 framework.

The system collects data from multiple vehicle-oriented sensors and communication interfaces, performs sensor fusion, detects faults, logs telemetry data, and supports future wireless telemetry streaming.

This project was developed as part of the ARYS Garage Embedded Systems Assignment.

---

# Features

## RTOS-Based Architecture (FreeRTOS)

The firmware is built using multiple concurrent FreeRTOS tasks for modular and real-time operation.

Implemented tasks include:

- IMU sensor acquisition
- GPS parsing
- RPM measurement
- CAN communication handling
- SD card data logging
- Sensor fusion processing
- Fault monitoring
- Telemetry transmission framework

---

# Implemented Interfaces

| Interface | Protocol | Status |
|---|---|---|
| MPU6050 IMU | I2C | Implemented |
| NEO-6M GPS | UART | Implemented |
| Hall RPM Sensor | PCNT/GPIO | Implemented |
| CAN Bus | TWAI CAN | Implemented |
| SD Card Logger | SPI | Implemented |
| Wireless Telemetry | WiFi | Framework Added |

---

# System Capabilities

## Real-Time Telemetry Acquisition

The system continuously acquires:

- Accelerometer data
- Gyroscope data
- GPS coordinates
- Vehicle speed
- RPM values
- Heading/orientation

---

## Sensor Fusion

A complementary filter-based sensor fusion module combines accelerometer and gyroscope data to estimate:

- Roll
- Pitch
- Yaw
- G-force
- Distance travelled

---

## Fault Monitoring

The firmware includes real-time fault detection for:

- IMU timeout
- GPS signal loss
- CAN timeout
- RPM dropout
- SD card simulation failure handling

---

## Data Logging

Telemetry data is timestamped and logged in CSV format to an SD card interface.

Logged parameters include:

- Timestamp
- RPM
- Accelerometer values
- Gyroscope values
- GPS coordinates
- Vehicle speed

---

# Project Structure

```text
Telemetry_Project/
│
├── components/
│   ├── can/
│   ├── fault_monitor/
│   ├── gps/
│   ├── imu/
│   ├── rpm/
│   ├── sd_card/
│   ├── sensor_fusion/
│   └── telemetry/
│
├── main/
│   └── main.c
│
├── CMakeLists.txt
└── sdkconfig
