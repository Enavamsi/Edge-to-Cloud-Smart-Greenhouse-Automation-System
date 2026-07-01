#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <AHT10.h>
#include <BH1750.h>

// --- Network Configuration ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "192.168.1.100"; // Replace with your Raspberry Pi IP
const int mqtt_port = 1883;

// --- Actuator Pins ---
#define FAN_PIN 13  
#define PUMP_PIN 14
#define LIGHT_PIN 26

// --- Greenhouse Thresholds ---
const float TEMP_THRESHOLD = 25.0;     
const float MOISTURE_THRESHOLD = 40.0; 
const float LUX_THRESHOLD = 300.0;     

// --- Sensor Objects & Pins ---
#define RXD2 17
#define TXD2 16
HardwareSerial rs485(1);
AHT10 myAHT10(AHT10_ADDRESS_0X38);
BH1750 lightMeter;
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastControlRun = 0;
unsigned long lastReconnectAttempt = 0;
byte queryData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
byte buffer[30];

void setup() {
  Serial.begin(115200);
  Wire.begin(); 
  
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(FAN_PIN, HIGH);
  digitalWrite(PUMP_PIN, HIGH);
  digitalWrite(LIGHT_PIN, HIGH);

  rs485.begin(4800, SERIAL_8N1, RXD2, TXD2);
  
  if (myAHT10.begin() != true) Serial.println("AHT10 Fail");
  if (!lightMeter.begin()) Serial.println("BH1750 Fail");

  WiFi.begin(ssid, password);
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  unsigned long now = millis();

  // Non-blocking MQTT Reconnect
  if (!client.connected()) {
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      if (WiFi.status() == WL_CONNECTED) {
        client.connect("ESP32_Greenhouse_Node");
      }
    }
  } else {
    client.loop(); 
  }

  // 10-Second Sensor Loop
  if (now - lastControlRun > 10000) {
    lastControlRun = now;

    float airTemp = myAHT10.readTemperature();
    float airHumid = myAHT10.readHumidity();
    float lux = lightMeter.readLightLevel();

    rs485.write(queryData, sizeof(queryData));
    rs485.flush();
    delay(300);

    int len = 0;
    while(rs485.available()) {
      buffer[len++] = rs485.read();
    }

    float soilMoist = 0, soilTemp = 0, ph = 0;
    int n = 0, p = 0, k = 0;
    bool soilValid = false;

    if(len >= 19) {
      soilMoist = ((buffer[3] << 8) | buffer[4]) / 10.0;
      soilTemp = ((buffer[5] << 8) | buffer[6]) / 10.0;
      ph = ((buffer[9] << 8) | buffer[10]) / 10.0;
      n = (buffer[11] << 8) | buffer[12];
      p = (buffer[13] << 8) | buffer[14];
      k = (buffer[15] << 8) | buffer[16];
      soilValid = true;
    }

    // Actuation Logic
    digitalWrite(FAN_PIN, (airTemp > TEMP_THRESHOLD) ? LOW : HIGH);
    digitalWrite(PUMP_PIN, (soilValid && soilMoist < MOISTURE_THRESHOLD && soilMoist > 0) ? LOW : HIGH);
    digitalWrite(LIGHT_PIN, (lux < LUX_THRESHOLD) ? LOW : HIGH);

    // Telemetry Sync
    if (client.connected() && soilValid) {
      String payload = "{";
      payload += "\"soil_moist\":" + String(soilMoist) + ",";
      payload += "\"soil_temp\":" + String(soilTemp) + ",";
      payload += "\"ph\":" + String(ph) + ",";
      payload += "\"n\":" + String(n) + ",";
      payload += "\"p\":" + String(p) + ",";
      payload += "\"k\":" + String(k) + ",";
      payload += "\"air_temp\":" + String(airTemp) + ",";
      payload += "\"air_humid\":" + String(airHumid) + ",";
      payload += "\"lux\":" + String(lux);
      payload += "}";

      client.publish("farm/sensors", payload.c_str());
    }
  }
}
