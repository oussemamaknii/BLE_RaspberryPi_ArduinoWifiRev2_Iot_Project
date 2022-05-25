#include <Arduino.h>
#include <ArduinoBLE.h>
#include "arduino_bma456.h"
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Arduino_LSM6DS3.h>

#define NUMBER_OF_SENSORS 4

LiquidCrystal lcd(12, 11, 5, 4, 9, 8);
float latitude, longitude;
SoftwareSerial gpSerial(2, 3);
TinyGPS gps;
uint32_t step = 0;

union multi_sensor_data {
  struct __attribute__((packed)) {
    float values[NUMBER_OF_SENSORS];
  };
  signed char bytes[NUMBER_OF_SENSORS * sizeof(float)];
};
union Response {
  struct __attribute__((packed)) {
    int value;
  };
  signed char bytes[sizeof(int)];
};


union multi_sensor_data multiSensorData;
uint8_t check[sizeof("password22")];
uint8_t value[sizeof("password22")];
union Response response;
boolean fall = false;


BLEService sensorDataService("8158b2fd-94e4-4ff5-a99d-9a7980e998d7");
BLECharacteristic multiSensorDataCharacteristic("d508bc0a-ecdd-5ba5-9d33-13d5887a7718", BLERead | BLENotify, sizeof multiSensorData.bytes);
BLECharacteristic passwordCharacteristic("7dfe60ef-175f-5a61-ac33-8a658b07cdb7", BLERead | BLEWrite | BLENotify, sizeof check);

void setupSensor() {
  bma456.initialize(RANGE_4G, ODR_1600_HZ, NORMAL_AVG4, CONTINUOUS);
  bma456.stepCounterEnable();

  lcd.begin(16, 2);

  gpSerial.begin(9600);

  if (IMU.begin()) {
  }
}

bool setupBleMode() {
  memmove(check, "password22", sizeof("password22"));

  if (!BLE.begin()) {
    return false;
  }

  BLE.setDeviceName("Station");
  BLE.setLocalName("Station");
  BLE.setAdvertisedService(sensorDataService);

  sensorDataService.addCharacteristic(multiSensorDataCharacteristic);
  sensorDataService.addCharacteristic(passwordCharacteristic);
  BLE.addService(sensorDataService);
  BLE.advertise();
  return true;
}

void setup() {
  Serial.begin(9600);

  setupSensor();

  setupBleMode();
}

void loop_sensors() {
  step = bma456.getStepCounterOutput();

  lcd.setCursor(0, 0);
  lcd.print("STEP : ");
  lcd.print(step);
  lcd.setCursor(0, 6);
  lcd.print("Fall : False");

  if (gps.encode(gpSerial.read())) {
    gps.f_get_position(&latitude, &longitude);
    Serial.print(latitude);
    Serial.println(longitude);
    if (longitude == 0 & latitude == 0) {
      multiSensorData.values[1] = 48.8246997;
      multiSensorData.values[2] = 2.2802696;
    } else {
      multiSensorData.values[1] = longitude;
      multiSensorData.values[2] = latitude;
    }
  }
  imu_loop();
}

void imu_loop() {
  
}

void loop() {

  loop_sensors();

  BLEDevice central = BLE.central();

  if (central) {
    while (central.connected()) {
      long interval = 2000;
      unsigned long currentMillis = millis();
      static long previousMillis = 0;

      if (currentMillis - previousMillis > interval) {
        if (passwordCharacteristic.written() == 1) {
          passwordCharacteristic.readValue(value, 10);
          if (memcmp(check, value, sizeof(check)) == 0) {
            response.value = 1;
            passwordCharacteristic.writeValue(response.bytes, sizeof response.bytes);
          } else {
            response.value = 0;
            passwordCharacteristic.writeValue(response.bytes, sizeof response.bytes);
          }
        }
        if (memcmp(check, value, sizeof(check)) == 0) {
          multiSensorData.values[0] = step;
          multiSensorDataCharacteristic.writeValue(multiSensorData.bytes, sizeof multiSensorData.bytes);
        } else {
          response.value = 0;
          passwordCharacteristic.writeValue(response.bytes, sizeof response.bytes);
        }
        previousMillis = currentMillis;
      }
    }
  }
}
