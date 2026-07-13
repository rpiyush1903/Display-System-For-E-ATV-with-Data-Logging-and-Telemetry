# Display-System-For-E-ATV-with-Data-Logging-and-Telemetry
# E-ATV Smart Display and Safety Charging System

## Overview
This repository contains the firmware and hardware design specifications for a custom Smart Display and Safety Relay unit, developed specifically for an electric All-Terrain Vehicle (e-ATV). The system acts as the primary dashboard for the driver while simultaneously functioning as a critical safety node. It interfaces directly with a JBD Battery Management System (BMS) via CAN bus to parse real-time battery data and controls a solid-state relay to isolate the tractive system in case of emergency or detected faults.

This system was designed and implemented for competitive e-ATV racing operations by Team AVEON Racing.

## Core Features
*   **Real-Time CAN Bus Integration:** Utilizes an MCP2515 CAN controller to sniff and request data from a JBD BMS at a 500kbps baud rate.
*   **Driver Dashboard:** Displays critical powertrain metrics including Pack Voltage, Draw/Charge Current, State of Charge (SOC), and Temperature using a TFT display via the `TFT_eSPI` library.
*   **Hardware Safety Interlock:** Integrates a safety relay on a dedicated GPIO pin, capable of de-energizing the main contactors to cut power when unsafe limits are detected or during diagnostic testing.
*   **Custom PCB Integration:** The hardware is consolidated onto a custom-designed printed circuit board to ensure vibration resistance and reliability in off-road racing environments.

## Hardware Architecture
### Components Used
*   **Microcontroller:** ESP32 / Arduino-compatible MCU
*   **CAN Controller:** MCP2515 (SPI Interface)
*   **Display:** TFT LCD Display
*   **BMS:** JBD Battery Management System (CAN enabled)
*   **Actuation:** 5V/12V Relay Module

### Pin Configuration (Diagnostic Build)
| Component | MCU Pin | Function |
| :--- | :--- | :--- |
| MCP2515 CS | Pin 7 | SPI Chip Select for CAN |
| Safety Relay | Pin 5 | Digital Output (HIGH = De-energized/Power ON, LOW = Energized/Power CUT) |
| TFT Display | SPI Bus | Hardware SPI pins configured via `User_Setup.h` in TFT_eSPI |

## Software Implementation
The firmware is written in C++ for the Arduino framework. It manages asynchronous tasks including SPI communication for the display and CAN polling. 

### CAN ID Mapping
The system parses the following standard CAN identifiers from the JBD BMS:
*   `0x100`: Pack Voltage (0.01V/bit) and Current (0.01A/bit). Includes logic to detect charge vs. discharge states.
*   `0x101`: State of Charge (SOC %).
*   `0x105`: Pack Temperature (0.1°C/bit, offset by 273.15K).

To maintain the CAN connection and request continuous telemetry, the MCU periodically transmits a keep-alive frame (`0x5A`) to the target BMS IDs.

## Schematics and PCB Layout
*(The custom PCB was designed to isolate the high-frequency SPI lines from the relay switching noise, ensuring stable display rendering and CAN transmission during active driving.)*

**Circuit Diagram**
![Circuit Diagram](insert_link_to_your_circuit_diagram_image_here)

**PCB Layout**
![PCB Image](insert_link_to_your_pcb_image_here)

## Installation & Setup
1.  Clone this repository.
2.  Install the required dependencies via Library Manager:
    *   `TFT_eSPI` (Configure `User_Setup.h` to match your exact TFT driver and pinout).
    *   `mcp_can` by Cory J. Fowler.
3.  Ensure the MCP2515 oscillator frequency in the code (`MCP_8MHZ` or `MCP_16MHZ`) matches your physical hardware crystal.
4.  Compile and upload to the microcontroller.
5.  Monitor the serial output at `9600` baud for hardware diagnostic validation before integrating into the ATV harness.

## Author
**Piyush Ranjan**  
General Secretary, Team AVEON Racing  
B.Tech Electrical and Electronics Engineering, BIT Mesra
