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

/*ROLLUP CALIBRATION - FIX OVERRIDE

Conditions :
 - rollup has been fully open or fully close for a while (SAFETY_DELAY)
 - rollup is not moving
Action :
 - full opening or full closing cycle
*/
boolean deshum = false;
#ifdef ROLLUP1_DESHUM
  boolean rollup1Deshum = false;
#endif
#ifdef ROLLUP2_DESHUM
  boolean rollup2Deshum = false;
#endif
#ifdef FAN1_DESHUM
  boolean fan1Deshum = false;
#endif
#ifdef FAN2_DESHUM
  boolean fan2Deshum = false;
#endif
#ifdef HEATER1_DESHUM
  boolean heater1Deshum = false;
#endif
//Override : rollup calibration
#ifdef R1_RECALIBRATE
  boolean safetyCycle1 = false;
  elapsedMillis safetyTimer1;
#endif
#ifdef R2_RECALIBRATE
  boolean safetyCycle2 = false;
  elapsedMillis safetyTimer2;
#endif
//Override : full ventilation
  boolean fullVentOverride = false;

#if defined(R1_RECALIBRATE) &&  ROLLUPS >= 1
void recalibrateR1(){
    if((R1.incrementCounter() == R1.increments())&&(R1.isMoving() == false)){
      if(safetyCycle1 == false){
        safetyCycle1 = true;
        safetyTimer1 = 0;
      }
      if(safetyTimer1 > (unsigned long)SAFETY_DELAY*60000){
        R1.setIncrementCounter(0);
        R1.forceMove(R1.rotationUp.value(), R1.increments());
        safetyCycle1 = false;
      }
    }
    else if((R1.incrementCounter() == 0)&&(R1.isMoving() == false)){
      if(safetyCycle1 == false){
        safetyCycle1 = true;
        safetyTimer1 = 0;
      }
      if(safetyTimer1 > (unsigned long)SAFETY_DELAY*60000){
        R1.setIncrementCounter(R1.increments());
        R1.forceMove(R1.rotationDown.value(), 0);
        safetyCycle1 = false;
      }
    }
}
#endif
#if defined(R2_RECALIBRATE) &&  ROLLUPS == 2
void recalibrateR2(){
    if((R2.incrementCounter() == R2.increments())&&(R2.isMoving() == false)){
      if(safetyCycle2 == false){
        safetyCycle2 = true;
        safetyTimer2 = 0;
      }
      if(safetyTimer2 > (unsigned long)SAFETY_DELAY*60000){
        R2.setIncrementCounter(0);
        R2.forceMove(R2.rotationUp.value(), R2.increments());
        safetyCycle2 = false;
      }
    }
    else if((R2.incrementCounter() == 0)&&(R2.isMoving() == false)){
      if(safetyCycle2 == false){
        safetyCycle2 = true;
        safetyTimer2 = 0;
      }
      if(safetyTimer2 > (unsigned long)SAFETY_DELAY*60000){
        R2.setIncrementCounter(R2.increments());
        R2.forceMove(R2.rotationDown.value(), 0);
        safetyCycle2 = false;
      }
    }
  }
  #endif

/*FULL VENTILATION - FIX OVERRIDE

Conditions :
 -fullVentiliation variable is true
Action :
 - open rollups and start all fans for a while (FULL_VENTILATION_DELAY)
*/
void fullVentilation(){
  if(fullVentOverride == true){
      unsigned int timing = FULL_VENTILATION_DELAY*60;
    #if defined(FULL_VENTILATION) && ROLLUPS >= 1
      R1.forceMove(timing, R1.increments());
    #endif
    #if defined(FULL_VENTILATION) &&  ROLLUPS == 2
      R2.forceMove(timing, R2.increments());
    #endif
    #if defined(FULL_VENTILATION) &&  FANS >= 1
      F1.forceAction(timing, true);
    #endif
    #if defined(FULL_VENTILATION) &&  FANS == 2
      F2.forceAction(timing, true);
    #endif
    fullVentOverride = false;
  }
}


