
#include <SPI.h>
#include <Adafruit_MAX31855.h>
#include <Wire.h>

#define MAXDO   3
#define MAXCS   4
#define MAXCLK  5

#define MOTOR_IN1 7
#define MOTOR_IN2 8

#define HB_TARGET_TEMP 50.0
#define HB_CHECK_DELAY 500

Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

typedef enum {
  COOLING,
  HEATING,
  BLOCKED,
}fds;

unsigned long previousMillis = 0;
unsigned long heatbedTimer = 0;
double heatbedTemperature = 0;
fds globalState = fds::COOLING;

void setup() {
  Wire.begin(8);
  Wire.onRequest(requestEvent);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);

  while (!Serial);

  Serial.begin(9600);
  Serial.println("Filament Dryer");
  
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
  //Serial.print("Internal Temp = ");
  //Serial.println(thermocouple.readInternal());
  
  double temp = thermocouple.readCelsius();
  if (isnan(temp)) {
    Serial.println("Something wrong with thermocouple!");
    globalState = fds::BLOCKED;
  }

  //Serial.print("C = ");
  //Serial.println(temp);
  heatbedTemperature = temp;
  
  if (globalState != fds::BLOCKED) {
    if (temp < HB_TARGET_TEMP) {
      globalState = fds::HEATING;
    } else {
      globalState = fds::COOLING;
    }
  }
  
  if (globalState == fds::HEATING) {
    digitalWrite(MOTOR_IN2, HIGH);
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

struct i2c_message {
  double temp;
  fds state;
};

void requestEvent() {
  i2c_message msg;
  msg.temp = heatbedTemperature;
  msg.state = globalState;
  
  Wire.write((uint8_t *) &msg, sizeof(msg));
  Serial.print("SENT = ");
  Serial.println(sizeof(msg));
}
