/*
   Relay.cpp - library for GreenButler specific relay control
   Copyright Marlon van der Linde <marlon250f@gmail.com> 2018
*/
#include "Relay.h"
#include "Arduino.h"
#include "parameters.h"

Relay::Relay()
{
  begin();
}

void Relay::begin()
{
  // Initialise the relay pins for output.
  pinMode(PIN_RELAY1_LAMP, OUTPUT);
  pinMode(PIN_RELAY2_HUMIDIFIER, OUTPUT);
  pinMode(PIN_RELAY3_FAN, OUTPUT);
  //pinMode(PIN_RELAY4_PUMP_B, OUTPUT);

  // default off = HIGH (RelayBoard in use has a default OFF on HIGH)
  
  //digitalWrite(PIN_RELAY4_PUMP_B, HIGH);
  digitalWrite(PIN_RELAY1_LAMP, HIGH);
  digitalWrite(PIN_RELAY2_HUMIDIFIER, HIGH);
  digitalWrite(PIN_RELAY3_FAN, HIGH);
}

void Relay::startLamp()
{
  digitalWrite(PIN_RELAY1_LAMP, LOW);
  this->stateLamp = true;
}

void Relay::stopLamp()
{
  digitalWrite(PIN_RELAY1_LAMP, HIGH);
  this->stateLamp = false;
}

void Relay::startHumidifier()
{
  digitalWrite(PIN_RELAY2_HUMIDIFIER, LOW);
  this->stateHumidifier = true;
}


void Relay::stopHumidifier()
{
  digitalWrite(PIN_RELAY2_HUMIDIFIER, HIGH);
  this->stateHumidifier = false;
}

void Relay::startFan()
{
  digitalWrite(PIN_RELAY3_FAN, LOW);
  this->stateFan = true;
}


void Relay::stopFan()
{
  digitalWrite(PIN_RELAY3_FAN, HIGH);
  this->stateFan = false;
}


//void Relay::openSolenoid()
//{
//  digitalWrite(PIN_RELAY1_SOLENOID, LOW);
//  this->stateSolenoid = true;
//}
//void Relay::closeSolenoid()
//{
//  digitalWrite(PIN_RELAY1_SOLENOID, HIGH);
//  this->stateSolenoid = false;
//}



// Note: All state check functions invert the digitalRead, as the relays are OFF/OPEN at HIGH

bool Relay::statusLamp()
{
  this->stateLamp = !digitalRead(PIN_RELAY1_LAMP);
  return this->stateLamp;
}

bool Relay::statusHumidifier()
{
  this->stateHumidifier = !digitalRead(PIN_RELAY2_HUMIDIFIER);
  return this->stateHumidifier;
}

bool Relay::statusFan()
{
  this->stateFan = !digitalRead(PIN_RELAY3_FAN);
  return this->stateFan;
}

//bool Relay::statusSolenoid()
//{
//  this->stateSolenoid = !digitalRead(PIN_RELAY1_SOLENOID);
//  return this->stateSolenoid;
//}

