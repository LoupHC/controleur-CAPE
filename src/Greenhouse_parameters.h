/*
  Greenhouse_prototype_CAPE_v.1.1.ino
  Copyright (C)2017 Loup Hébert-Chartrand. All right reserved

  This code has been made to interface with Arduino-like microcontrollers,
  for inclusion in greenhouse automation devices.

  Supported devices :
  - DS18B20 temperature sensor
  - DS3231 RTC module
  - 20x4 LCD Display with I2C backpack

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
/*
*******************************************************
*******************CONTROL PARAMETERS******************
*******************************************************

********************Timepoints parameters***************

Timepoints define the greenhouse normal temperature range over time. To set a timepoint, four parameters are asked:
- type (timpoint can be set relatively to sunrise or sunset, or set manually)
- time (if set relatively to sunrise or sunset, the time parameter is a value between -60 minuts and +60 minuts,.
        if set manually, the time parameter is two values : an hour value and a minut value)
- heating temperature (reference temperature for heating devices)
- cooling temperature (reference temperature for cooling devices(rollups included)
- ramping time (in timepoints transitions, how many time (in minuts) before increasing/decreasing cooling/heating temperature of 0.5C)

EXAMPLE 1 (timepoint relative to sunrise):

#define TP1_TYPE        SR
#define TP1_HOUR        0
#define TP1_MN_MOD      -30
#define TP1_HEAT        18
#define TP1_COOL        20
#define TP1_RAMP        15
Timepoint occur 30 minuts before sunrise, heating temperature reference is set to 18C, cooling temperature reference is set to 20C.

EXAMPLE 2 (timepoint relative to sunrise):

#define TP1_TYPE        CLOCK
#define TP1_HOUR        12
#define TP1_MN_MOD      30
#define TP1_HEAT        20
#define TP1_COOL        25
#define TP1_RAMP        15
Timepoint occur at 12h30, heating temperature reference is set to 20C, cooling temperature reference is set to 25C.

********************Rollups parameters***************

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

For each stages parameters (there is usally many stages), two parameters are asked :
- temperature modificator (Adds to cooling reference temperature, defines at which temperature the "cool stage" starts)
- target increments (while in this stage, rollup will move to reach this increment, in % of opening.

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

Total opening time is 189 seconds, total closing time is 150 seconds. Pause between motor movements is 30 seconds.
Stage 1: At cooling temperature + 0C, rollup will open at 25%. At cooling temperature +0(mod) -1(hysteresis), it will close back to 0%.
Stage 2: At cooling temperature + 1C, rollup will open at 50%. At cooling temperature +1(mod) -1(hysteresis), it will close back to 25%(last stage target target increment).
Stage 3: At cooling temperature + 2C, rollup will open at 75%. At cooling temperature +2(mod) -1(hysteresis), it will close back to 50%(last stage target target increment).
Stage 4: At cooling temperature + 3C, rollup will open at 100%. At cooling temperature +3(mod) -1(hysteresis), it will close back to 75%(last stage target target increment).

If cooling temperature is 24C :
Stage 1: At 24C, rollup will open at 25%. At 23C, it will close back to 0%.
Stage 2: At 25C, rollup will open at 50%. At 24C, it will close back to 25%.
Stage 3: At 26C, rollup will open at 75%. At 25C, it will close back to 50%.
Stage 4: At 27C, rollup will open at 100%. At 25C, it will close back to 75%.


********************Fans/heaters parameters***************

Fan parameters set the general behaviour of ON/OFF cooling devices (typically, fans), according to measured temperature and cooling reference temperature
Two parameters are asked :
- hysteresis (tolerated temperature drop before shutting)
- temperature modificator (Adds to cooling reference temperature, defines at which temperature it starts)

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

EXAMPLE :
Example :
#define H1_MOD          -1
#define H1_HYST         2
At heating reference -1, furnace will start.
At heating reference temperature +-1 (mod) +1 (hyst), furna*************

Create greenhouse object
#define TIMEPOINTS      5          //# of timepoints
#define ROLLUPS         2             //# of rollups
#define STAGES          5             //# of stagesce will stop.

If cooling reference temperature is 18C :
At 17C, furnace will start.
At 19C, furnace  stop.
*/

//#define COMPUTER_INTERFACE    //Uncomment this line if you rely only on this sheet for programming

