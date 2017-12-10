/*
  Greenhouse_prototype_CAPE_v.1.1.ino
  Copyright (C)2017 Loup Hébert-Chartrand. All right reserved  
  
  This code has been made to interface with Arduino-like microcontrollers,
  for inclusion in greenhouse automation devices.
  
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

/*******************************************************
/*******************CONTROL PARAMETERS******************
/*******************************************************

/********************Timepoints parameters***************

Timepoints define the greenhouse normal temperature range over time. To set a timepoint, four parameters are asked:
- type (timpoint can be set relatively to sunrise or sunset, or set manually)
- time (if set relatively to sunrise or sunset, the time parameter is a value between -60 minuts and +60 minuts,.
        if set manually, the time parameter is two values : an hour value and a minut value)
- heating temperature (reference temperature for heating devices)
- cooling temperature (reference temperature for cooling devices(rollups included)

SYNTAX RULES:
- type variable(TYPE) : SR, CLOCK and SS (sunrise, manual, sunset)
- hour variable(HOUR) : 0 for SR/SS types 
                  0-24 for CLOCK type
- min variable(MN_MOD) :  -60 to 60 for SR/SS types
                  0 to 60 for CLOCK type
- heating temperature variable(HEAT) : 0-50
- cooling temperature variable(COOL) : 0-50

EXAMPLE 1 (timepoint relative to sunrise): 

#define TP1_TYPE        SR
#define TP1_HOUR        0
#define TP1_MN_MOD      -30
#define TP1_HEAT        18
#define TP1_COOL        20  
Timepoint occur 30 minuts before sunrise, heating temperature reference is set to 18C, cooling temperature reference is set to 20C.

EXAMPLE 2 (timepoint relative to sunrise): 

#define TP1_TYPE        CLOCK
#define TP1_HOUR        12
#define TP1_MN_MOD      30
#define TP1_HEAT        20
#define TP1_COOL        25  
Timepoint occur at 12h30, heating temperature reference is set to 20C, cooling temperature reference is set to 25C.

/********************Rollups parameters***************
 
Rollup parameters set the general behaviour of the roll-up motors, according to measured temperature and cooling reference temperature
A rollup program splits in two parts : global parameters and stages parameters 
- Global parameters are active at all time
- Stages parameters are only active within a short temperature range, defined as "stage" or "cool stage". 
  They set the target increment (%) within this temperature range

For global parameters, four parameters are asked:
- hysteresis (tolerated temperature drop before closing)
- rotation up (# of sec before rollup reaches the top)
- rotation down (# of sec before rollup reaches the bottom)
- pause time(pause (in sec) between each motor move)

SYNTAX RULES :
- hysteresis (HYST): 0 to 5
- rotation up(ROTUP): 0 to 300
- rotation down (ROTDOWN): 0 to 300
- pause time(PAUSE): 0 to 240

For each stages parameters (there is usally many stages), two parameters are asked :
- temperature modificator (Adds to cooling reference temperature, defines at which temperature the "cool stage" starts)
- target increments (while in this stage, rollup will move to reach this increment, in % of opening.

SYNTAX RULES :
- temperature mod(MOD) : -5 to 10
- target increment(TARGET): 0 to 100

EXAMPLE :
#define R1_HYST         1
#define R1_ROTUP        189
#define R1_ROTDOWN      150
#define R1_PAUSE        30

#define R1_S1_MOD       0 
#define R1_S1_TARGET    25
#define R1_S2_MOD       1
#define R1_S2_TARGET    50
#define R1_S3_MOD       2
#define R1_S3_TARGET    75
#define R1_S4_MOD       3
#define R1_S4_TARGET    100

Total rotation time is 189 seconds, total closing time is 150 seconds.
At cooling temperature + 0C, rollup will open at 25%. At cooling temperature +0(mod) -1(hysteresis), it will close back to 0%.
At cooling temperature + 1C, rollup will open at 50%. At cooling temperature +1(mod) -1(hysteresis), it will close back to 25%(last stage target target increment).
At cooling temperature + 2C, rollup will open at 75%. At cooling temperature +2(mod) -1(hysteresis), it will close back to 50%(last stage target target increment).
At cooling temperature + 3C, rollup will open at 100%. At cooling temperature +3(mod) -1(hysteresis), it will close back to 75%(last stage target target increment).

If cooling temperature is 24C : 
At 24C, rollup will open at 25%. At 23C, it will close back to 0%.
At 25C, rollup will open at 50%. At 24C, it will close back to 25%.
At 26C, rollup will open at 75%. At 25C, it will close back to 50%.
At 27C, rollup will open at 100%. At 25C, it will close back to 75%.
*/

