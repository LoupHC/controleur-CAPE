/*
  Greenhouse_prototype_CAPE_v.1.1.ino
  Copyright (C)2017 Loup Hébert-Chartrand. All right reserved

  This code has was made to interface with Arduino-like microcontrollers,
  for inclusion in greenhouse automation devices.

  Supported devices :
  - DS18B20 temperature sensor
  - DS3231 RTC module
  - 20x4 Serial LCD Display

  You can find the latest version of this code at :
  https://github.com/LoupHC/controleur-CAPE


  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

//***************************************************************************
//***************************************************************************
//***************************************************************************
/*
 *                                WARNING!!!
 *
 * CHANGING ANYTHING BELOW THIS LINE COULD HAVE UNSUSPECTED EFFECTS ON YOUR
 * CONTROLLER'S BEHAVIOUR.
 * CHECK "Greenhouse_parameters.h" TO CHANGE GREENHOUSE'S SETTINGS.
 *
 * HAVE A LOOK THO! IT'S WHERE THE REAL MAGIC TAKE PLACE!!
 * IF YOU MAKE ANY IMPROVEMENTS TO THIS CODE THAT COULD BENEFIT OTHER FARMERS, AND
 * WISH TO SHARE IT, YOU CAN CONTACT ME AT : loup.hebert.chartrand@gmail.com
 *
 *
*/
//***************************************************************************
//***************************************************************************
//***************************************************************************

#include "Arduino.h"

//********************LIBRARIES**************************
#include "Greenhouse_parameters.h"
#include "GreenhouseLib.h"

#define VERSION 101


Greenhouse greenhouse(TIMEZONE, LATITUDE, LONGITUDE, TIMEPOINTS, ROLLUPS, STAGES, FANS, HEATERS);

//********************POINTERS**************************

#if ROLLUPS >=1
  Rollup &R1 = greenhouse.rollup[0];
#endif
#if ROLLUPS == 2
  Rollup &R2 = greenhouse.rollup[1];
#endif
#if FANS >= 1
  Fan &F1 = greenhouse.fan[0];
  Fan &F2 = greenhouse.fan[1];
#endif
#if HEATERS >= 1
  Heater &H1 = greenhouse.heater[0];
#endif
#if HEATERS == 2
  Heater &H2 = greenhouse.heater[1];
#endif
#if TIMEPOINTS >= 1
  Timepoint &T1 = greenhouse.timepoint[0];
#endif
#if TIMEPOINTS >= 2
  Timepoint &T2 = greenhouse.timepoint[1];
#endif
#if TIMEPOINTS >= 3
  Timepoint &T3 = greenhouse.timepoint[2];
#endif
#if TIMEPOINTS >= 4
  Timepoint &T4 = greenhouse.timepoint[3];
#endif
#if TIMEPOINTS == 5
  Timepoint &T5 = greenhouse.timepoint[4];
#endif

//********************VARIABLES**************************

//Time array
byteParameter rightNow[6];
byte rightNowValue[6];
//Temperature inside the greenhouse
floatParameter greenhouseTemperature;
floatParameter greenhouseHumidity;
byte printx = 0;
boolean manualOverride = false;
boolean sensorFailure = false;
boolean EEPROMReset = false;
//********************SENSORS**************************
//See "Greenhouse_sensors.h" for sensor functions
#include "Greenhouse_sensors.h"

//********************INTERFACE**************************
//See "Greenhouse_interface.h" for LCD display functions
#include "Greenhouse_interface.h"

//********************ERRORS**************************
#include "Preprocessor_error_codes.h"


//***************************************************
//********************SETUP**************************
//***************************************************

void setup() {
  //start communication with serial monitor
  Serial.begin(9600);
  //start communication internal temp/humididty sensor
  Wire.begin();
  //start communication with LCD
  initLCD(20,4);
  //start communication with keypad
  #ifdef KEYPAD_DISPLAY
    keypad.begin( makeKeymap(keys) );
  #endif
  //start communication with temp probe
  #ifdef TEMP_DS18B20
    sensors.begin();
    sensors.setResolution(12);
  #endif
  //start communication with humidity probe
  #ifdef HUMIDITY_DHT
    dht.begin();
  #endif
  //start communication with clock
  #ifdef CLOCK_DS3231
    rtc.begin();
  #endif
  //start communication with relay driver
  #ifdef I2C_OUTPUTS
    mcp.begin();
    mcp.pinMode(SAFETY_PIN, OUTPUT);
    mcp.digitalWrite(SAFETY_PIN, HIGH);
  #endif
  // change RTC settings
  #ifdef RTC_TIME_SET
    rtc.setTime(HOUR_SET, MINUT_SET, SECOND_SET);
  #endif
  #ifdef RTC_DATE_SET
    rtc.setDate(DAY_SET, MONTH_SET, YEAR_SET);
  #endif
  //get sensors values
  getDateAndTime();
  getGreenhouseTemp();

  //set time within greenhouse object
  greenhouse.setNow(rightNowValue);

  //calculate hour saving, sunrise, sunset
  greenhouse.solarCalculations();

  //Load parameters from EEPROM or Greenhouse_parameters.h
  loadParameters();

//********************Parameters*******************

  //actual time, timepoint and targetTemp
  greenhouse.startingParameters();
}


//***************************************************
//*********************LOOP**************************
//***************************************************

void loop() {

  //actual time
  getDateAndTime();
  //actual temperature
  getGreenhouseTemp();
  //actual humididty
  getGreenhouseHum();
  //diplay infos on LCD screen
  lcdDisplay();
  //timepoint and target temperatures definitions, outputs routine
  greenhouse.fullRoutine(rightNowValue, greenhouseTemperature.value());
}