/*DESHUM CYCLE - RELATIVE OVERRIDE

Conditions :
  -between start time and stop time
  -temperature stays over a minimum
Action :
  - open rollups at target increment
  AND/OR
  - activate heater
  AND/OR
  - activate fan(s)
*/
void deshumCycle(){
  #if defined(ROLLUP1_DESHUM) && ROLLUPS >= 1
    if((((greenhouse.rightNow(2) == ROLLUP1_DESHUM_START_HOUR)&&(greenhouse.rightNow(1) >= ROLLUP1_DESHUM_START_MIN))||(greenhouse.rightNow(2)>ROLLUP1_DESHUM_START_HOUR))&&(((greenhouse.rightNow(2) == ROLLUP1_DESHUM_STOP_HOUR)&&(greenhouse.rightNow(1) < ROLLUP1_DESHUM_STOP_MIN))||(greenhouse.rightNow(2) < ROLLUP1_DESHUM_STOP_HOUR))&&(greenhouseTemperature.value() > DESHUM_MININIM)&&(deshum == true)){
      R1.forceMove(ROLLUP1_DESHUM_INCREMENT);
      if(R1.override() == true){
        rollup1Deshum = true;
      }
    }
    else{
      rollup1Deshum = false;
    }
  #endif
  #if defined(ROLLUP2_DESHUM)  && ROLLUPS == 2
    if((((greenhouse.rightNow(2) == ROLLUP2_DESHUM_START_HOUR)&&(greenhouse.rightNow(1) >= ROLLUP2_DESHUM_START_MIN))||(greenhouse.rightNow(2)>ROLLUP2_DESHUM_START_HOUR))&&(((greenhouse.rightNow(2) == ROLLUP2_DESHUM_STOP_HOUR)&&(greenhouse.rightNow(1) < ROLLUP2_DESHUM_STOP_MIN))||(greenhouse.rightNow(2) < ROLLUP2_DESHUM_STOP_HOUR))&&(greenhouseTemperature.value() > DESHUM_MININIM)&&(deshum == true)){
      R2.forceMove(ROLLUP2_DESHUM_INCREMENT);
      if(R2.override() == true){
        rollup2Deshum = true;
      }
    }
    else{
      rollup2Deshum = false;
    }
  #endif
  #if defined(FAN1_DESHUM) && FANS >= 1
    if((((greenhouse.rightNow(2) == FAN1_DESHUM_START_HOUR)&&(greenhouse.rightNow(1) >= FAN1_DESHUM_START_MIN))||(greenhouse.rightNow(2)>FAN1_DESHUM_START_HOUR))&&(((greenhouse.rightNow(2) == FAN1_DESHUM_STOP_HOUR)&&(greenhouse.rightNow(1) < FAN1_DESHUM_STOP_MIN))||(greenhouse.rightNow(2) < FAN1_DESHUM_STOP_HOUR))&&(greenhouseTemperature.value() > DESHUM_MININIM)&&(deshum == true)){
      F1.forceAction(true);
      fan1Deshum = true;
    }
    else{
      fan1Deshum = false;
    }
  #endif
  #if defined(FAN2_DESHUM) && FANS == 2
    if((((greenhouse.rightNow(2) == FAN2_DESHUM_START_HOUR)&&(greenhouse.rightNow(1) >= FAN2_DESHUM_START_MIN))||(greenhouse.rightNow(2)>FAN2_DESHUM_START_HOUR))&&(((greenhouse.rightNow(2) == FAN2_DESHUM_STOP_HOUR)&&(greenhouse.rightNow(1) < FAN2_DESHUM_STOP_MIN))||(greenhouse.rightNow(2) < FAN2_DESHUM_STOP_HOUR))&&(greenhouseTemperature.value() > DESHUM_MININIM)&&(deshum == true)){
      F2.forceAction(true);
      fan2Deshum = true;
    }
    else{
      fan2Deshum = false;
    }
  #endif
  #if defined(HEATER1_DESHUM) && HEATERS >= 1
    if((((greenhouse.rightNow(2) == HEATER1_DESHUM_START_HOUR)&&(greenhouse.rightNow(1) >= HEATER1_DESHUM_START_MIN))||(greenhouse.rightNow(2)>HEATER1_DESHUM_START_HOUR))&&(((greenhouse.rightNow(2) == HEATER1_DESHUM_STOP_HOUR)&&(greenhouse.rightNow(1) < HEATER1_DESHUM_STOP_MIN))||(greenhouse.rightNow(2) < HEATER1_DESHUM_STOP_HOUR))&&(greenhouseTemperature.value() > DESHUM_MININIM)&&(deshum == true)){
      H1.forceAction(true);
      heater1Deshum = true;
    }
    else{
      heater1Deshum = false;
    }
  #endif
}
