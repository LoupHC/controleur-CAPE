/*
  Greenhouse_prototype_CAPE_v.1.1.ino
  Copyright (C)2017 Loup Hébert-Chartrand. All right reserved  
  
  This code has been made to interface with Arduino-like microcontrollers,
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
 * HAVE A LOOK THO! IT'S WHERE THE REAL MAGIC ACTUALLY TAKE PLACE!!
 * IF YOU MAKE ANY IMPROVEMENTS TO THIS CODE THAT COULD BENEFIT OTHERS, AND 
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

//********************PINOUT**************************
#define WEST_OPENING_PIN 7 //connect this pin to the opening relay (west motor)
#define WEST_CLOSING_PIN 4 //connect this pin to the closing relay (west motor)
#define EAST_OPENING_PIN 6 //connect this pin to the opening relay (east motor)
#define EAST_CLOSING_PIN 5 //connect this pin to the closing relay (east motor)
#define FAN_PIN          8 //Connect this pin to the fan relay
#define HEATER_PIN       9 //connect this pin to the heater relay
#define ONE_WIRE_BUS     A1 //connect this pin to the DS18B20 data line
#define DHT_PIN          2

//********************GREENHOUSE**************************

//Create greenhouse object
const byte TIMEPOINTS = 5;          //# of timepoints 
const byte ROLLUPS = 2;             //# of rollups
const byte STAGES = 4;          //# of timepoints 
const byte FANS = 1;                //# of fans
const byte HEATERS = 1;             //# of heaters

Greenhouse greenhouse(TIMEZONE, LATITUDE, LONGITUDE, TIMEPOINTS, ROLLUPS, STAGES, FANS, HEATERS);

//********************POINTERS**************************

Timepoint &timepoint1 = greenhouse.timepoint[0];
Timepoint &timepoint2 = greenhouse.timepoint[1];
Timepoint &timepoint3 = greenhouse.timepoint[2];
Timepoint &timepoint4 = greenhouse.timepoint[3];
Timepoint &timepoint5 = greenhouse.timepoint[4];
Rollup &rollup1 = greenhouse.rollup[0];
Rollup &rollup2 = greenhouse.rollup[1];
Fan &fan1 = greenhouse.fan[0];
Heater &heater1 = greenhouse.heater[0];

//********************VARIABLES**************************

//Time array
byteParameter rightNow[6];
byte rightNowValue[6];
//Temperature inside the greenhouse
floatParameter greenhouseTemperature;
floatParameter greenhouseHumidity;

boolean sensorFailure = false;
//********************INTERFACE**************************

//See "Greenhouse_interface.h" for LCD display functions
#include "Greenhouse_interface.h"


//********************SENSORS**************************

//See "Greenhouse_interface.h" for sensor functions
#include "Greenhouse_sensors.h"



//***************************************************
//********************SETUP**************************
//***************************************************

void setup() {
  //start communication with serial monitor
  Serial.begin(115200);
  //start communication with interface
  initLCD(20,4);

  #ifdef KEYPAD_MENU
    keypad.begin( makeKeymap(keys) );
  #endif

  //start communication with temp probe
  sensors.begin();
  sensors.setResolution(12);
  dht.begin();

  //start communication with clock
  rtc.begin();
  
  #ifdef I2C_OUTPUTS
    //start communication with relay driver
    mcp.begin();
  #endif
  
  // change RTC settings
  #ifdef RTC_TIME_SET
    rtc.setTime(HOUR_SET, MINUT_SET, SECOND_SET);
  #endif
  #ifdef RTC_DATE_SET
    rtc.setDate(DAY_SET, MONTH_SET, YEAR_SET);
  #endif
  //get RTC values
  getDateAndTime();
  //set time within greenhouse object
  greenhouse.setNow(rightNowValue);
  //calculate hour saving, sunrise, sunset
  greenhouse.solarCalculations();
  
//********************Parameters*******************
  //********************Timepoint*******************
  timepoint1.setParameters(TP1_TYPE, TP1_HOUR, TP1_MN_MOD, TP1_HEAT, TP1_COOL);
  timepoint2.setParameters(TP2_TYPE, TP2_HOUR, TP2_MN_MOD, TP2_HEAT, TP2_COOL);
  timepoint3.setParameters(TP3_TYPE, TP3_HOUR, TP3_MN_MOD, TP3_HEAT, TP3_COOL);
  timepoint4.setParameters(TP4_TYPE, TP4_HOUR, TP4_MN_MOD, TP4_HEAT, TP4_COOL);
  timepoint5.setParameters(TP5_TYPE, TP5_HOUR, TP5_MN_MOD, TP5_HEAT, TP5_COOL);
  //********************Rollup 1 (east)*******************
  rollup1.initOutputs(MAN_TEMP, ACT_HIGH, EAST_OPENING_PIN, EAST_CLOSING_PIN);
  rollup1.setParameters(0, R1_HYST, R1_ROTUP, R1_ROTDOWN, 100, R1_PAUSE, true);
  rollup1.setStageParameters(1,R1_S1_MOD, R1_S1_TARGET);
  rollup1.setStageParameters(2,R1_S2_MOD, R1_S2_TARGET);
  rollup1.setStageParameters(3,R1_S3_MOD, R1_S3_TARGET);
  rollup1.setStageParameters(4,R1_S4_MOD, R1_S4_TARGET);
  //********************Rollup 2 (west)*******************
  rollup2.initOutputs(MAN_TEMP, ACT_HIGH, WEST_OPENING_PIN, WEST_CLOSING_PIN); 
  rollup2.setParameters(0, R2_HYST, R2_ROTUP, R2_ROTDOWN, 100, R2_PAUSE, true); 
  rollup2.setStageParameters(1,R2_S1_MOD, R2_S1_TARGET);
  rollup2.setStageParameters(2,R2_S2_MOD, R2_S2_TARGET);
  rollup2.setStageParameters(3,R2_S3_MOD, R2_S3_TARGET);
  rollup2.setStageParameters(4,R2_S4_MOD, R2_S4_TARGET);
  //*************************Fan**************************
  fan1.initOutput(VAR_TEMP, ACT_HIGH, FAN_PIN);
  fan1.setParameters(F1_MOD, F1_HYST, true);
  //************************Heater************************    
  heater1.initOutput(VAR_TEMP, ACT_HIGH, HEATER_PIN);
  heater1.setParameters(H1_MOD, H1_HYST, true);
  //******************************************************

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
  //diplay infos on LCD screen
  lcdDisplay();
  //timepoint and target temperatures definitions, outputs routine
  greenhouse.fullRoutine(rightNowValue, greenhouseTemperature.value());
}


//***************************************************
//*********************MACROS**************************
//***************************************************




