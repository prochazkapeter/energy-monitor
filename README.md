# energy-monitor

Energy consumption monitoring device using NodeMCU-32S and PZEM-004T v 3.0 module over MQTT.
This device could be easily integrated to Home Assistant using Mosquitto broker add-on.
You could visualize the data using standard hassio tools or implement ingestion to InfluxDB and
use Grafana.

This repo also contains 3D model of custom enclosure, which was printed using Prusa Mini+.
The ESP32 dev kit is mounted on custom PCB with DC power supply from 230 V. The PCB schematics 
are located in em_schematic directory.

## Quick Start

- download firmware and power up the device
- device acts as an AP, connect to it via mobile phone
- fill in Wi-Fi credentials and MQTT broker details

### MQTT Topics
- device sends measurements in JSON format to **homeenergy/sensor1** (can be changed in config.h)
- device listens to commands on **esp32/sensor1**
- data format -> {"voltage":"231.1","current":"0.540","power":"92.100",
  "energy":"18.53","projection":"180640","frequency":"50.0","pfactor":"0.74"}
 
### MQTT Commands
- **on**/**off** -> control the blue LED on ESP32 dev kit
- **resetEnergy** -> reset energy counter
- **resetWiFi** -> reset WiFi credentials
- **restartESP** -> restart the device
- **datefrom-dd/mm/yy** -> set start of the billing period
- **billdate-dd/mm/yy** -> set end of the billing period

