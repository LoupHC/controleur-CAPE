/*
  GreenhouseLib.cpp
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
#include "GreenhouseLib.h"


Greenhouse::Greenhouse(int timezone, float latitude, float longitude, byte timepoints, byte rollups, byte stages, byte fans, byte heaters){
  _timepoints = timepoints;
  _rollups = rollups;
  _stages = stages;
  _fans = fans;
  _heaters = heaters;
  _timezone = timezone;
  _latitude = latitude;
  _longitude = longitude;
  for (byte x = 0; x < _rollups; x++){
    rollup[x].setStages(_stages+1);
  }
}

Greenhouse::~Greenhouse(){
}

void Greenhouse::setNow(byte rightNow[6]){

  for(byte x = 0; x < 6 ; x++){
      _rightNow[x] = rightNow[x];
  }

    myLord.DST(_rightNow);
}

void Greenhouse::fullRoutine(byte rightNow[6], float greenhouseTemperature){
  setNow(rightNow);
  solarCalculations();
  checkProgramSuccession();
  selectActualProgram();
  startRamping();
  #if MAX_ROLLUPS >= 1
  for (byte x = 0; x < _rollups; x++){
    rollup[x].manualRoutine(_coolingTemp, greenhouseTemperature);
  }
  #endif
  #if MAX_FANS >= 1
  for (byte x = 0; x < _fans; x++){
    fan[x].routine(_coolingTemp, greenhouseTemperature);
  }
  #endif
  #if MAX_HEATERS >= 1
  for (byte x = 0; x < _heaters; x++){

    heater[x].routine(_heatingTemp, greenhouseTemperature);
  }
  #endif
}

void Greenhouse::solarCalculations(){
  initTimeLord(_timezone, _latitude, _longitude);
  //Première lecture d'horloge pour définir le lever et coucher du soleil
  setSunrise();
  setSunset();
  for(int x = 0; x < _timepoints;x++){
    timepoint[x].checkTime();
  }
}

void Greenhouse::initTimeLord(int timezone, float latitude, float longitude){
  myLord.TimeZone(timezone * 60);
  myLord.Position(latitude, longitude);
  myLord.DstRules(3,2,11,1,60); // DST Rules for USA
}


void Greenhouse::setSunrise(){
  //Définit l'heure du lever et coucher du soleil
  for(byte x = 0; x < 6 ; x++){
      _sunTime[x] = _rightNow[x];
  }
  myLord.SunRise(_sunTime); ///On détermine l'heure du lever du soleil
  Timepoint::sunRise[HOUR] = (short)_sunTime[HOUR];
  Timepoint::sunRise[MINUT] = (short)_sunTime[MINUT];

  #ifdef DEBUG_SOLARCALC
    Serial.print("lever du soleil :");Serial.print(Timepoint::sunRise[HOUR]);  Serial.print(":");  Serial.println(Timepoint::sunRise[MINUT]);
    Serial.println("----");
  #endif
}

void Greenhouse::setSunset(){
  // Sunset:
  for(byte x = 0; x < 6 ; x++){
      _sunTime[x] = _rightNow[x];
  }
  myLord.SunSet(_sunTime); // Computes Sun Set. Prints:
  Timepoint::sunSet[HOUR] = (short)_sunTime[HOUR];
  Timepoint::sunSet[MINUT] = (short)_sunTime[MINUT];
  #ifdef DEBUG_SOLARCALC
    Serial.print("coucher du soleil :");  Serial.print(Timepoint::sunSet[HOUR]);  Serial.print(":");  Serial.println(Timepoint::sunSet[MINUT]);
    Serial.println("----");
  #endif
}

void Greenhouse::startingParameters(){
  //Exécution du programme
  selectActualProgram();
  setTempCible();

}
void Greenhouse::checkProgramSuccession(){
  if(_timepoints > 1){
    for(int x = 1; x < _timepoints;x++){
      if (((timepoint[x].hr() == timepoint[x-1].hr())  && (timepoint[x].mn() < timepoint[x-1].mn()))||(timepoint[x].hr() < timepoint[x-1].hr())){
        timepoint[x].setTime(timepoint[x-1].type(),timepoint[x-1].hrMod(),timepoint[x-1].mnMod());
        Serial.println("Error");
      }
    }
  }
}
void Greenhouse::selectActualProgram(){
  //Sélectionne le programme en cour

    #ifdef DEBUG_PROGRAM
      Serial.println("----");
      Serial.print ("Heure actuelle ");Serial.print(" : ");Serial.print(_rightNow[HOUR] );Serial.print(" : ");Serial.println(_rightNow[MINUT]);
    #endif
    for (byte y = 0; y < (_timepoints-1); y++){

    #ifdef DEBUG_PROGRAM
      Serial.print ("Programme "); Serial.print(y+1);Serial.print(" : ");Serial.print(timepoint[y].hr());Serial.print(" : ");Serial.println(timepoint[y].mn());
    #endif
      if (((_rightNow[HOUR] == timepoint[y].hr())  && (_rightNow[MINUT] >= timepoint[y].mn()))||((_rightNow[HOUR] > timepoint[y].hr()) && (_rightNow[HOUR] < timepoint[y+1].hr()))||((_rightNow[HOUR] == timepoint[y+1].hr())  && (_rightNow[MINUT] <timepoint[y+1].mn()))){
          _timepoint = y+1;
        }
    }

    #ifdef DEBUG_PROGRAM
      Serial.print ("Programme ");Serial.print(_timepoints);Serial.print(" : ");Serial.print(timepoint[_timepoints-1].hr());Serial.print(" : ");Serial.println(timepoint[_timepoints-1].mn());
    #endif

    if (((_rightNow[HOUR] == timepoint[_timepoints-1].hr())  && (_rightNow[MINUT] >= timepoint[_timepoints-1].mn()))||(_rightNow[HOUR] > timepoint[_timepoints-1].hr())||(_rightNow[HOUR] < timepoint[0].hr())||((_rightNow[HOUR] == timepoint[0].hr())  && (_rightNow[MINUT] < timepoint[0].mn()))){
      _timepoint = _timepoints;
    }
    #ifdef DEBUG_PROGRAM
      Serial.print ("Program is : ");
      Serial.println(_timepoint);
      Serial.println("----");
    #endif
}

void Greenhouse::setTempCible(){
  _coolingTemp = timepoint[_timepoint-1].coolingTemp();
  _heatingTemp = timepoint[_timepoint-1].heatingTemp();
}

void Greenhouse::startRamping(){
  //Définition des variables locales
  float newHeatingTemp;
  float newCoolingTemp;

  newHeatingTemp = timepoint[_timepoint-1].heatingTemp();
  newCoolingTemp = timepoint[_timepoint-1].coolingTemp();
  unsigned long rampTime = (unsigned long)timepoint[_timepoint-1].ramping()*60000;
  if (ramping > rampTime){

    if (newCoolingTemp > _coolingTemp){
      _coolingTemp += 0.5;
      if(_coolingTemp > newCoolingTemp){
        _coolingTemp = newCoolingTemp;
      }
    }
    else if (newCoolingTemp < _coolingTemp){
      _coolingTemp -= 0.5;
      if(_coolingTemp < newCoolingTemp){
        _coolingTemp = newCoolingTemp;
      }
    }
    if (newHeatingTemp > _heatingTemp){
      _heatingTemp += 0.5;
      if(_heatingTemp > newHeatingTemp){
        _heatingTemp = newHeatingTemp;
      }
    }
    else if (newHeatingTemp < _heatingTemp){
      _heatingTemp -= 0.5;
      if(_heatingTemp < newHeatingTemp){
        _heatingTemp = newHeatingTemp;
      }
    }
    ramping = 0;
  }
}

byte Greenhouse::rightNow(byte index){
  return _rightNow[index];
}
