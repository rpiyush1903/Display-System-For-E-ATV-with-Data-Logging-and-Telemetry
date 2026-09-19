# Smart Display and Safety System for an E-ATV Using CAN Bus Communication Protocol

**Team AVEON Racing**

## Description

This project is a custom dashboard and safety unit that I built for our electric All-Terrain Vehicle. It sits between the driver and the battery pack, doing two jobs at once: showing the driver what the battery is doing in real time, and acting as a safety switch that can cut power to the tractive system if something goes wrong.

The unit reads data directly from our JBD Battery Management System over CAN bus and shows it on a TFT display mounted on the dash — pack voltage, current draw (or charge current), state of charge, and pack temperature. It also drives a relay that can de-energize the main contactors, either automatically when a fault is detected or manually during testing.

Everything runs on an ESP32/Arduino-compatible microcontroller, with an MCP2515 module handling the CAN interface, and the whole thing is built onto a custom PCB so it can survive the vibration and abuse of off-road racing.

## Why I Built This

When you're racing an e-ATV, the battery pack is both the thing that makes the vehicle go and the thing most likely to cause a serious problem if it's mismanaged. Two concerns pushed me to build this system.

**Charging and pack safety.** A lithium pack that's overcharged, over-discharged, or running too hot can fail badly, and on a race vehicle there isn't always someone watching a laptop connected to the BMS. I wanted a hardware safety interlock that sits close to the pack and can isolate it the moment something crosses a safe limit, rather than relying on the driver to notice a problem and react in time.

**Driver awareness.** Beyond safety, the driver genuinely needs to know what the battery is doing while driving — voltage, current draw, temperature, and remaining charge. Without this, the driver is flying blind: they don't know if they're pushing the pack too hard, how much charge is left for the rest of a heat, or whether the pack is running hotter than it should. A simple, readable display on the dash solves that, and having it double as a diagnostic tool during pit stops and testing was a natural extension.

Putting both of these on one board also meant fewer separate systems, less wiring, and one less thing to go wrong on the vehicle.

## How I Did It

I started from the JBD BMS's CAN output, since that's already generating the data I needed — I didn't want to add extra sensors when the BMS was already measuring everything.

**Reading the BMS over CAN.** I used an MCP2515 CAN controller (SPI interface) to talk to the BMS at 500kbps. The BMS broadcasts (and can be polled) on a few standard IDs, and I parse the ones that matter for the dashboard:

| CAN ID | Data | Notes |
| :--- | :--- | :--- |
| `0x100` | Pack voltage and current | 0.01V/bit and 0.01A/bit; I check the sign/state to tell charging apart from discharging |
| `0x101` | State of charge | Percentage |
| `0x105` | Pack temperature | 0.1°C/bit, offset by 273.15K |

Since the BMS doesn't just keep streaming data forever on its own, I have the microcontroller send a small keep-alive frame (`0x5A`) periodically to the BMS's target IDs, which keeps the connection alive and the data flowing.

**Showing it to the driver.** The parsed values get pushed to a TFT display using the `TFT_eSPI` library, connected over the hardware SPI bus. I set up `User_Setup.h` to match the exact driver and pinout of the display I used, since getting this wrong is the most common reason the display doesn't come up at all.

**The safety side.** A relay is wired to a dedicated GPIO pin (pin 5 in the diagnostic build). HIGH keeps the relay de-energized and power flowing normally; pulling it LOW cuts power by opening the main contactors. This lets the firmware isolate the pack automatically if a reading goes outside safe limits, and it also gives me a manual way to cut power during bench testing without touching the high-voltage wiring directly.

**Putting it on one board.** Once the logic was working on a breadboard, I moved everything onto a custom PCB. The main reason was reliability — an ATV shakes a lot, and loose header connections are a common failure point. I also made sure to physically separate the high-speed SPI lines (display and CAN) from the relay switching lines on the layout, since the relay can introduce electrical noise that was causing flicker on the display and occasional glitches on the CAN bus during earlier tests.

### Pin Reference

| Component | MCU Pin | Function |
| :--- | :--- | :--- |
| MCP2515 CS | Pin 7 | SPI chip select for the CAN controller |
| Safety Relay | Pin 5 | Digital output (HIGH = de-energized / power on, LOW = energized / power cut) |
| TFT Display | SPI bus | Hardware SPI, configured via `User_Setup.h` |


### Schematics and PCB Layout

*(Circuit diagram and PCB images go here.)*
