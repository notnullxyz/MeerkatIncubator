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

// To keep track of device runtimes.
unsigned long startTimeLamp;
unsigned long startTimeHumidifier;
unsigned long startTimeFan;

uint8_t lampOn = 0;

enum DisplayMode {DISPLAYMODE_SENSORS = 1, DISPLAYMODE_STATUSES = 0}; // toggle switch for display mode.
DisplayMode displayMode;

void setup() {
  pinMode(PIN_DISPLAY_MODE_TOGGLESW, INPUT);

  Serial.begin(115200);

  // In the beginnining...
  Clock.begin();
  relay.begin();
  display.begin();
  sensors.begin();

  wdt_enable(WDTO_8S);  // watchdog threshold to 8 secs
  configureTimers();
  setupAlarms();
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
  if (alarmsFired & 2) { // check alarm type 2
    handleAlarm();
  }
}

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
  if ((timercount % COUNTER_10S) == 0) task_10S();
  if ((timercount % COUNTER_1M) == 0) task_1M();
  if ((timercount % COUNTER_15M) == 0) task_15M();
  if ((timercount % COUNTER_1H) == 0) task_1H();
}

// This function is called every 5 seconds
void task_1S() {
  Serial.print(F("1S"));
  checkAndResetReadingIndicator();

  // do a check for OK'ness every second, and stop stuff if it must be stopped.
  checkTempOKTurnOffLamp();
  checkAndStopFan();
  checkAndStopHumidifier();

  triggerScreenUpdate();
}

// This function is called every 10 Seconds
void task_10S() {
  Serial.print(F("10S"));
  readSensorsAndNotify(); // reading sensors every ten seconds is good enough ?

  if (checkTempTooLow() == 1) {
    Serial.println(F("temp low - start lamp"));
    checkAndSetAndStartLamp();
  }
}

// This function is called every minute
void task_1M() {
  Serial.print(F("1M"));

  // once per minute, check for humidity upper limit , and kick off the humid alarm
  if (sensors.getLastHumidity() >= HUMIDITY_MAX) {
    Serial.println(" humid high - start fan and/or stop humidifier");
    handleHumidLimitAlarm();
  }

  // once per minute check if humidity is too low, and kick off the humidifier
  if (sensors.getLastHumidity() <= HUMIDITY_MIN) {
    Serial.println(" humid low - start humidifier and/or stop fan ");
    handleHumidLimitAlarm();
  }

  
}

// This function is called every 15 minutes
void task_15M() {
  Serial.print(F("15M"));
}

// This function is called every Hour
void task_1H() {
  Serial.print(F("1H"));  
}

// Get the display toggle switch position.
int getDisplayToggleSwitchPosition() {
  int switchPos = digitalRead(PIN_DISPLAY_MODE_TOGGLESW);
  displayMode = (switchPos == HIGH) ? 1 : 0;
  return displayMode;
}

/**
   Manages a nifty reading indicator on the lcd, times it, and clears it when ready.
*/
void checkAndResetReadingIndicator() {
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
  if (displayMode == DISPLAYMODE_SENSORS) {
    display.showReadingIndicator();
    readingIndicatorActive = 1;
  }
  sensors.getActualReadings();
}