/********************Fans/heaters parameters***************

Fan parameters set the general behaviour of ON/OFF cooling devices (typically, fans), according to measured temperature and cooling reference temperature
Two parameters are asked :
- hysteresis (tolerated temperature drop before shutting)
- temperature modificator (Adds to cooling reference temperature, defines at which temperature it starts)

SYNTAX RULES:
- hysteresis : 0 to 5
- temperature modificator : -5 to 10

Example : 
#define F1_MOD          3
#define F1_HYST         1
At cooling reference +3, fan will starts.
At cooling reference temperature +3 (mod) -1 (hyst), fan will stops.

If cooling reference temperature is 24C :
At 27C, fan will starts.
At 26C, fan will stops.
 
Heater parameters set the general behaviour of ON/OFF heating devices (typically, heaters), according to measured temperature and heating reference temperature
- hysteresis (tolerated temperature rise before shutting)
- temperature modificator (Substract to heating reference temperature, defines at which temperature it starts)

SYNTAX RULES:
- hysteresis : 0 to 5
- temperature modificator : -10 to 5

EXAMPLE :
Example : 
#define H1_MOD          -1
#define H1_HYST         2
At heating reference -1, furnace will start.
At heating reference temperature +-1 (mod) +1 (hyst), furnace will stop.

If cooling reference temperature is 18C :
At 17C, furnace will start.
At 19C, furnace will stop.
*/ 

//************************************************************
//*******************CONTROL PARAMETERS***********************
//************************************************************
 
//********************Geographic/time parameters***************
#define TIMEZONE      -5
#define LATITUDE      45.50
#define LONGITUDE    -73.56

//#define RTC_TIME_SET
#define HOUR_SET       21
#define MINUT_SET      6
#define SECOND_SET     30

//#define RTC_DATE_SET
#define DAY_SET        8
#define MONTH_SET      12
#define YEAR_SET       2017
//*******************************************************Timepoint 1
#define TP1_TYPE        SR
#define TP1_HOUR        0
#define TP1_MN_MOD      -30
#define TP1_HEAT        18
#define TP1_COOL        20   
//*******************************************************Timepoint 2
#define TP2_TYPE        SR
#define TP2_HOUR        0
#define TP2_MN_MOD      0
#define TP2_HEAT        18
#define TP2_COOL        22
//*******************************************************Timepoint 3
#define TP3_TYPE        CLOCK
#define TP3_HOUR        12
#define TP3_MN_MOD      30
#define TP3_HEAT        20
#define TP3_COOL        24
//*******************************************************Timepoint 4
#define TP4_TYPE        SS
#define TP4_HOUR        0
#define TP4_MN_MOD      -60
#define TP4_HEAT        16
#define TP4_COOL        18
//*******************************************************Timepoint 5
#define TP5_TYPE        SS
#define TP5_HOUR        0
#define TP5_MN_MOD      0
#define TP5_HEAT        17
#define TP5_COOL        19
//*******************************************************************
//*******************************************************Rollup 1 (overral parameters)
#define R1_HYST         1
#define R1_ROTUP        189
#define R1_ROTDOWN      150
#define R1_PAUSE        30
//*******************************************************Rollup 1 (stages)
#define R1_S1_MOD       0 
#define R1_S1_TARGET    25
#define R1_S2_MOD       1
#define R1_S2_TARGET    50
#define R1_S3_MOD       2
#define R1_S3_TARGET    75
#define R1_S4_MOD       3
#define R1_S4_TARGET    100
//*******************************************************Rollup 2 (overral parameters)
#define R2_HYST         1
#define R2_ROTUP        189
#define R2_ROTDOWN      150
#define R2_PAUSE        30
//*******************************************************Rollup 2 (stages)
#define R2_S1_MOD       0
#define R2_S1_TARGET    10
#define R2_S2_MOD       1
#define R2_S2_TARGET    30
#define R2_S3_MOD       2
#define R2_S3_TARGET    50
#define R2_S4_MOD       3
#define R2_S4_TARGET    70
//*************************************************************************
//*******************************************************Fan parameters
#define F1_MOD          3
#define F1_HYST         1
//**********************************************************************
//*******************************************************Heater parameters
#define H1_MOD          -1
#define H1_HYST         2
//************************************************************************

//***************************************************************************
//***************************************************************************
//***************************************************************************
/* 
 *                                WARNING!!!
 *  
 * CHANGING ANYTHING BELOW THIS LINE COULD HAVE WEIRD EFFECT ON YOUR 
 * CONTROLLER'S BEHAVIOUR. 
 * SHOULDN'T STOP YOU FROM HAVING A LOOK THO! IT'S WHERE THE REAL
 * MAGIC ACTUALLY TAKE PLACE!!
 * 
*/
//***************************************************************************
//***************************************************************************
//***************************************************************************

#include "Arduino.h"

