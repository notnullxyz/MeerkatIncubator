/*
   Relay.h - library for GreenButler specific relay control
   Copyright Marlon van der Linde <marlon250f@gmail.com> 2018
*/
#ifndef Relay_h
#define Relay_h

class Relay
{
  public:
    Relay();
    void begin();

    // Functions pertaining to starting and activating relay controlled devices
    void startLamp();
    void startFan();
    void startHumidifier();
    //void openSolenoid();

    // Functions for stopping and deactivating relay controlled devices
    void stopLamp();
    void stopFan();
    void stopHumidifier();
    //void closeSolenoid();

    // Functions for simply returning the current status of relay controlled devices
    bool statusLamp();
    bool statusFan();
    bool statusHumidifier();
    //bool statusSolenoid();

  private:
    bool stateLamp = 0;
    bool stateFan = 0;
    bool stateHumidifier = 0;
    //bool stateSolenoid = 0;

    unsigned long startTimePumpA = 0L;
    unsigned long startTimePumpB = 0L;
};

#endif
