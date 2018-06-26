#include <DS3231_Simple.h>

#include <dht.h>

#include <LiquidCrystal.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "parameters.h"
#include "Display.h"
#include "Relay.h"
#include "Sensors.h"

Display display;
Relay relay;
Sensors sensors;
DS3231_Simple Clock;

volatile uint16_t timercount;
uint8_t alarmsFired;
int readingIndicatorActive = 0;
int updateTimers = 0;
DateTime rtcDateTime;

enum DisplayMode {DISPLAYMODE_SENSORS = 1, DISPLAYMODE_STATUSES = 0}; // toggle switch for display mode.
DisplayMode displayMode;

// To keep track of device runtimes.
unsigned long startTimeLamp;
unsigned long startTimeHumidifier;
unsigned long startTimeFan;

void setup() {
  pinMode(PIN_DISPLAY_MODE_TOGGLESW, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  Serial.begin(115200);
  Serial.println(F("Startup/setup()"));

  // In the beginnining...
  Clock.begin();
  relay.begin();
  display.begin();
  sensors.begin();

  wdt_enable(WDTO_8S);  // watchdog threshold to 8 secs
  configureTimers();
  setupAlarms();
  buzzerTest();
  //whateverWillBuzz();
}

void buzzerTest() {
  Serial.println(F("buzzer test"));
  tone(PIN_BUZZER, 500, 200);
  delay(50);
  tone(PIN_BUZZER, 830, 200);
  delay(50);
  tone(PIN_BUZZER, 1130, 200);
  delay(50);
  tone(PIN_BUZZER, 1375, 200);
  delay(50);
  tone(PIN_BUZZER, 1690, 200);
  delay(50);
}

void buzzerHourly() {
  tone(PIN_BUZZER, 650, 300);
  delay(350);
  tone(PIN_BUZZER, 800, 300);
  delay(350);
  tone(PIN_BUZZER, 1250, 400);
  delay(350);  
}

void buzzerBlip() {
  tone(PIN_BUZZER, 200, 20);
}

void whateverWillBuzz() {
  tone(PIN_BUZZER, 391, 700); // que
  delay(600);
  tone(PIN_BUZZER, 349, 400);  // se
  delay(400);
  tone(PIN_BUZZER, 329, 650);  // ra
  delay(600);
  tone(PIN_BUZZER, 261, 400);  // se
  delay(400);
  tone(PIN_BUZZER, 329, 600);  // ra
  delay(900);
  tone(PIN_BUZZER, 369, 400);  // what
  delay(400);
  tone(PIN_BUZZER, 440, 250);  // ev
  delay(250);
  tone(PIN_BUZZER, 391, 250);  // ver
  delay(250);
  tone(PIN_BUZZER, 329, 250);  // will
  delay(250);
  tone(PIN_BUZZER, 293, 600);  // be
  delay(700);
  tone(PIN_BUZZER, 220, 300);  // will
  delay(300);
  tone(PIN_BUZZER, 293, 800);  // be
  delay(500);
}


void loop() {
  wdt_reset();  // reset watchdog, still ok here.

  if (updateTimers != 0)
  {
    timercount++;
    checkScheduler();
    updateTimers = 0;
  }

  alarmsFired = Clock.checkAlarms();
  if (alarmsFired & 2) { // check alarm type 2 - and handle it if its time!
    handleAlarm();
  }
}


//
// ========================================================= SCHEDULERS INTERUPTS TIMERS  ========================================================
//

/*
  Configure the timer/s and interrupts.
  This goes out of sight and out of mind quick, read about it, remember it!
*/
void configureTimers() {
  cli();                  // disable global interrupts
  TCCR1A = TCCR1B = 0;    // set entire TCCR1A and B registers to 0
  OCR1A = 15624;          // set interrupt to desired timer value
  // 16 000 000 cpu clock / (1024 prescaler * 1 hz desired interrupt frequency) - 1

  TIMSK1 |= (1 << OCIE1A);  // set the OCIE1A bit to enable timer compare interrupt
  TCCR1B |= (1 << WGM12);   // set WGM12 bit to enable CTC mode
  TCCR1B |= (1 << CS10);    // setting the CS10 bit on the TCCR1B register, the timer is running.

  // ISR(TIMER1_OVF_vect) will be called when it overflows.
  // ISR(TIMER1_COMPA_vect) will be called when it compare matches.
  TCCR1B |= (1 << CS12);    // Setting CS10 and CS12 bits = clock/1024 prescaling
  sei();                    // enable global interrupts
}

/**
  The handler for timer1 compare match interrupt
*/
ISR(TIMER1_COMPA_vect) {
  updateTimers = 1;
}

/**
   This function checks the timercount variable as it is ticked by the interupt compares, and fires off the task functions.
   Do not over-read this, it is simple.
*/
void checkScheduler() {
  if ((timercount % COUNTER_1S) == 0) task_1S();
  if ((timercount % COUNTER_5S) == 0) task_5S();
  if ((timercount % COUNTER_1M) == 0) task_1M();
  if ((timercount % COUNTER_15M) == 0) task_15M();
  if ((timercount % COUNTER_1H) == 0) task_1H();
}

// This function is called every 5 seconds
void task_1S() {
  Serial.print(F("1S"));
  checkAndResetReadingIndicator();
  triggerScreenUpdate();
}

// This function is called every 10 Seconds
void task_5S() {
  Serial.print(F("5S"));
  readSensorsAndNotify(); // reading sensors every 5 seconds is good enough ?
}

// This function is called every minute
void task_1M() {
  Serial.print(F("1M"));
}

// This function is called every 15 minutes
void task_15M() {
  Serial.print(F("15M"));
}

// This function is called every Hour
void task_1H() {
  Serial.print(F("1H"));
  buzzerHourly();
  turnEggs();
}




// Get the display toggle switch position.
int getDisplayToggleSwitchPosition() {
  Serial.println(F("get sw pos"));  
  int switchPos = digitalRead(PIN_DISPLAY_MODE_TOGGLESW);
  displayMode = (switchPos == HIGH) ? 1 : 0;
  return displayMode;
}

/**
   Manages a nifty reading indicator on the lcd, times it, and clears it when ready.
*/
void checkAndResetReadingIndicator() {
  Serial.print(F("*"));  
  if (readingIndicatorActive == 1) {
    readingIndicatorActive++;
  } else if (readingIndicatorActive > 1) {
    display.clearReadingIndicator();
    readingIndicatorActive = 0;
  }
}

/**
   Keep the sensor reading calls and display updates together.
   This also times and shows a little reading indicator on the lcd.
*/
void readSensorsAndNotify() {
  Serial.println(F("^"));  
  if (displayMode == DISPLAYMODE_SENSORS) {
    display.showReadingIndicator();
    readingIndicatorActive = 1;
  }
  sensors.getActualReadings();
}

// Keep screen update logic separated out of the timer code.
void triggerScreenUpdate() {
  Serial.print(F(" ScrUpd "));  
  displayMode = getDisplayToggleSwitchPosition();

  // Update the display every 5 seconds with statuses depending on the toggle switch position.
  if (displayMode == DISPLAYMODE_STATUSES) {
    // display the relay statuses
    display.updateStatuses(
      relay.statusLamp(),
      relay.statusHumidifier(),
      relay.statusFan());

  } else if (displayMode == DISPLAYMODE_SENSORS) {
    // display the time, date, temp, humidity
    display.updateSensorReadings(
      sensors.getLastTemperature(),
      sensors.getLastHumidity(),
      getRealDate(),
      getRealTime());
  }
}


//
// ========================================================= RTC AND ALARM ========================================================
//

/**
   For setting RTC date and time.
   This should only be trigerred when operator is ready to send serial responses.
*/
void promptSerialForRealTime() {
  Clock.promptForTimeAndDate(Serial);
}

/**
   Return the RTC time string.
*/
char* getRealTime() {
  rtcDateTime = Clock.read();
  char timeString[9];
  sprintf_P(timeString, PSTR("%02d:%02d"), rtcDateTime.Hour, rtcDateTime.Minute);
  return timeString;
}

/**
   Return the RTC date string.
*/
char* getRealDate() {
  rtcDateTime = Clock.read();
  char dateString[8];
  sprintf_P(dateString, PSTR("%02d/%02d/%02d"), rtcDateTime.Day, rtcDateTime.Month, rtcDateTime.Year);
  return dateString;
}

// Setup a main daily alarm
void setupAlarms() {
  Clock.disableAlarms();
  DateTime alarmTimestamp = Clock.read();
  alarmTimestamp.Hour = ALARM_DAILY_HOUR;
  alarmTimestamp.Minute = ALARM_DAILY_MINUTE;
  Clock.setAlarm(alarmTimestamp, DS3231_Simple::ALARM_DAILY);
}

/**
   This is the handler function for the main daily alarm...
*/
void handleAlarm() {
  Serial.print(F("ALARM_MAIN!"));
}


//
// ========================================================= GRUNT WORK ========================================================
//


// ===================== ON OFF convenience function ===============================

void lampOn() {
  Serial.print(F("lamp: on"));
  relay.startLamp();
}

void lampOff() {
  Serial.print(F("lamp: off"));
  relay.stopLamp();
}

void fanOn() {
  Serial.print(F("fan: on"));
  relay.startFan();
}

void fanOff() {
  Serial.print(F("fan: off"));
  relay.stopFan();
}

void humidifierOn() {
  Serial.print(F("humidifier: on"));
  relay.startHumidifier();
}

void humidifierOff() {
  Serial.print(F("humidifier: off"));
  relay.stopHumidifier();
}


// ====================================== TEMPERATURE CONTROL ====================
/**
 * bool tempMax()
 * bool tempMin()
 * bool tempOK()
 */

// True if current temperature is at or over the max defined
bool tempMax() {
  float temperature = sensors.getLastTemperature();
  return (temperature >= DEGREES_MAX) ? true : false;
}

// True if current temperature is at or below the minimum defined
bool tempMin() {
  float temperature = sensors.getLastTemperature();
  return (temperature <= DEGREES_MIN) ? true : false;
}

// True if current temperature is between the min and max defined ranges
bool tempOK() {
  float temperature = sensors.getLastTemperature();
  return ((temperature >= DEGREES_MIN) && (temperature <= DEGREES_MAX)) ? true : false;
}


// ======================================== HUMIDITY CONTROL ====================

// True if current humidity is at or over the max defined
bool humidMax() {
  float humidity = sensors.getLastHumidity();
  return (humidity >= HUMIDITY_MAX) ? true : false;
}

// True if current humidity is at or below the minimum defined
bool humidMin() {
  float humidity = sensors.getLastHumidity();
  return (humidity <= HUMIDITY_MIN) ? true : false;
}

// True if current humidity is between the min and max defined ranges
bool humidOK() {
  float humidity = sensors.getLastHumidity();
  return ((humidity >= HUMIDITY_MIN) && (humidity <= HUMIDITY_MAX)) ? true : false;
}


// ========================================== CLIMATE COMMAND CENTRE! ======================

void periodicControl() {

  if (humidOK() || tempOK()) {  // if everything is ok, don't change anything.
    return;
  }

  if (!humidOK()) { // something needs ajdustment with humidity.
    
  }
  
}

// ========================================== EGG TRAY CONTROL ==============================


