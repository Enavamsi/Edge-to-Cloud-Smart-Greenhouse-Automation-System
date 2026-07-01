#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <AHT10.h>
#include <BH1750.h>

// --- Configuration ---
const char* ssid = "CONNECT AND DIE";
const char* password = "nfdq5490";
const char* mqtt_server = "10.151.201.2"; 
const int mqtt_port = 1883;

// --- Sensor Objects ---
#define RXD2 17
#define TXD2 16
HardwareSerial rs485(1);
AHT10 myAHT10(AHT10_ADDRESS_0X38);
BH1750 lightMeter;

// --- Modbus Setup ---
byte queryData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
byte buffer[25];

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.print("\nConnecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void reconnect() {

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32_MultiSensor_Node")) {
      Serial.println("connected");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(); // Shared I2C bus for AHT10 and BH1750
  
  // Initialize RS485
  rs485.begin(4800, SERIAL_8N1, RXD2, TXD2);

  // Initialize AHT10
  if (myAHT10.begin() != true) Serial.println("AHT10 Fail");
  
  // Initialize BH1750
  if (lightMeter.begin()) Serial.println("BH1750 OK");

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // 1. Read Soil Sensor (RS485)
  rs485.write(queryData, sizeof(queryData));
  rs485.flush();
  delay(300);
  int len = 0;
  while(rs485.available()) buffer[len++] = rs485.read();

  float soilMoist = 0, soilTemp = 0, ph = 0;
  int n = 0, p = 0, k = 0;

  if(len >= 19) {
    soilMoist = ((buffer[3] << 8) | buffer[4]) / 10.0;
    soilTemp = ((buffer[5] << 8) | buffer[6]) / 10.0;
    ph = ((buffer[9] << 8) | buffer[10]) / 10.0;
    n = (buffer[11] << 8) | buffer[12];
    p = (buffer[13] << 8) | buffer[14];
    k = (buffer[15] << 8) | buffer[16];
  }

  // 2. Read Air Sensor (AHT10)
  float airTemp = myAHT10.readTemperature();
  float airHumid = myAHT10.readHumidity();

  // 3. Read Light (BH1750)
  float lux = lightMeter.readLightLevel();

  // 4. Construct JSON Payload
  String payload = "{";
  payload += "\"soil_moist\":" + String(soilMoist) + ",";
  payload += "\"soil_temp\":" + String(soilTemp) + "3,";
  payload += "\"ph\":" + String(ph) + ",";
  payload += "\"n\":" + String(n) + ",";
  payload += "\"p\":" + String(p) + ",";
  payload += "\"k\":" + String(k) + ",";
  payload += "\"air_temp\":" + String(airTemp) + ",";
  payload += "\"air_humid\":" + String(airHumid) + ",";
  payload += "\"lux\":" + String(lux);
  payload += "}";

  // 5. Publish
  client.publish("farm/sensors", payload.c_str());
  
  Serial.println("Published: " + payload);
  delay(1000); // 10 second interval
}