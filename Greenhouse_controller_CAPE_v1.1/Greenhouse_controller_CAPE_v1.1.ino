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

#define VERSION 102


#define TIMEPOINTS 4
#define ONE_ROLLUP_AT_THE_TIME

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
#endif
#if FANS == 2
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

//********************VARIABLES**************************

//Time array
byteParameter rightNow[6];
byte rightNowValue[6];
//Temperature inside the greenhouse
floatParameter greenhouseTemperature;
floatParameter greenhouseHumidity;
float coolingTemperature;
float heatingTemperature;
//flags
boolean sensorFailure = false;
boolean EEPROMReset = false;

//********************SENSORS**************************
//See "Greenhouse_sensors.h" for sensor functions
#include "Greenhouse_sensors.h"
//********************INTERFACE**************************
//See "Greenhouse_overrides.h" for overrides functions
#include "Greenhouse_overrides.h"
//********************INTERFACE**************************
//See "Greenhouse_interface.h" for LCD display functions
#include "Greenhouse_interface.h"
//********************ERRORS**************************
//#include "Preprocessor_error_codes.h"


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
  //Temperature range
  greenhouseTemperature.setLimits(-180, 100);
  //last recorded value if probe doesnt reply back at first cycle
  if(EEPROM.read(1) == 111){
    float emergencyTemp = EEPROM.read(2);
    greenhouseTemperature.setValue(emergencyTemp);
  }
  else{
    greenhouseTemperature.setValue(20);
  }
  deshum = EEPROM.read(3);

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
  #ifdef MCP_I2C_OUTPUTS
    mcp.begin();
  #endif
  //Add safety alarm
  greenhouse.addAlarm(ACT_LOW, ALARM_PIN);
  #ifdef ALARM_MAX_TEMP
    greenhouse.setAlarmMaxTemp(ALARM_MAX_TEMP);
  #endif
  #ifdef ALARM_MIN_TEMP
    greenhouse.setAlarmMinTemp(ALARM_MIN_TEMP);
  #endif
  // change RTC settings
  #if defined(SET_TIME) && defined(CLOCK_DS3231)
    rtc.setTime(HR, MIN,0);
  #endif
  #if defined(SET_TIME) && defined(CLOCK_DS3231)
    rtc.setDate(DAY, MONTH, YEAR);
  #endif
  //get sensors values
  getDateAndTime();
  getGreenhouseTemp();
  delay(1000);
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
  //Serial.println(greenhouseTemperature.value());
  //actual time
  getDateAndTime();
  //actual temperature
  getGreenhouseTemp();
  //actual humididty
  getGreenhouseHum();
  //diplay infos on LCD screen
  lcdDisplay();
  //timepoint and target temperatures definitions, outputs routine
  greenhouse.fullRoutine(rightNowValue, &coolingTemperature, &heatingTemperature);

  #if ROLLUPS == 2 && defined(ONE_ROLLUP_AT_THE_TIME)
    if(!R2.isMoving()){
  #endif
    #if ROLLUPS >= 1 && defined(ROLLUP1_DESHUM)
      R1.routine(rollup1Deshum, coolingTemperature, greenhouseTemperature.value());
    #elif ROLLUPS >= 1 && !defined(ROLLUP1_DESHUM)
      R1.routine(coolingTemperature, greenhouseTemperature.value());
    #endif  
  #if ROLLUPS == 2 && defined(ONE_ROLLUP_AT_THE_TIME)
    }
  #endif


  #if ROLLUPS == 2 && defined(ONE_ROLLUP_AT_THE_TIME)
    if(!R1.isMoving()){
  #endif
    #if ROLLUPS == 2 && defined(ROLLUP2_DESHUM)
      R2.routine(rollup2Deshum, coolingTemperature, greenhouseTemperature.value());
    #elif ROLLUPS == 2 && !defined(ROLLUP2_DESHUM)
      R2.routine(coolingTemperature, greenhouseTemperature.value());
    #endif
  #if ROLLUPS == 2 && defined(ONE_ROLLUP_AT_THE_TIME)
    }
  #endif

  #if FANS >= 1 && defined(FAN1_DESHUM)
    F1.routine(fan1Deshum, coolingTemperature, greenhouseTemperature.value());
  #elif FANS >= 1 && !defined(FAN1_DESHUM)
    F1.routine(coolingTemperature, greenhouseTemperature.value());
  #endif
  #if FANS == 2
    F2.routine(coolingTemperature, greenhouseTemperature.value());
  #endif

  #if HEATERS >= 1 && defined(HEATER1_DESHUM)
    H1.routine(heater1Deshum, heatingTemperature, greenhouseTemperature.value());
  #elif HEATERS >= 1 && !defined(HEATER1_DESHUM)
    H1.routine(heatingTemperature, greenhouseTemperature.value());
  #endif

  #if HEATERS == 2
    H2.routine(heatingTemperature, greenhouseTemperature.value());
  #endif
  
    greenhouse.checkAlarm(greenhouseTemperature.value());
  #if defined(R1_RECALIBRATE) && ROLLUPS >= 1
    recalibrateR1();
  #endif
  #if defined(R2_RECALIBRATE) && ROLLUPS == 2
    recalibrateR2();
  #endif
  #ifdef FULL_VENTILATION
    fullVentilation();
  #endif
  #if defined(ROLLUP1_DESHUM)||defined(ROLLUP2_DESHUM)||defined(FAN1_DESHUM)||defined(FAN2_DESHUM)||defined(HEATER1_DESHUM)
    deshumCycle();
  #endif
}
//***************************************************
//*********************MACROS**************************
//***************************************************

