#include <Wire.h>
#include <BH1750.h>
#include <AHT10.h>
#include <HardwareSerial.h>

// RS485 Pins
#define RXD2 16
#define TXD2 17
#define RE_DE_PIN 4 

// I2C Sensors
BH1750 lightMeter;
AHT10 myAHT10(AHT10_ADDRESS_0X38);

void setup() {
  Serial.begin(115200);
  
  // Initialize I2C (SDA=21, SCL=22)
  Wire.begin(21, 22);
  
  // Initialize RS485
  Serial2.begin(4800, SERIAL_8N1, RXD2, TXD2); 
  pinMode(RE_DE_PIN, OUTPUT);
  digitalWrite(RE_DE_PIN, LOW);

  Serial.println("--- Starting Multi-Sensor System ---");
  
  if (lightMeter.begin()) Serial.println("BH1750 OK");
  if (myAHT10.begin()) Serial.println("AHT10 OK");
}

void loop() {
  // 1. READ I2C SENSORS
  float lux = lightMeter.readLightLevel();
  float airTemp = myAHT10.readTemperature();
  float airHum = myAHT10.readHumidity();

  // 2. READ RS485 SOIL SENSOR
  byte query[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
  byte response[19];

  digitalWrite(RE_DE_PIN, HIGH); 
  delay(10);
  Serial2.write(query, sizeof(query));  
  Serial2.flush();
  digitalWrite(RE_DE_PIN, LOW); 

  delay(1000); 

  Serial.println("\n--- Environment Data ---");
  Serial.print("Light: "); Serial.print(lux); Serial.println(" lx");
  Serial.print("Air Temp: "); Serial.print(airTemp); Serial.println(" C");
  Serial.print("Air Humi: "); Serial.print(airHum); Serial.println(" C");

  if (Serial2.available() >= 19) {
    Serial2.readBytes(response, 19);
    
    // Scaling based on datasheet [cite: 80, 89, 93, 97]
    float soilHum = ((response[3] << 8) | response[4]) / 10.0;
    int16_t sTempRaw = (response[5] << 8) | response[6];
    float soilTemp = sTempRaw / 10.0;
    unsigned int ec = (response[7] << 8) | response[8];
    float ph = ((response[9] << 8) | response[10]) / 10.0;
    unsigned int n = (response[11] << 8) | response[12];
    unsigned int p = (response[13] << 8) | response[14];
    unsigned int k = (response[15] << 8) | response[16];

    Serial.println("--- Soil Data ---");
    Serial.print("Soil Moisture: "); Serial.print(soilHum); Serial.println(" %");
    Serial.print("Soil Temp: "); Serial.print(soilTemp); Serial.println(" C");
    Serial.print("pH: "); Serial.println(ph);
    Serial.print("NPK: "); Serial.print(n); Serial.print("/"); Serial.print(p); Serial.print("/"); Serial.println(k);
  } else {
    Serial.println("Soil Sensor: No Response (Check Wiring/Power)");
  }

  delay(1000);
}