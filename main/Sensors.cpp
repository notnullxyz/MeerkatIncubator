/*
   Sensors.cpp - library for GreenButler specific sensor control
   Copyright Marlon van der Linde <marlon250f@gmail.com> 2018
*/

#include "Sensors.h"
#include "Arduino.h"
#include "parameters.h"

#include <dht.h>

dht DHT;

Sensors::Sensors() {
  begin();
}

void Sensors::begin() {
  // initialise any sensor stuff
}

/** 
 * Read actual sensor values and store them in the instance variables for later retrieval. 
 * This is intended to be read on a periodic basis, remember that some sensors take longer than others
 * to get output.
 */
void Sensors::getActualReadings() {

  // Read DHT sensor
  int chk = DHT.read11(PIN_SENSOR_DHT11);
  // chk = DHTLIB_OK, DHTLIB_ERROR_CHECKSUM, DHTLIB_ERROR_TIMEOUT
  lastTemperature = DHT.temperature;
  lastHumidity = DHT.humidity;
}

/**
 * Simply return the last reading from the temperature sensor
 */
float Sensors::getLastTemperature() {
  return this->lastTemperature;
}

/**
 * Simply return the last reading from the humidity sensor
 */
float Sensors::getLastHumidity() {
  return this->lastHumidity;
}

