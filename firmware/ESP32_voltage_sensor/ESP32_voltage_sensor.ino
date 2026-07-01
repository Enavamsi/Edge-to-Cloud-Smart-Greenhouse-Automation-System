
/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-measure-voltage
 */

#define ANALOG_IN_PIN  34 // ESP32 pin GPIO32 connected to voltage sensor
#define REF_VOLTAGE    3.3
#define ADC_RESOLUTION 4096.0
#define R1             34200.0 // resistor values in voltage sensor (ohms)
#define R2             8000.0  // resistor values in voltage sensor (ohms)

void setup() {
  Serial.begin(9600);

  // set ADC attenuation
  analogSetAttenuation(ADC_11db);
}

void loop() {

  int adc_value = analogRead(ANALOG_IN_PIN);

  float voltage_adc = ((float)adc_value * REF_VOLTAGE) / ADC_RESOLUTION;

  float voltage_in = voltage_adc * (R1 + R2) / R2;

  Serial.print("Measured Voltage = ");
  Serial.println(voltage_in, 2);

  delay(500);
}