//********************LIBRARIES**************************
#include "GreenhouseLib.h"
#include "Keypad.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DS3231.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>


//********************PINOUT**************************
#define WEST_OPENING_PIN 4 //connect this pin to the opening relay (west motor)
#define WEST_CLOSING_PIN 5 //connect this pin to the closing relay (west motor)
#define EAST_OPENING_PIN 6 //connect this pin to the opening relay (east motor)
#define EAST_CLOSING_PIN 7 //connect this pin to the closing relay (east motor)
#define FAN_PIN          2 //Connect this pin to the fan relay
#define HEATER_PIN       3 //connect this pin to the heater relay
#define ONE_WIRE_BUS     A1 //connect this pin to the DS18B20 data line



//********************OBJECTS**************************
// Create lcd object using NewLiquidCrystal lib
#define I2C_ADDR    0x27              // Define I2C Address where the PCF8574A is
#define BACKLIGHT_PIN     3
LiquidCrystal_I2C  lcd(I2C_ADDR, 2, 1, 0, 4, 5, 6, 7);

//Create DS18B20 object
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//Create a RTC object
DS3231  rtc(SDA, SCL);                // Init the DS3231 using the hardware interface
Time  t;

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

byte rightNow[6];
float greenhouseTemperature;

//***************************************************
//********************SETUP**************************
//***************************************************

void setup() {
  //start communication with serial monitor
  Serial.begin(9600);
  //start communication with temp probe
  initLCD(20,4);
  //start communication with temp probe
  sensors.begin();
  //start communication with temp probe
  rtc.begin();
  
  // change RTC settings
  #ifdef RTC_TIME_SET
    rtc.setTime(HOUR_SET, MINUT_SET, SECOND_SET);
  #endif
  #ifdef RTC_DATE_SET
    rtc.setDate(DAY_SET, MONTH_SET, YEAR_SET);
  #endif
  //get RTC values
  getDateAndTime();                             //get RTC values
  //set time within greenhouse object
  greenhouse.setNow(rightNow);
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
  rollup1.setStageMod(1,R1_S1_MOD);
  rollup1.setStageTarget(1,R1_S1_TARGET);
  rollup1.setStageMod(2,R1_S2_MOD);
  rollup1.setStageTarget(2,R1_S2_TARGET);
  rollup1.setStageMod(3,R1_S3_MOD);
  rollup1.setStageTarget(3,R1_S3_TARGET);
  rollup1.setStageMod(4,R1_S4_MOD);
  rollup1.setStageTarget(4,R1_S4_TARGET);
  //********************Rollup 2 (west)*******************
  rollup2.initOutputs(MAN_TEMP, ACT_HIGH, WEST_OPENING_PIN, WEST_CLOSING_PIN); 
  rollup2.setParameters(0, R2_HYST, R2_ROTUP, R2_ROTDOWN, 100, R2_PAUSE, true); 
  rollup2.setStageMod(1,R2_S1_MOD);
  rollup2.setStageTarget(1,R2_S1_TARGET);
  rollup2.setStageMod(2,R2_S2_MOD);
  rollup2.setStageTarget(2,R2_S2_TARGET);
  rollup2.setStageMod(3,R2_S3_MOD);
  rollup2.setStageTarget(3,R2_S3_TARGET);
  rollup2.setStageMod(4,R2_S4_MOD);
  rollup2.setStageTarget(4,R2_S4_TARGET);
  //*************************Fan**************************
  fan1.initOutput(VAR_TEMP, FAN_PIN);
  fan1.setParameters(F1_MOD, F1_HYST, true);
  //************************Heater************************    
  heater1.initOutput(VAR_TEMP, HEATER_PIN);
  heater1.setParameters(H1_MOD, H1_HYST, true);
  //******************************************************

  //actual time, timepoint and targetTemp
  greenhouse.startingParameters();  ;
  
}


//***************************************************
//*********************LOOP**************************
//***************************************************

void loop() {
  //actual time
  getDateAndTime();
  //actual temperature
  greenhouseTemperature = getGreenhouseTemp();
  //diplay infos on LCD screen
  lcdDisplay();
  //timepoint and target temperatures definitions, outputs routine
  greenhouse.fullRoutine(rightNow, greenhouseTemperature);
}


//***************************************************
//*********************MACROS**************************
//***************************************************


void getDateAndTime(){
  t = rtc.getTime();
  rightNow[5] = t.year-2000;
  rightNow[4] = t.mon;
  rightNow[3] = t.date;
  rightNow[HOUR] = t.hour;
  rightNow[MINUT] = t.min;
  rightNow[0] = t.sec;
  
  #ifdef DEBUG_CLOCK
  for(int x = 0; x < sizeof(rightNow); x++){
    Serial.print(rightNow[x]);
    Serial.print(":");
  }
  Serial.println("");
  #endif
}


