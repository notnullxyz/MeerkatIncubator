
#define SYS_NAME_VERSION "MeerkatEgger v1.1"
#define SYS_GREETING "Hatch'em'All"

#define LCD_CHARS 16
#define LCD_LINES 2

#define TIMER_READ 5000

#define NAME_LAMP "Lamp"
#define NAME_HUMIDIFIER "Humidifier"
#define NAME_FAN "Fan"
//#define NAME_SOLENOID "XXX"

// Pin mapping from relay board to arduino IO
#define PIN_RELAY1_LAMP  10
#define PIN_RELAY2_HUMIDIFIER  9
#define PIN_RELAY3_FAN  8
//#define PIN_RELAY4_PUMP_B  11

// Pin mapping from LCD to arduino IO
#define PIN_LCD_RS 2
#define PIN_LCD_ENABLE 3
#define PIN_LCD_D4 4
#define PIN_LCD_D5 5
#define PIN_LCD_D6 6
#define PIN_LCD_D7 7

#define PIN_DISPLAY_MODE_TOGGLESW A3

// Sensors
#define PIN_SENSOR_DHT11 12

// Buzzer
#define PIN_BUZZER 13

// ----- RUNNING PARAMETERS -----
// -- This is the daily alarm
#define ALARM_DAILY_HOUR 00
#define ALARM_DAILY_MINUTE 01

// -- How many seconds the devices must run
#define RUNTIME_SECONDS_HUMIDIFIER 30
#define RUNTIME_SECONDS_FAN 10

// -- Disabling any devices? (1 = disable)
#define LAMP_DISABLE 0
#define HUMIDIFIER_DISABLE 0
#define FAN_DISABLE 0

// -- Start and stop the lamp if temperature is lower or higher than this:
#define DEGREES_MIN 24
#define DEGREES_MAX 28

// -- humidity range
#define HUMIDITY_MIN 45
#define HUMIDITY_MAX 70

// -- PADDING for adding tolerance/slack in the temperature and humidity min and maxes, 0 disables
#define HUMIDITY_SLACK_PERCENT 5
#define TEMPERATURE_SLACK_DEGREES 1

/* Timer Constants & Variables */
const int COUNTER_1S = 1;
const int COUNTER_5S = 5;
const int COUNTER_1M = 60;
const int COUNTER_15M = 900;
const int COUNTER_1H = 3600;


