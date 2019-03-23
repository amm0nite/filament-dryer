
#include <SPI.h>
#include <Adafruit_MAX31855.h>

#define MAXDO   3
#define MAXCS   4
#define MAXCLK  5

#define MOTOR_IN1 7
#define MOTOR_IN2 8

#define HB_TARGET_TEMP 50.0
#define HB_CHECK_DELAY 500

Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

unsigned long previousMillis = 0;
unsigned long heatbedTimer = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);

  while (!Serial);

  Serial.begin(9600);
  Serial.println("Spool Dryer");
  
  // wait for MAX chip to stabilize
  delay(500);
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long delta = (currentMillis > previousMillis) ? currentMillis - previousMillis : 0;
  previousMillis = currentMillis;
  
  heatbedTimer += delta;
  if (heatbedTimer > HB_CHECK_DELAY) {
    heatbed();
    heatbedTimer = 0;
  }
}

void heatbed() {
  Serial.print("Internal Temp = ");
  Serial.println(thermocouple.readInternal());
  
  double c = thermocouple.readCelsius();
  if (isnan(c)) {
    Serial.println("Something wrong with thermocouple!");
  } else {
    Serial.print("C = ");
    Serial.println(c);
  }

  if (c < HB_TARGET_TEMP) {
    digitalWrite(MOTOR_IN2, HIGH);
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(LED_BUILTIN, LOW);
  }
}
