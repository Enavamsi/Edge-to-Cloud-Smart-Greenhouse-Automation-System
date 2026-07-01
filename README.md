# Edge-to-Cloud-Smart-Greenhouse-Automation-System
# 🌿 Smart Greenhouse IoT Gateway

An edge-to-cloud precision agriculture system that monitors soil nutrients, environmental conditions, and automates greenhouse hardware. 

## 📖 Description

This project implements a robust three-tier IoT architecture designed to maintain optimal plant health. An **ESP32 edge node** interfaces with industrial RS485 soil sensors and I2C environmental sensors to make real-time, local actuation decisions (controlling water pumps, cooling fans, and grow lights). 

To ensure zero data loss and enable remote monitoring, a **Raspberry Pi Gateway** acts as a local MQTT broker. It runs dual Python background services to simultaneously archive telemetry to a local CSV file and synchronize the data with a remote MySQL database via a Hostinger web API.

---

## ✨ Key Features

* **Edge Automation:** Non-blocking hardware control (Pump, Fan, Lights) based on hardcoded environmental thresholds.
* **Multi-Protocol Sensing:** Integrates Modbus RTU (RS485) for NPK/pH soil readings and I2C for air temperature/humidity/lux.
* **Dual-Layer Data Logging:** * `soil_logger.py`: Local CSV archiving for offline persistence.
  * `site_entry.py`: Cloud API synchronization for remote dashboard viewing.
* **Fault Tolerant:** Automated MQTT reconnection logic ensures 24/7 system uptime even after network drops.

---

## 🏗️ System Architecture

1. **Sensing Layer (ESP32):** Reads physical environment data every 10 seconds.
2. **Action Layer (ESP32):** Triggers 12V/220V relays if thresholds are breached.
3. **Transport Layer (MQTT):** Publishes formatted JSON payloads over local Wi-Fi.
4. **Gateway Layer (Raspberry Pi):** Subscribes to MQTT topics, saves to disk, and pushes to the web.
5. **Application Layer (Hostinger):** Catches HTTP POST requests and stores them in a remote database.

---

## 🛠️ Hardware Bill of Materials

| Component | Function | Protocol / Pins |
| :--- | :--- | :--- |
| **ESP32** | Main Microcontroller | Wi-Fi / GPIO |
| **Raspberry Pi** | Local Server / MQTT Broker | TCP/IP |
| **7-in-1 NPK Sensor** | Soil Moisture, Temp, pH, Nutrients | RS485 to TTL (RX:17, TX:16) |
| **AHT10** | Air Temperature & Humidity | I2C (SDA/SCL) |
| **BH1750** | Ambient Light (Lux) | I2C (SDA/SCL) |
| **3-Channel Relay** | Controls Fan, Pump, and Lights | Digital Out (13, 14, 26) |

---

## 💻 Software Installation

### 1. ESP32 Edge Node
1. Open `firmware/greenhouse_node.ino` in the Arduino IDE.
2. Install required libraries: `PubSubClient`, `AHT10`, `BH1750`.
3. Update your `ssid`, `password`, and `mqtt_server` (IP of your Raspberry Pi).
4. Flash to the ESP32.

### 2. Raspberry Pi Gateway
Ensure your Pi is running Mosquitto MQTT broker.

```bash
# Clone the repository
git clone https://github.com/Enavamsi/Edge-to-Cloud-Smart-Greenhouse-Automation-System.git
cd smart-greenhouse-iot/gateway

# Install dependencies
pip3 install -r requirements.txt

# Run the local logger
python3 soil_logger.py

# Run the cloud sync (in a separate terminal or as a service)
python3 site_entry.py