void loadParameters(){

    if(EEPROM[0] != VERSION){
      EEPROMReset = true;
      #ifdef CLOCK_DS3231
        rtc.setTime(HR, MIN, 0);
        rtc.setDate(DAY, MONTH, YEAR);
      #endif
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
      R1.setParameters(R1_HYST, R1_ROTUP, R1_ROTDOWN, R1_PAUSE);
    #endif
    #if ROLLUPS >= 1 && STAGES >= 1
      R1.stage[0].mod.setValue(0);
      R1.stage[0].target.setValue(0);
      R1.stage[1].mod.setValue(R1_S1_MOD);
      R1.stage[1].target.setValue(R1_S1_TARGET);
    #endif
    #if ROLLUPS >= 1 && STAGES >= 2
      R1.stage[2].mod.setValue(R1_S2_MOD);
      R1.stage[2].target.setValue(R1_S2_TARGET);
    #endif
    #if ROLLUPS >= 1 && STAGES >= 3
      R1.stage[3].mod.setValue(R1_S3_MOD);
      R1.stage[3].target.setValue(R1_S3_TARGET);
    #endif
    #if ROLLUPS >= 1 && STAGES >= 4
      R1.stage[4].mod.setValue(R1_S4_MOD);
      R1.stage[4].target.setValue(R1_S4_TARGET);
    #endif

    #if ROLLUPS == 2
      R2.setParameters(R2_HYST, R2_ROTUP, R2_ROTDOWN, R2_PAUSE);
    #endif
    #if ROLLUPS == 2 && STAGES >= 1
      R2.stage[0].mod.setValue(0);
      R2.stage[0].target.setValue(0);
      R2.stage[1].mod.setValue(R2_S1_MOD);
      R2.stage[1].target.setValue(R2_S1_TARGET);
    #endif
    #if ROLLUPS == 2 && STAGES >= 2
      R2.stage[2].mod.setValue(R2_S2_MOD);
      R2.stage[2].target.setValue(R2_S2_TARGET);
    #endif
    #if ROLLUPS == 2 && STAGES >= 3
      R2.stage[3].mod.setValue(R2_S3_MOD);
      R2.stage[3].target.setValue(R2_S3_TARGET);
    #endif
    #if ROLLUPS == 2 && STAGES >= 4
      R2.stage[4].mod.setValue(R2_S4_MOD);
      R2.stage[4].target.setValue(R2_S4_TARGET);
    #endif
    #if FANS >= 1
      F1.setParameters(F1_MOD, F1_HYST);
    #endif
    #if FANS == 2
      F2.setParameters(F2_MOD, F2_HYST);
    #endif
    #if HEATERS >= 1
      H1.setParameters(H1_MOD, H1_HYST);
    #endif
    #if HEATERS == 2
      H2.setParameters(H2_MOD, H2_HYST);
    #endif
    
    #if TIMEPOINTS >= 1 && defined(ENABLE_DIF)
      T1.setParameters(DIF_TYPE, DIF_HR_MOD, DIF_MN_MOD, DIF_HEAT_SUN, DIF_COOL_SUN, DIF_HEAT_CLOUD, DIF_COOL_CLOUD, DIF_RAMP);
    #endif
    #if TIMEPOINTS >= 1 && !defined(ENABLE_DIF)
      T1.setParameters(DAY_TYPE, DAY_HR_MOD, DAY_MN_MOD, DAY_HEAT_SUN, DAY_COOL_SUN, DAY_HEAT_CLOUD, DAY_COOL_CLOUD, DAY_RAMP);
    #endif
    
    #if TIMEPOINTS >= 2
      T2.setParameters(DAY_TYPE, DAY_HR_MOD, DAY_MN_MOD, DAY_HEAT_SUN, DAY_COOL_SUN, DAY_HEAT_CLOUD, DAY_COOL_CLOUD, DAY_RAMP);
    #endif
    
    #if TIMEPOINTS >= 3 && defined(ENABLE_PRENIGHT)
      T3.setParameters(PREN_TYPE, PREN_HR_MOD, PREN_MN_MOD, PREN_HEAT_SUN, PREN_COOL_SUN, PREN_HEAT_CLOUD, PREN_COOL_CLOUD, PREN_RAMP);
    #endif
    #if TIMEPOINTS >= 1 && !defined(ENABLE_PRENIGHT)
      T3.setParameters(NIGHT_TYPE, NIGHT_HR_MOD, NIGHT_MN_MOD, NIGHT_HEAT_SUN, NIGHT_COOL_SUN, NIGHT_HEAT_CLOUD, NIGHT_COOL_CLOUD, NIGHT_RAMP);
    #endif

    
    #if TIMEPOINTS >= 4
      T4.setParameters(NIGHT_TYPE, NIGHT_HR_MOD, NIGHT_MN_MOD, NIGHT_HEAT_SUN, NIGHT_COOL_SUN, NIGHT_HEAT_CLOUD, NIGHT_COOL_CLOUD, NIGHT_RAMP);
    #endif


    }

    else{
      #if ROLLUPS >= 1
        R1.EEPROMGet();
      #endif
      #if ROLLUPS == 2
        R2.EEPROMGet();
      #endif
      #if FANS >=1
        F1.EEPROMGet();
      #endif
      #if FANS == 2
        F2.EEPROMGet();
      #endif
      #if HEATERS >=1
        H1.EEPROMGet();
      #endif
      #if HEATERS ==2
        H2.EEPROMGet();
      #endif

      #if TIMEPOINTS >= 1
      T1.EEPROMGet();
      #endif
      #if TIMEPOINTS >= 2
        T2.EEPROMGet();
      #endif
      #if TIMEPOINTS >= 3
        T3.EEPROMGet();
      #endif
      #if TIMEPOINTS >= 4
        T4.EEPROMGet();
      #endif

    }

    EEPROM[0] = VERSION;
    EEPROMReset = false;
}
