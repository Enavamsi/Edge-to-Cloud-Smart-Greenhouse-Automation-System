
#define RXD2 17
#define TXD2 16

HardwareSerial rs485(1);

// Modbus query to read 7 registers (Moisture → Potassium)
byte queryData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};

byte buffer[25];

void setup() {

  Serial.begin(115200);

  rs485.begin(4800, SERIAL_8N1, RXD2, TXD2);

  Serial.println("Soil Sensor Starting...");
}

void loop() {

  rs485.write(queryData, sizeof(queryData));
  rs485.flush();

  delay(300);

  int len = 0;

  while(rs485.available())
  {
    buffer[len++] = rs485.read();
  }

  if(len >= 19)
  {
    int moisture = (buffer[3] << 8) | buffer[4];
    int temperature = (buffer[5] << 8) | buffer[6];

    int ph = (buffer[9] << 8) | buffer[10];
    int nitrogen = (buffer[11] << 8) | buffer[12];
    int phosphorus = (buffer[13] << 8) | buffer[14];
    int potassium = (buffer[15] << 8) | buffer[16];

    Serial.println("------ Soil Data ------");

    Serial.print("Moisture: ");
    Serial.print(moisture / 10.0);
    Serial.println(" %");

    Serial.print("Temperature: ");
    Serial.print(temperature / 10.0);
    Serial.println(" °C");


    Serial.print("pH: ");
    Serial.println(ph / 10.0);

    Serial.print("Nitrogen: ");
    Serial.print(nitrogen);
    Serial.println(" mg/kg");

    Serial.print("Phosphorus: ");
    Serial.print(phosphorus);
    Serial.println(" mg/kg");

    Serial.print("Potassium: ");
    Serial.print(potassium);
    Serial.println(" mg/kg");

    Serial.println("-----------------------");
  }
  else
  {
    Serial.println("No valid response");
  }

  delay(2000);
}

