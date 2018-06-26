/*
   display.h - library for GreenButler specific display control
   Copyright Marlon van der Linde <marlon250f@gmail.com> 2018
*/
#include <LiquidCrystal.h>

#ifndef Display_h
#define Display_h

class Display
{
  public:
    LiquidCrystal lcd;

    Display();

    /**
       Construction and initialisation
    */
    void begin();

    /**
       Clear screen, cursor reset, and other specific preps for the LCD
    */
    void reset();

    /**
       Toggles blank/Mute of the greenbutler display.
    */
    void mute();

    /**
     * Updates the runScreen() layout with the given values for relay statuses.
     * @param bool lamp Boolean indicator whether the lamp is running or not
     * @param bool humidifier Boolean indicator whether humidifier is running or not
     * @param bool fan Boolean indicator whether the fan is running or not
     */
    void updateStatuses(bool lamp, bool humidifier, bool fan);

    /**
     * Updates the runScreen() layour with the given readings from the sensors with date and time.
     * @param float temp The current temperature reading
     * @param float humid The current humidity readings
     * @param String The string representation of the RTC time
     * @param String The string representation of the RTC date
     */
    void updateSensorReadings(float temp, float humid, char* realdate, char* realtime);

    /**
     * Show a sensor/input reading indicator
     */
    void showReadingIndicator();

    /**
     * Clear the sensor/input reading indicator
     */
    void clearReadingIndicator();
    
  private:
    int muted = 0;

};

#endif