void loadParameters(){

    if(EEPROM[0] != VERSION){
      EEPROMReset = true;
      rtc.setTime(HOUR_SET, MINUT_SET, SECOND_SET);
      rtc.setDate(DAY_SET, MONTH_SET, YEAR_SET);
    }
    #ifdef COMPUTER_INTERFACE
      EEPROMReset = true;
    #endif

    //Define pinout for each devices
  #if ROLLUPS >= 1
    R1.initOutputs(ACT_HIGH, ROLLUP1_OPENING_PIN, ROLLUP1_CLOSING_PIN);
  #endif
  #if ROLLUPS == 2
    R2.initOutputs(ACT_HIGH, ROLLUP2_OPENING_PIN, ROLLUP2_CLOSING_PIN);
  #endif
  #if FANS >= 1
    F1.initOutput(ACT_HIGH, FAN1_PIN);
  #endif
  #if FANS == 2
    F2.initOutput(ACT_HIGH, FAN2_PIN);
  #endif
  #if HEATERS >= 1
    H1.initOutput(ACT_HIGH, HEATER1_PIN);
  #endif
  #if HEATERS == 2
    H2.initOutput(ACT_HIGH, HEATER2_PIN);
  #endif

    //If fresh start, set parameters from Greenhouse_parameters.h
    if(EEPROMReset == true){
    #if ROLLUPS >= 1
      R1.setParameters(R1_HYST, R1_ROTUP, R1_ROTDOWN, 100, R1_PAUSE, true);
      R1.setStageParameters(0,R1_S0_MOD, R1_S0_TARGET);
      R1.setStageParameters(1,R1_S1_MOD, R1_S1_TARGET);
      R1.setStageParameters(2,R1_S2_MOD, R1_S2_TARGET);
      R1.setStageParameters(3,R1_S3_MOD, R1_S3_TARGET);
      R1.setStageParameters(4,R1_S4_MOD, R1_S4_TARGET);
    #endif
    #if ROLLUPS == 2
      R2.setParameters(R2_HYST, R2_ROTUP, R2_ROTDOWN, 100, R2_PAUSE, true);
      R2.setStageParameters(0,R2_S0_MOD, R2_S0_TARGET);
      R2.setStageParameters(1,R2_S1_MOD, R2_S1_TARGET);
      R2.setStageParameters(2,R2_S2_MOD, R2_S2_TARGET);
      R2.setStageParameters(3,R2_S3_MOD, R2_S3_TARGET);
      R2.setStageParameters(4,R2_S4_MOD, R2_S4_TARGET);
    #endif
    #if FANS >= 1
      F1.setParameters(F1_MOD, F1_HYST, true);
    #endif
    #if FANS == 2
      F2.setParameters(F2_MOD, F2_HYST, true);
    #endif
    #if HEATERS >= 1
      H1.setParameters(H1_MOD, H1_HYST, true);
    #endif
    #if HEATERS == 2
      H2.setParameters(H2_MOD, H2_HYST, true);
    #endif
    #if TIMEPOINTS >= 1
      T1.setParameters(TP1_TYPE, TP1_HOUR, TP1_MN_MOD, TP1_HEAT, TP1_COOL, TP1_RAMP);
    #endif
    #if TIMEPOINTS >= 2
      T2.setParameters(TP2_TYPE, TP2_HOUR, TP2_MN_MOD, TP2_HEAT, TP2_COOL, TP2_RAMP);
    #endif
    #if TIMEPOINTS >= 3
      T3.setParameters(TP3_TYPE, TP3_HOUR, TP3_MN_MOD, TP3_HEAT, TP3_COOL, TP3_RAMP);
    #endif
    #if TIMEPOINTS >= 4
      T4.setParameters(TP4_TYPE, TP4_HOUR, TP4_MN_MOD, TP4_HEAT, TP4_COOL, TP4_RAMP);
    #endif
    #if TIMEPOINTS == 5
      T5.setParameters(TP5_TYPE, TP5_HOUR, TP5_MN_MOD, TP5_HEAT, TP5_COOL, TP5_RAMP);
    #endif


    }

    else{
      #if ROLLUPS >= 1
        R1.loadEEPROMParameters();
        R1.setIncrements(100);
        R1.setSafety(true);
      #endif
      #if ROLLUPS == 2
        R2.loadEEPROMParameters();
        R2.setIncrements(100);
        R2.setSafety(true);
      #endif
      #if FANS >=1
        F1.loadEEPROMParameters();
        F1.setSafety(true);
      #endif
      #if FANS == 2
        F2.loadEEPROMParameters();
        F2.setSafety(true);
      #endif
      #if HEATERS >=1
        H1.loadEEPROMParameters();
        H1.setSafety(true);
      #endif
      #if HEATERS ==2
        H2.loadEEPROMParameters();
        H2.setSafety(true);
      #endif

      #if TIMEPOINTS >= 1
      T1.loadEEPROMParameters();
      #endif
      #if TIMEPOINTS >= 2
        T2.loadEEPROMParameters();
      #endif
      #if TIMEPOINTS >= 3
        T3.loadEEPROMParameters();
      #endif
      #if TIMEPOINTS >= 4
        T4.loadEEPROMParameters();
      #endif
      #if TIMEPOINTS == 5
        T5.loadEEPROMParameters();
      #endif

    }

    EEPROM[0] = VERSION;
    EEPROMReset = false;
}
