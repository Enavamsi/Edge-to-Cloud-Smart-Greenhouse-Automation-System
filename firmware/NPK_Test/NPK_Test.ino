#include <SoftwareSerial.h>

#define RX_PIN 2
#define TX_PIN 3

SoftwareSerial rs485(RX_PIN, TX_PIN);

// Read 7 registers (Moisture to Potassium)
byte queryData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};

void setup() {
  Serial.begin(115200);
  rs485.begin(4800);  // Try 9600 if no response
}

void loop() {

  rs485.write(queryData, sizeof(queryData));
  rs485.flush();
  delay(300);

  byte buffer[30];
  int len = 0;

  while (rs485.available()) {
    buffer[len++] = rs485.read();
  }

  if (len >= 19) {   // 7 registers = 14 bytes data + header

    int moisture = (buffer[3] << 8) | buffer[4];
    int temp     = (buffer[5] << 8) | buffer[6];
    int ec_dummy = (buffer[7] << 8) | buffer[8];  // ignore
    int ph       = (buffer[9] << 8) | buffer[10];
    int nitrogen = (buffer[11] << 8) | buffer[12];
    int phosphorus = (buffer[13] << 8) | buffer[14];
    int potassium = (buffer[15] << 8) | buffer[16];

    Serial.println("----- Soil Data -----");

    Serial.print("Moisture: ");
    Serial.print(moisture / 10.0);
    Serial.println(" %");

    Serial.print("Temperature: ");
    Serial.print(temp / 10.0);
    Serial.println(" °C");

    Serial.print("pH: ");
    Serial.println(ph / 10.0);

    Serial.print("Nitrogen (N): ");
    Serial.print(nitrogen);
    Serial.println(" mg/kg");

    Serial.print("Phosphorus (P): ");
    Serial.print(phosphorus);
    Serial.println(" mg/kg");

    Serial.print("Potassium (K): ");
    Serial.print(potassium);
    Serial.println(" mg/kg");

    Serial.println("----------------------\n");
  }
  else {
    Serial.println("No valid response");
  }

  delay(2000);
}