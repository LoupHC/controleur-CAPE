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
  _weather = SUN;
  _alarmEnabled = false;
  for (byte x = 0; x < _rollups; x++){
    rollup[x].setStages(_stages);
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
    rollup[x].EEPROMPut();
    rollup[x].routine(_coolingTemp, greenhouseTemperature);
  }
  #endif
  #if MAX_FANS >= 1
  for (byte x = 0; x < _fans; x++){
    fan[x].EEPROMPut();
    fan[x].routine(_coolingTemp, greenhouseTemperature);
  }
  #endif
  #if MAX_HEATERS >= 1
  for (byte x = 0; x < _heaters; x++){
    heater[x].EEPROMPut();
    heater[x].routine(_heatingTemp, greenhouseTemperature);
  }
  #endif
  for (byte x = 0; x < _timepoints; x++){
    timepoint[x].EEPROMPut();
  }
  if(_alarmEnabled){
    checkAlarm(greenhouseTemperature);
  }
}

void Greenhouse::fullRoutine(byte rightNow[6], float* coolingTemp, float* heatingTemp){
  setNow(rightNow);
  solarCalculations();
  checkProgramSuccession();
  selectActualProgram();
  startRamping();
  #if MAX_ROLLUPS >= 1
  for (byte x = 0; x < _rollups; x++){
    rollup[x].EEPROMPut();
  }
  #endif
  #if MAX_FANS >= 1
  for (byte x = 0; x < _fans; x++){
    fan[x].EEPROMPut();
  }
  #endif
  #if MAX_HEATERS >= 1
  for (byte x = 0; x < _heaters; x++){
    heater[x].EEPROMPut();
  }
  #endif
  for (byte x = 0; x < _timepoints; x++){
    timepoint[x].EEPROMPut();
  }
  *coolingTemp = _coolingTemp;
  *heatingTemp = _heatingTemp;
}

void Greenhouse::solarCalculations(){
  initTimeLord(_timezone, _latitude, _longitude);
  //Première lecture d'horloge pour définir le lever et coucher du soleil
  setSunrise();
  setSunset();
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
        timepoint[x].type.setValue(timepoint[x-1].type.value());
        timepoint[x].setTimepoint(timepoint[x-1].hrMod.value(),timepoint[x-1].mnMod.value());
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
  if(_weather == SUN){
    _coolingTemp = timepoint[_timepoint-1].coolingTemp.value();
    _heatingTemp = timepoint[_timepoint-1].heatingTemp.value();
  }
  else if(_weather == CLOUD){
      _coolingTemp = timepoint[_timepoint-1].coolingTempCloud.value();
      _heatingTemp = timepoint[_timepoint-1].heatingTempCloud.value();
  }
}

void Greenhouse::startRamping(){
  if(_weather == SUN){
    _newCoolingTemp = timepoint[_timepoint-1].coolingTemp.value();
    _newHeatingTemp = timepoint[_timepoint-1].heatingTemp.value();
  }
  else if(_weather == CLOUD){
      _newCoolingTemp = timepoint[_timepoint-1].coolingTempCloud.value();
      _newHeatingTemp = timepoint[_timepoint-1].heatingTempCloud.value();
  }

  unsigned long rampTime = (unsigned long)timepoint[_timepoint-1].ramping.value()*60000;

  if (ramping > rampTime){

    if (_newCoolingTemp > _coolingTemp){
      _coolingTemp += 0.5;
      if(_coolingTemp > _newCoolingTemp){
        _coolingTemp = _newCoolingTemp;
      }
    }
    else if (_newCoolingTemp < _coolingTemp){
      _coolingTemp -= 0.5;
      if(_coolingTemp < _newCoolingTemp){
        _coolingTemp = _newCoolingTemp;
      }
    }
    if (_newHeatingTemp > _heatingTemp){
      _heatingTemp += 0.5;
      if(_heatingTemp > _newHeatingTemp){
        _heatingTemp = _newHeatingTemp;
      }
    }
    else if (_newHeatingTemp < _heatingTemp){
      _heatingTemp -= 0.5;
      if(_heatingTemp < _newHeatingTemp){
        _heatingTemp = _newHeatingTemp;
      }
    }
    ramping = 0;
  }
}

void Greenhouse::setWeather(byte weather){
  _weather = weather;
}

byte Greenhouse::rightNow(byte index){
  return _rightNow[index];
}

byte Greenhouse::weather(){
  return _weather;
}
byte Greenhouse::nowTimepoint(){
  return _timepoint;
}
float Greenhouse::heatingTemp(){
  return _heatingTemp;
}
float Greenhouse::coolingTemp(){
  return _coolingTemp;
}

void Greenhouse::checkAlarm(float temperature){
  if(_alarmMin != OFF_VAL){
    if(temperature < _alarmMin){
      alarmBlast();
    }
    else if(temperature > _alarmMax){
      alarmBlast();
    }
    else{
      stopAlarm();
    }
  }
  if(_alarmMax != OFF_VAL){
  }
}
void Greenhouse::addAlarm(boolean relayType, byte alarmPin){
  if (_alarmPin != OFF){
    _alarmRelayType = relayType;
    _alarmPin = alarmPin;
    _alarmEnabled = true;
    _alarmMax = OFF_VAL;
    _alarmMin = OFF_VAL;
    #ifdef IOS_OUTPUTS
        pinMode(_alarmPin, OUTPUT);
        if(_alarmRelayType == ACT_LOW){
          digitalWrite(_alarmPin, HIGH);
        }
        else{
          digitalWrite(_alarmPin, LOW);
        }
    #endif
      #ifdef MCP_I2C_OUTPUTS
          mcp.pinMode(_alarmPin, OUTPUT);
          if(_alarmRelayType == ACT_LOW){
            mcp.digitalWrite(_alarmPin, HIGH);
          }
          else{
            mcp.digitalWrite(_alarmPin, LOW);
          }
    #endif

  }

}
void Greenhouse::setAlarmMaxTemp(float temperature){
  _alarmMax = temperature;
}
void Greenhouse::setAlarmMinTemp(float temperature){
  _alarmMin = temperature;
}
void Greenhouse::alarmBlast(){
  if(_alarmEnabled){
    if(_alarmIsTriggered == false){
      #ifdef IOS_OUTPUTS
        if(_alarmRelayType == ACT_LOW){
          digitalWrite(_alarmPin, LOW);
        }
        else{
          digitalWrite(_alarmPin, HIGH);
        }
      #endif

      #ifdef MCP_I2C_OUTPUTS
        if(_alarmRelayType == ACT_LOW){
          mcp.digitalWrite(_alarmPin, LOW);
        }
        else{
          mcp.digitalWrite(_alarmPin, HIGH);
        }
      #endif
      _alarmIsTriggered = true;
    }
  }
}
void Greenhouse::stopAlarm(){
  if(_alarmEnabled){
    if (_alarmIsTriggered){
      #ifdef IOS_OUTPUTS
        if(_alarmRelayType == ACT_LOW){
          digitalWrite(_alarmPin, HIGH);
        }
        else{
          digitalWrite(_alarmPin, LOW);
        }
      #endif
        #ifdef MCP_I2C_OUTPUTS
          if(_alarmRelayType == ACT_LOW){
            mcp.digitalWrite(_alarmPin, HIGH);
          }
          else{
            mcp.digitalWrite(_alarmPin, LOW);
          }
      #endif
      _alarmIsTriggered = false;
    }
  }
}
