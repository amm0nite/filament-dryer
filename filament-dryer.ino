
#include <SPI.h>
#include <Adafruit_MAX31855.h>
#include <Wire.h>

#define MAXDO   3
#define MAXCS   4
#define MAXCLK  5

#define MOTOR_IN1 7
#define MOTOR_IN2 8

#define TARGET_TEMP 50.0
#define TARGET_SIZE 2
#define REFRESH_DELAY 500

#define MIN_TEMP 15
#define MAX_TEMP 80
#define MINIMUM_CHANGE 1
#define HEATING_TIMEOUT 1000
#define COOLING_TIMEOUT 10000

Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

typedef enum {
  INIT,
  ERROR_MIN_TEMP,
  ERROR_MAX_TEMP,
  ERROR_SENSOR,
  ERROR_TIMEOUT_HEATING,
  ERROR_TIMEOUT_COOLING,
  NOMINAL_HEATING,
  NOMINAL_COOLING,
}fds;

unsigned long previousMillis = 0;
unsigned long refreshTimer = 0;
unsigned long stateTimer = 0;
double heatbedTemperature = 0;
double stateStartTemperature = 0;
fds globalState = fds::INIT;

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
  
  refreshTimer += delta;
  stateTimer += delta;

  if (refreshTimer > REFRESH_DELAY) {
    tick();
    refreshTimer = 0;
  }
}

void tick() {
  updateTemperature();
  checkTemperature();
  
  manageState();

  heatbed();
}

void updateTemperature() {
  //Serial.print("Internal Temp = ");
  //Serial.println(thermocouple.readInternal());
  double temp = thermocouple.readCelsius();
  //Serial.print("C = ");
  //Serial.println(temp);
  heatbedTemperature = temp;
}

void changeState(fds newState) {
  stateTimer = 0;
  stateStartTemperature = heatbedTemperature;
  globalState = newState;

  Serial.print("changeState");
  Serial.println(newState);
}

void checkTemperature() {
  if (isnan(heatbedTemperature)) {
    changeState(fds::ERROR_SENSOR);
    return;
  }

  if (heatbedTemperature < MIN_TEMP) {
    changeState(fds::ERROR_MIN_TEMP);
    return;
  }

  if (heatbedTemperature > MAX_TEMP) {
    changeState(fds::ERROR_MAX_TEMP);
    return;
  }
}

void manageState() {
  if (globalState == fds::INIT) {
    changeState(fds::NOMINAL_COOLING);
    return;
  }

  if (globalState == fds::NOMINAL_COOLING) {
    if (stateTimer > COOLING_TIMEOUT && heatbedTemperature > (stateStartTemperature - MINIMUM_CHANGE)) {
      changeState(fds::ERROR_TIMEOUT_COOLING);
      return;
    }
    if (heatbedTemperature < (TARGET_TEMP - TARGET_SIZE)) {
      changeState(fds::NOMINAL_HEATING);
      return;
    }
    //cooling;
    return;
  }

  if (globalState == fds::NOMINAL_HEATING) {
    if (stateTimer > HEATING_TIMEOUT && heatbedTemperature < (stateStartTemperature + MINIMUM_CHANGE)) {
      changeState(fds::ERROR_TIMEOUT_HEATING);
      return;
    }
    if (heatbedTemperature > (TARGET_TEMP + TARGET_SIZE)) {
      changeState(fds::NOMINAL_COOLING);
      return;
    }
    //heating
    return;
  }
}

void heatbed() {
  if (globalState == fds::NOMINAL_HEATING) {
    digitalWrite(MOTOR_IN2, HIGH);
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
    return;
  }
  
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  return;
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
