#include <WiFi.h>
#include <ESPping.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <BH1750.h>
#include <AHT10.h>
#include <HardwareSerial.h>

// --- Configuration ---
const char* ssid = "realme narzo 60x 5G";
const char* password = "n83c8y8n";
const char* mqtt_server = "10.238.152.2"; 
const int mqtt_port = 1883;

// --- Pin Definitions ---
#define RXD2 16
#define TXD2 17
#define RE_DE_PIN 4 

WiFiClient espClient;
PubSubClient client(espClient);
BH1750 lightMeter;
AHT10 myAHT10(AHT10_ADDRESS_0X38);

unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void callback(char* topic, byte* payload, unsigned long length) {
  // Handle incoming commands if needed
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("esp32/commands");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(1000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

  // I2C Setup (SDA=21, SCL=22)
  Wire.begin(21, 22);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  myAHT10.begin();

  // RS485 Setup
  Serial2.begin(4800, SERIAL_8N1, RXD2, TXD2); 
  pinMode(RE_DE_PIN, OUTPUT);
  digitalWrite(RE_DE_PIN, LOW);

  // Ping Test
  if(Ping.ping(mqtt_server, 3)) {
    Serial.println("Ping Success!");
  }

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;

    // 1. Read I2C Sensors
    float lux = lightMeter.readLightLevel();
    float airTemp = myAHT10.readTemperature();
    float airHum = myAHT10.readHumidity();

    // 2. Read RS485 Soil Sensor (7 Registers) [cite: 86]
    byte query[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08}; 
    byte resp[19];
    
    digitalWrite(RE_DE_PIN, HIGH);
    Serial2.write(query, sizeof(query));
    Serial2.flush();
    digitalWrite(RE_DE_PIN, LOW);

    delay(500);

    String soilData = "";
    if (Serial2.available() >= 19) {
      Serial2.readBytes(resp, 19);
      float sMoist = ((resp[3] << 8) | resp[4]) / 10.0;
      int16_t rawSTemp = (resp[5] << 8) | resp[6]; 
      float sTemp = rawSTemp / 10.0;
      unsigned int sEC = (resp[7] << 8) | resp[8];
      float sPH = ((resp[9] << 8) | resp[10]) / 10.0; 
      
      soilData = ",\"soilMoist\":" + String(sMoist) + 
                 ",\"soilTemp\":" + String(sTemp) + 
                 ",\"soilEC\":" + String(sEC) + 
                 ",\"soilPH\":" + String(sPH);
    }

    // 3. Construct JSON Payload
    String payload = "{\"lux\":" + String(lux) + 
                     ",\"airTemp\":" + String(airTemp) + 
                     ",\"airHum\":" + String(airHum) + 
                     soilData + "}";

    Serial.print("Publishing: ");
    Serial.println(payload);
    client.publish("esp32/data", payload.c_str());
  }
}