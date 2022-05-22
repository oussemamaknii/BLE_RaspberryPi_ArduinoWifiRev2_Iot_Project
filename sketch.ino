#include <Arduino.h>
#include <ArduinoBLE.h>
#include "arduino_bma456.h"
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Arduino_LSM6DS3.h>


const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 9, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
float latitude, longitude;
SoftwareSerial gpSerial(2, 3);
TinyGPS gps;

//----------------------------------------------------------------------------------------------------------------------
// BLE UUIDs
//----------------------------------------------------------------------------------------------------------------------

#define BLE_UUID_SENSOR_DATA_SERVICE "8158b2fd-94e4-4ff5-a99d-9a7980e998d7"
#define BLE_UUID_MULTI_SENSOR_DATA "d508bc0a-ecdd-5ba5-9d33-13d5887a7718"
#define BLE_UUID_PASSWORD "7dfe60ef-175f-5a61-ac33-8a658b07cdb7"

#define NUMBER_OF_SENSORS 3

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
bool once = true;
union Response response;


BLEService sensorDataService(BLE_UUID_SENSOR_DATA_SERVICE);
BLECharacteristic multiSensorDataCharacteristic(BLE_UUID_MULTI_SENSOR_DATA, BLERead | BLENotify, sizeof multiSensorData.bytes);
BLECharacteristic passwordCharacteristic(BLE_UUID_PASSWORD, BLERead | BLEWrite | BLENotify, sizeof check);

void setupSensor() {
  bma456.initialize(RANGE_4G, ODR_1600_HZ, NORMAL_AVG4, CONTINUOUS);
  bma456.stepCounterEnable();

  lcd.begin(16, 2);

  gpSerial.begin(9600);

  // if (IMU.begin()) {
  //   Serial.println("completed successfully.");
  // }
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

  if (setupBleMode()) {
    Serial.println(BLE.address());
    // multiSensorData.values[0] = 15.26;
  }
}

void loop_sensors() {
  step = bma456.getStepCounterOutput();
  // Serial.print("step : ");
  // Serial.println(step);

  lcd.clear();
  lcd.print("STEP : ");
  lcd.setCursor(0, 6);
  lcd.print(step);

  if (gps.encode(gpSerial.read())) {
    gps.f_get_position(&latitude, &longitude);
    Serial.print(latitude);
    Serial.println(longitude);
    multiSensorData.values[1] = longitude;
    multiSensorData.values[2] = latitude;
  }

  //  char buffer[8];    // string buffer for use with dtostrf() function
  //  float ax, ay, az;  // accelerometer values
  //  float gx, gy, gz;  // gyroscope values

  //  // Retrieve and print IMU values
  //  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()
  //     && IMU.readAcceleration(ax, ay, az) && IMU.readGyroscope(gx, gy, gz)) {
  //       multiSensorData.values[3] = dtostrf(ax, 4, 1, buffer);
  //       multiSensorData.values[4] = dtostrf(ay, 4, 1, buffer);
  //       multiSensorData.values[5] = dtostrf(az, 4, 1, buffer);
  //       multiSensorData.values[6] = dtostrf(gx, 4, 1, buffer);
  //       multiSensorData.values[7] = dtostrf(gy, 4, 1, buffer);
  //       multiSensorData.values[8] = dtostrf(gz, 4, 1, buffer);
  //  }
}


void loop() {
  static long previousMillis = 0;

  loop_sensors();

  BLEDevice central = BLE.central();

  if (central) {

    Serial.print("Incoming connection from: ");
    Serial.println(central.address());

    while (central.connected()) {
      long interval = 2000;
      unsigned long currentMillis = millis();

      if (currentMillis - previousMillis > interval) {
        if (passwordCharacteristic.written() == 1) {
          passwordCharacteristic.readValue(value, 10);
          if (memcmp(check, value, sizeof(check)) == 0) {
            response.value = 1;
            Serial.println("yo");
            passwordCharacteristic.writeValue(response.bytes, sizeof response.bytes);
          } else {
            response.value = 0;
            Serial.println("yoo");
            passwordCharacteristic.writeValue(response.bytes, sizeof response.bytes);
          }
        }
        if (memcmp(check, value, sizeof(check)) == 0) {
          multiSensorData.values[0] = step;
          Serial.println("Sending");
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
