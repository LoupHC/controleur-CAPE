/*
  GreenhouseLib.h
  Copyright (C)2017 Loup Hébert-Chartrand. All right reserved

  You can find the latest version of this code at :
  https://github.com/LoupHC/GreenhouseLib

  This library is free software; you can redistribute it and/or
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

#include "Arduino.h"
#include "TimeLord.h"
#include "elapsedMillis.h"
#include "Parameters.h"
#include "Defines.h"


#include "GreenhouseLib_rollups.h"
#include "GreenhouseLib_FF.h"
#include "GreenhouseLib_timing.h"



#ifndef GreenhouseLib_h
#define GreenhouseLib_h



class Greenhouse
{
  public:
    Greenhouse(int timezone, float latitude, float longitude, byte timepoints, byte rollups, byte stages, byte fans, byte heaters);
    ~Greenhouse();
    void setNow(byte rightNow[]);
    void solarCalculations();
    void startingParameters();
    void EEPROMUpdate();
    TimeLord myLord;
    #if MAX_TIMEPOINTS >= 1
      Timepoint timepoint[MAX_TIMEPOINTS];
    #endif
    #if MAX_ROLLUPS >= 1
      Rollup rollup[MAX_ROLLUPS];
    #endif
    #if MAX_FANS >= 1
      Fan fan[MAX_FANS];
    #endif
    #if MAX_HEATERS >= 1
      Heater heater[MAX_HEATERS];
    #endif

    //Confirm timepoint, cooling/heating temperatures, routine for each outputs
    void fullRoutine(byte rightNow[], float greenhouseTemperature);
    //Update parameters in EEPROM for each instance, every 10 seconds
    //void EEPROMUpdate();
    byte rightNow(byte index);
    byte _rightNow[6];      //actual time
    byte _sunTime[6];       //for sunrise, sunset calculation
    float _heatingTemp;     //reference temperature for heating devices
    float _coolingTemp;     //reference temperature for cooling devices
    byte _timezone;         //time zone of your location (Most of Quebec : -5)
    float _longitude;
    float _latitude;
    byte _timepoint;        //actual timepoint

  private:
    byte _timepoints;        //# of timepoints
    byte _rollups;           //# of rollups
    byte _stages;
    byte _fans;              //# of fans
    byte _heaters;           //# of heaters
    void initTimeLord(int timezone, float latitude, float longitude);
    void setSunrise();
    void setSunset();
    void selectActualProgram();
    void checkProgramSuccession();
    void setTempCible();
    void startRamping();

    elapsedMillis ramping;  //ramping timer


};

#endif