// Keep screen update logic separated out of the timer code.
void triggerScreenUpdate() {

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

/**
   Checks if lamp need to start based on temperature too low.
   Returns 1 if lamp should start, or 0 if not.
*/
int checkTempTooLow() {
  float temperature = sensors.getLastTemperature();
  bool tempTooLow = false;

  if (temperature < DEGREES_MIN) {
    tempTooLow = true;
  }

  return tempTooLow ? 1 : 0;
}

/**
   Stop (and reset) the lamp if temperature is over max
*/
void checkTempOKTurnOffLamp() {

  // we can skip all the checks and logic if the fans aren't ectually running
  if (relay.statusLamp() == true) {
    float temperature = sensors.getLastTemperature();
    bool tempNormal = false;

    if (temperature > DEGREES_MIN) {
      tempNormal = true;
    }

    if (tempNormal) {
      relay.stopLamp();
    }
  }
}

/**
   Start the lamp after doing any neccesary checks and sets.
*/
void checkAndSetAndStartLamp() {
  if (LAMP_DISABLE == 0) {
    startTimeLamp = millis();
    relay.startLamp();
    lampOn = 1;
  }
}


// stop and reset the fan if the reasons for running it is over.
void checkAndStopFan() {
  // we can skip all the checks and logic if the fan isn't ectually running
  if (relay.statusFan() == true) {
    float humidity = sensors.getLastHumidity();
    bool humidNormal = false;

    if (humidity < HUMIDITY_MAX) {
      humidNormal = true;
    }

    if (humidNormal) {
      relay.stopFan();
    }
  }
}

// stop and reset the humidifier if the reason for running it is over.
void checkAndStopHumidifier() {
  // we can skip all the checks and logic if the humidifier isn't ectually running
  if (relay.statusHumidifier() == true) {
    float humidity = sensors.getLastHumidity();
    bool humidNormal = false;

    if (humidity < HUMIDITY_MAX) {
      humidNormal = true;
    }

    if (humidNormal) {
      relay.stopFan();
    }
  }
}

void setupAlarms() {
  Clock.disableAlarms();
  DateTime alarmTimestamp = Clock.read();
  alarmTimestamp.Hour = ALARM_DAILY_HOUR;
  alarmTimestamp.Minute = ALARM_DAILY_MINUTE;
  Clock.setAlarm(alarmTimestamp, DS3231_Simple::ALARM_DAILY);
}

/**
   This is the handler function for the main daily watering.
   Relays can be triggered here.
*/
void handleAlarm() {
  Serial.print("ALARM_MAIN!");
  checkAndSetAndStartLamp();
}

/**
   This function is called when the humidity is too high.
   The purpose here is to start the fan and extract humidity
*/
void handleHumidLimitAlarm() {
  // these two will handle too much humidity
  checkAndStopHumidifier();
  checkAndSetAndStartFan();

  // these two will handle too little humidity
  checkAndStopFan();
  checkAndSetAndStartHumidifier();
}

/**
   All prestart safety checks happen here.
   Will return true if safe enough to go ahead with start
   @return boolean
*/
bool startSafetyCheck() {
  // short circuited, no checks right now.
  return true;
}

/**
   Set start times, and then fire off the fan to get rid of humidity
*/
void checkAndSetAndStartFan() {
  if (!startSafetyCheck()) {
    Serial.print("Safety checks did not pass. Not starting anything now.");
    return;
  }

  if (FAN_DISABLE == 0) {
    startTimeFan = millis();
    relay.startFan();
  }
}

/**
 * Set the start times and then fire off the humidifier
 */
void checkAndSetAndStartHumidifier() {
  if (!startSafetyCheck()) {
    Serial.print("Safety checks did not pass. Not starting anything now.");
    return;
  }

  if (HUMIDIFIER_DISABLE == 0) {
    startTimeHumidifier = millis();
    relay.startHumidifier();
  }
  
}

/**
   Unset the start time of the humidifier, if its running, then switch it off after a prefined amount of time.
*/
void checkAndResetAndTerminateHumidifier() {
  unsigned long currentMillis = millis();

  // check how long X has been running, if it meets parameter criteria, stop it. (eg: dehumidify fan runtime?)

  if (relay.statusHumidifier() == true) {
    if (((currentMillis - startTimeHumidifier) / 1000) >= RUNTIME_SECONDS_HUMIDIFIER) {
      startTimeHumidifier = 0L;
      relay.stopHumidifier();
    }
  }
}


/**
   Unset the start time of the extractor fan, if its running, then switch it off after a prefined amount of time.
*/
void checkAndResetAndTerminateFan() {
  unsigned long currentMillis = millis();

  // check how long X has been running, if it meets parameter criteria, stop it. (eg: dehumidify fan runtime?)

  if (relay.statusHumidifier() == true) {
    if (((currentMillis - startTimeFan) / 1000) >= RUNTIME_SECONDS_FAN) {
      startTimeFan = 0L;
      relay.stopFan();
    }
  }
}

