
#include "Arduino.h"

#ifndef Defines_h
#define Defines_h


//Debug lines
//#define DEBUG_ROLLUP_TIMING
//#define DEBUG_ROLLUP_TEMP
//#define DEBUG_FAN
//#define DEBUG_HEATER
//#define DEBUG_EEPROM
//#define DEBUG_PROGRAM
//#define DEBUG_SOLARCALC
//#define DEBUG_CLOCK

//Outputs type
#define IOS_OUTPUTS
//#define MCP_I2C_OUTPUTS

#define TIMEPOINT_INDEX 0
#define ROLLUP_INDEX 50
#define FAN_INDEX 80
#define HEATER_INDEX 100

#define TYPE_INDEX 0
#define MOD_INDEX 1
#define HOUR_INDEX 2
#define MIN_INDEX 3
#define HEAT_INDEX 4
#define COOL_INDEX 5

#define TEMP_INDEX 0
#define HYST_INDEX 1
#define SAFETY_INDEX 2
#define ROTATION_UP_INDEX 3
#define ROTATION_DOWN_INDEX 4
#define INCREMENTS_INDEX 5
#define PAUSE_INDEX 6

#define VAR_TEMP 0
#define FIX_TEMP 1
#define MAN_TEMP 2

#define ACT_HIGH 1
#define ACT_LOW 0

#define OFF_VAL 255
#define SAFETY_DELAY 1800000

#define HOUR 2
#define MINUT 1
#define SR 0
#define CLOCK 1
#define SS 2

#define ON true
#define OFF false

#ifdef MCP_I2C_OUTPUTS
  #include <Wire.h>
  #include "Adafruit_MCP23008.h"
  extern  Adafruit_MCP23008 mcp;
#endif


#endif
