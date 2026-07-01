#include <WiFi.h>
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

// Query for 7 registers: Moisture, Temp, (skip EC), PH, N, P, K
byte queryData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};

void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32-Vamsi-Node-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("esp32/commands");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

  // I2C Sensors (SDA: 21, SCL: 22)
  Wire.begin(21, 22);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  myAHT10.begin();

  // RS485 Setup
  Serial2.begin(4800, SERIAL_8N1, RXD2, TXD2); 
  pinMode(RE_DE_PIN, OUTPUT);
  digitalWrite(RE_DE_PIN, LOW);

  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 10000) { // Publish every 10 seconds
    lastMsg = now;

    // 1. Read Ambient I2C Sensors
    float lux = lightMeter.readLightLevel();
    float airTemp = myAHT10.readTemperature();
    float airHum = myAHT10.readHumidity();

    // 2. Read RS485 Soil Sensor
    digitalWrite(RE_DE_PIN, HIGH);
    Serial2.write(queryData, sizeof(queryData));
    Serial2.flush();
    digitalWrite(RE_DE_PIN, LOW);

    delay(500); // Wait for sensor processing

    byte buffer[20];
    int len = 0;
    while (Serial2.available() && len < 20) {
      buffer[len++] = Serial2.read();
    }

    String soilJson = "";
    if (len >= 19) {
      float sMoist = ((buffer[3] << 8) | buffer[4]) / 10.0;
      int16_t rawSTemp = (buffer[5] << 8) | buffer[6];
      float sTemp = rawSTemp / 10.0;
      float sPH = ((buffer[9] << 8) | buffer[10]) / 10.0;
      int N = (buffer[11] << 8) | buffer[12];
      int P = (buffer[13] << 8) | buffer[14];
      int K = (buffer[15] << 8) | buffer[16];

      soilJson = ",\"soilMoist\":" + String(sMoist) + 
                 ",\"soilTemp\":" + String(sTemp) + 
                 ",\"pH\":" + String(sPH) + 
                 ",\"N\":" + String(N) + 
                 ",\"P\":" + String(P) + 
                 ",\"K\":" + String(K);
    }

    // 3. Construct and Publish Payload
    String payload = "{\"lux\":" + String(lux) + 
                     ",\"airTemp\":" + String(airTemp) + 
                     ",\"airHum\":" + String(airHum) + 
                     soilJson + "}";

    Serial.println("Publishing: " + payload);
    client.publish("esp32/data", payload.c_str());
  }
}