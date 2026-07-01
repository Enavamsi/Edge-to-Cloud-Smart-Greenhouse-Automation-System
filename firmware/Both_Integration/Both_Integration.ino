#include <Wire.h>
#include <BH1750.h>
#include <AHT10.h>

// Initialize sensor objects
BH1750 lightMeter;
AHT10 myAHT10(AHT10_ADDRESS_0X38);

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial monitor to open
  
  // Initialize I2C bus
  Wire.begin();
  
  Serial.println(F("--- I2C Sensor Mesh Initializing ---"));

  // Initialize BH1750
  if (lightMeter.begin()) {
    Serial.println(F("BH1750 Light Sensor: OK"));
  } else {
    Serial.println(F("BH1750 Light Sensor: Check Wiring!"));
  }

  // Initialize AHT10
  if (myAHT10.begin()) {
    Serial.println(F("AHT10 Temp/Hum Sensor: OK"));
  } else {
    Serial.println(F("AHT10 Temp/Hum Sensor: Check Wiring/Calibration!"));
  }
  
  Serial.println(F("------------------------------------"));
}

void loop() {
  // --- Read BH1750 Light Level ---
  float lux = lightMeter.readLightLevel();
  
  // --- Read AHT10 Data ---
  float temp = myAHT10.readTemperature();
  float hum  = myAHT10.readHumidity();

  // --- Output to Serial ---
  Serial.print(F("[BH1750] Light: "));
  Serial.print(lux);
  Serial.println(F(" lx"));

  if (temp != 255) { // Check for AHT10 error value
    Serial.print(F("[AHT10]  Temp:  "));
    Serial.print(temp);
    Serial.print(F(" C | Hum: "));
    Serial.print(hum);
    Serial.println(F(" %"));
  } else {
    Serial.println(F("[AHT10] Error: Failed to read sensor"));
  }

  Serial.println(F("------------------------------------"));

  // The AHT10 recommends a slow polling rate (8s - 30s)
  // We'll use 5 seconds as a middle ground for this test.
  delay(5000);
}