//************************************************************
//*******************CONTROL PARAMETERS***********************
//************************************************************

//********************GREENHOUSE**************************


//Create greenhouse object
#define TIMEPOINTS            3
        //# of timepoints
#define ROLLUPS               1          //# of rollups
#define STAGES                4          //# of cool stages (for rollups)
#define FANS                  2          //# of fans
#define HEATERS               0          //# of heaters
//********************PINOUT**************************
#define ROLLUP1_OPENING_PIN   0    //connect this pin to the opening relay (west motor)
#define ROLLUP1_CLOSING_PIN   1    //connect this pin to the closing relay (west motor)
#define ROLLUP2_OPENING_PIN   OFF    //connect this pin to the opening relay (east motor)
#define ROLLUP2_CLOSING_PIN   OFF    //connect this pin to the closing relay (east motor)
#define FAN1_PIN              4  //Connect this pin to the fan relay
#define FAN2_PIN              5  //Connect this pin to the fan relay
#define HEATER1_PIN           OFF    //connect this pin to the heater relay
#define HEATER2_PIN           OFF    //connect this pin to the heater relay
#define ALARM_PIN             7    //Connect this pin to the safety buzzer

//*************COORD GÉOGRAPHIQUES*********************
#define TIMEZONE              -5
#define LATITUDE              46.03
#define LONGITUDE             -73.56

//TIME AND DATE ARE SET AT FIRST UPLOAD ONLY
//For subsequent RTC programming :
//1. Uncomment (by erasing the "//") the line #define RTC_TIME_SET and #define RTC_DATE_SET and upload to set time
//2. Put back the comment markers("//") and upload again
//ALWAYS SET TO WINTER TIME

//#define RTC_TIME_SET
#define HOUR_SET            16
#define MINUT_SET           07
#define SECOND_SET          34

//#define RTC_DATE_SET
#define DAY_SET             19
#define MONTH_SET           3
#define YEAR_SET            2018
//*******************************************************************
/*
TIMEPOINTS PARAMETERS - SYNTAX RULES:
- type variable(TYPE) :   SR, CLOCK or SS (sunrise, manual, sunset)
- hour variable(HOUR) :   SR/SS types => 0 -24 to 24
                          CLOCK type => 0 to 24
- min variable(MN_MOD) :  SR/SS types => -60 to 60
                          CLOCK type => 0 to 60
- heating temperature variable(HEAT) : 0 to 50
- cooling temperature variable(COOL) : 0 to 50
- ramping (RAMP) : 0 to
*/
//*******************************************************Timepoint 1
#define TP1_TYPE            SR
#define TP1_HOUR            0
#define TP1_MN_MOD          0
#define TP1_HEAT            16
#define TP1_COOL            20
#define TP1_HEAT_CLOUD      16
#define TP1_COOL_CLOUD      18
#define TP1_RAMP            15
//*******************************************************Timepoint 2
#define TP2_TYPE            CLOCK
#define TP2_HOUR            10
#define TP2_MN_MOD          0
#define TP2_HEAT            18
#define TP2_COOL            24
#define TP2_HEAT_CLOUD      16
#define TP2_COOL_CLOUD      18
#define TP2_RAMP            15
//*******************************************************Timepoint 3
#define TP3_TYPE            SS
#define TP3_HOUR            -1
#define TP3_MN_MOD          0
#define TP3_HEAT            18
#define TP3_COOL            22
#define TP3_HEAT_CLOUD      16
#define TP3_COOL_CLOUD      18
#define TP3_RAMP            5
//*******************************************************Timepoint 4
#define TP4_TYPE            SS
#define TP4_HOUR            0
#define TP4_MN_MOD          0
#define TP4_HEAT            18
#define TP4_COOL            22
#define TP4_HEAT_CLOUD      16
#define TP4_COOL_CLOUD      18
#define TP4_RAMP            30
//*******************************************************************
/*
ROLLUP PARAMETERS - SYNTAX RULES :
- hysteresis (HYST): 0 to 5
- rotation up(ROTUP): 0 to 300
- rotation down (ROTDOWN): 0 to 300
- pause time(PAUSE): 0 to 240
*/
//*******************************************************Rollup 1 (overral parameters)
#define R1_HYST             1
#define R1_ROTUP            25
#define R1_ROTDOWN          25
#define R1_PAUSE            5
//*******************************************************Rollup 2 (overral parameters)

