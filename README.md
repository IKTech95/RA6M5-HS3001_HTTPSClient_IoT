# RA6M5 HTTPS Client with HS3001 Sensor
This project demonstrates how to use the **Renesas RA6M5** microcontroller as an **HTTPS client** that reads temperature data from the **HS3001 sensor** and sends it securely to the **Adafruit IO** platform via HTTPS REST API. 

## 🛠️ Hardware Requirements
- Renesas EK-RA6M5 board  [Evaluation Kit for RA6M5 MCU Group](https://www.renesas.com/en/design-resources/boards-kits/ek-ra6m5)
- HS3001 temperature & humidity sensor [US082-HS3001EVZ](https://www.renesas.com/en/design-resources/boards-kits/us082-hs3001evz)
- Micro USB cable
- LAN cable
- Ethernet Switch
- Jumper wires

---
## 🔌 Pin Connections (Sensor)
| RA6M5 Pin | HS3001 Pin | Description       |
|----------:|------------|-------------------|
| P511      | SDA        | I²C Data          |
| P512      | SCL        | I²C Clock         |
| GND       | GND        | Ground            |
| 3.3V      | VDD        | Power Supply      |
---
## Board Connections
- Connect EK-RA6M5 board to Host machine using micro usb cable.
- Connect LAN cable on EK-RA6M5 at ethernet port and other to the ethernet switch.

> Switch should have WAN connection to communicate the server over internet. The Switch should be connected to the router which is connected to the Internet.

## SEGGER RTT block address
- As soon as the project is compiled and downloaded, find the block address(for the variable in RAM called _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).


## Validating Operation
> Check also the **hs3001_ra6m5_https_client_notes.md** file for further information **in hs3001_ra6m5_https_client folder.**