float getGreenhouseTemp(){
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);
    if((temp < -20.00)||(temp > 80)){
      temp = greenhouse._coolingTemp+10;
      return temp;
    }
    return temp;
}

byte onebit[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10000,
};
byte twobit[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10100,
};byte threebit[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10101,
};
byte bitCount1 = 0;
byte bitCount2 = 0;

void initLCD(byte length, byte width){
  lcd.begin(length, width);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.createChar(0, onebit);
  lcd.createChar(1, twobit);
  lcd.createChar(2, threebit);
  lcd.clear();
}


void lcdDisplay(){
  lcdPrintTemp();
  lcdPrintTempCible();
  lcdPrintRollups();
  lcdPrintOutputs();
  lcdPrintTime();
}

void lcdPrintOutputs(){
    //Fan
    lcd.setCursor(0, 3); lcd.print(F("          "));
    lcd.setCursor(0, 3); lcd.print(F("FAN: "));
    if(digitalRead(FAN_PIN) == HIGH){
      lcd.print("ON");
    }
    else{
      lcd.print("OFF");
    }
    //Heater
    lcd.setCursor(9, 3); lcd.print(F("          "));
    lcd.setCursor(9, 3); lcd.print(F("|HEAT: "));
    if(digitalRead(HEATER_PIN) == HIGH){
      lcd.print("ON");
    }
    else{
      lcd.print("OFF");
    }
}
void lcdPrintRollups(){
    //East rollup
    lcd.setCursor(0, 2); lcd.print(F("          "));
    lcd.setCursor(0, 2); lcd.print(F("EAST:"));
    if(rollup1.incrementCounter() == OFF_VAL){  
      lcd.setCursor(0, 2);lcd.print(F(""));
    }
    else if (rollup1.opening() == true){
      lcd.setCursor(0, 2); lcd.print("OPENING  ");
    }
    else if (rollup1.closing() == true){
      lcd.setCursor(0, 2); lcd.print("CLOSING  ");
    }
    else{
      lcd.print(rollup1.incrementCounter());lcd.print("%");
    }
    if(((rollup1.closing() == false)&&(rollup1.closingCycle() == true))||((rollup1.opening() == false)&&(rollup1.openingCycle() == true))){
      if(bitCount1 == 0){
        lcd.write(byte(0));
        bitCount1 ++;
      }
      else if(bitCount1 == 1){
        lcd.write(byte(1));
        bitCount1 ++;
      }
      else if(bitCount1 == 2){
        lcd.write(byte(2));
        bitCount1 = 0;
      }
    }
    //West rollup
    lcd.setCursor(9, 2); lcd.print(F("          "));
      lcd.setCursor(9, 2); lcd.print(F("|WEST:"));
    if(rollup2.incrementCounter() == OFF_VAL){  
      lcd.print(F(""));
    }
    else if (rollup2.opening() == true){
      lcd.setCursor(9, 2); lcd.print("|OPENING   ");
    }
    else if (rollup2.closing() == true){
      lcd.setCursor(9, 2); lcd.print("|CLOSING   ");
    }
    else{
      lcd.print(rollup2.incrementCounter());lcd.print("%");
    }
    if(((rollup2.closing() == false)&&(rollup2.closingCycle() == true))||((rollup2.opening() == false)&&(rollup2.openingCycle() == true))){
      if(bitCount2 == 0){
        lcd.write(byte(0));
        bitCount2 ++;
      }
      else if(bitCount2 == 1){
        lcd.write(byte(1));
        bitCount2 ++;
      }
      else if(bitCount2 == 2){
        lcd.write(byte(2));
        bitCount2 = 0;
      }
    }
}
void lcdPrintTemp(){
    float temperature = greenhouseTemperature;
    lcd.setCursor(0,0); lcd.print(F("         "));
    lcd.setCursor(0,0); lcd.print(temperature); lcd.print(F("C"));
}

void lcdPrintTempCible(){
      lcd.setCursor(9,0);lcd.print("|TT:");lcd.print((short)(greenhouse._heatingTemp + rollup1.tempParameter())); lcd.print(F("-"));lcd.print((short)(greenhouse._coolingTemp + rollup1.tempParameter())); lcd.print(F("C"));
}
void lcdPrintTime(){
    lcd.setCursor(0,1); lcdPrintDigits(rightNow[HOUR]); lcd.print(F(":")); lcdPrintDigits(rightNow[MINUT]);
    lcd.setCursor(9,1); lcd.print(F("|Timep. ")), lcd.print(greenhouse._timepoint);
}

void lcdPrintDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
  lcd.print("0");
  lcd.print(digits);
}

void serialPrintDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
  Serial.print("0");
}