#define R2_HYST             1
#define R2_ROTUP            25
#define R2_ROTDOWN          25
#define R2_PAUSE            5

//*******************************************************************
/*
ROLLUP STAGES - SYNTAX RULES :
- temperature mod(MOD) : -5 to 10
- target increment(TARGET): 0 to 100
*/
//*******************************************************Rollup 1 (stages)
#define R1_S0_MOD           0
#define R1_S0_TARGET        0
#define R1_S1_MOD           0
#define R1_S1_TARGET        25
#define R1_S2_MOD           1
#define R1_S2_TARGET        50
#define R1_S3_MOD           2
#define R1_S3_TARGET        75
#define R1_S4_MOD           3
#define R1_S4_TARGET        100
//*******************************************************Rollup 2 (stages)

#define R2_S0_MOD           0
#define R2_S0_TARGET        0
#define R2_S1_MOD           1
#define R2_S1_TARGET        10
#define R2_S2_MOD           2
#define R2_S2_TARGET        30
#define R2_S3_MOD           3
#define R2_S3_TARGET        50
#define R2_S4_MOD           4
#define R2_S4_TARGET        70

//*************************************************************************
/*
FAN PARAMETERS - SYNTAX RULES:
- hysteresis (HYST): 0 to 5
- temperature modificator (MOD): -5 to 10
*/
//*******************************************************Fan parameters

#define F1_HYST             1
#define F1_MOD              1
#define F2_HYST             1
#define F2_MOD              3

//**********************************************************************
/*
HEATER PARAMETERS - SYNTAX RULES:
- hysteresis : 0 to 5
- temperature modificator : -10 to 5
*/
//*******************************************************Heater parameters

#define H1_HYST             2
#define H1_MOD              -1
#define H2_HYST             2
#define H2_MOD              -1

//************************************************************************
/*ALARM
  Fire alarm (ALARM_PIN) when temperature get over a maximum or under a minimum
*/
#define ALARM_MIN_TEMP    10.00
#define ALARM_MAX_TEMP    30.00


/*ROLLUP CALIBRATION - FIX OVERRIDE

Conditions :
 - rollup has been fully open or fully close for a while (SAFETY_DELAY(min))
 - rollup is not moving
Action :
 - full opening or full closing cycle
*/

#define R1_RECALIBRATE
#define R2_RECALIBRATE
#define SAFETY_DELAY                  30


/*FULL VENTILATION - FIX OVERRIDE

Conditions :
 -Full ventiliation action is called
Action :
 - open rollups and start all fans for a while (FULL_VENTILATION_DELAY(min))
*/

#define FULL_VENTILATION
#define FULL_VENTILATION_DELAY        5


/*DESHUM CYCLE - RELATIVE OVERRIDE

Conditions :
  - deshum cycle has been activated
  -between start time and stop time
  -temperature stays over a minimum
Action :
  - open rollups at target increment
  AND/OR
  - activate heater
  AND/OR
  - activate fan(s)
*/

//#define HEATER1_DESHUM
//#define HEATER1_DESHUM_START_HOUR      6
//#define HEATER1_DESHUM_START_MIN       5
//#define HEATER1_DESHUM_STOP_HOUR       6
//#define HEATER1_DESHUM_STOP_MIN        15

//#define FAN1_DESHUM
//#define FAN1_DESHUM_START_HOUR         6
//#define FAN1_DESHUM_START_MIN          5
//#define FAN1_DESHUM_STOP_HOUR          6
//#define FAN1_DESHUM_STOP_MIN           20

//#define ROLLUP1_DESHUM
//#define ROLLUP1_DESHUM_INCREMENT       50
//#define ROLLUP1_DESHUM_START_HOUR      6
//#define ROLLUP1_DESHUM_START_MIN       5
//#define ROLLUP1_DESHUM_STOP_HOUR       6
//#define ROLLUP1_DESHUM_STOP_MIN        20

//#define ROLLUP2_DESHUM
//#define ROLLUP2_DESHUM_INCREMENT       50
//#define ROLLUP2_DESHUM_START_HOUR      6
//#define ROLLUP2_DESHUM_START_MIN       5
//#define ROLLUP2_DESHUM_STOP_HOUR       6
//#define ROLLUP2_DESHUM_STOP_MIN        20

//#define DESHUM_MININIM                 14
