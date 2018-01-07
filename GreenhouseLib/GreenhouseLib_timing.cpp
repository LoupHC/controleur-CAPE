/*
  GreenhouseLib_timing.cpp
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
#include "GreenhouseLib_timing.h"



//****************************************************************
//*******************TIMEZONES FUNCTIONS**************************
//****************************************************************
Timepoint::Timepoint(){
  _type.setLimits(0, 2);
  _mn.setLimits(-60, 60);
  _hr.setLimits(0,60);
  _coolingTemp.setLimits(0,50);
  _heatingTemp.setLimits(0,50);
  _localIndex = TIMEPOINT_INDEX + _index;
  _index += 5;
  EEPROMTimer = 0;
}

Timepoint::~Timepoint(){}

void Timepoint::setParameters(byte type, short hour, short min, float heatingTemp, float coolingTemp){
  	setType(type);
    setTime(hour, min);
    setHeatTemp(heatingTemp);
    setCoolTemp(coolingTemp);

    #ifdef DEBUG_PROGRAM
      Serial.println(F("-----"));
      Serial.println(F("Timepoint"));
      Serial.print(F("Time : "));
      Serial.print(_hr.value());
      Serial.print(F(":"));
      Serial.println(_mn.value());
      Serial.print(F("Cooling temp : "));
      Serial.println(_coolingTemp.value());
      Serial.print(F("Heating temp : "));
      Serial.println(_heatingTemp.value());
    #endif

}
void Timepoint::setTime(short mod){
	if (_type.value() == SR){
		short Time[3];
		Time[HOUR] = sunRise[HOUR];
		Time[MINUT] = sunRise[MINUT] + mod;
		convertDecimalToTime(&Time[HOUR], &Time[MINUT]);
		_hr.setValue(Time[HOUR]);
		_mn.setValue(Time[MINUT]);
	}
	else if (_type.value() == SS){
		short Time[3];
		Time[HOUR] = sunSet[HOUR];
		Time[MINUT] = sunSet[MINUT] + mod;
		convertDecimalToTime(&Time[HOUR], &Time[MINUT]);
		_hr.setValue(Time[HOUR]);
		_mn.setValue(Time[MINUT]);
	}
}

void Timepoint::setTime(short hour, short min){
	if (_type.value() == SR){
		short Time[3];
		Time[HOUR] = sunRise[HOUR];
		Time[MINUT] = sunRise[MINUT] + min;
		convertDecimalToTime(&Time[HOUR], &Time[MINUT]);
		_hr.setValue(Time[HOUR]);
		_mn.setValue(Time[MINUT]);
	}
	else if (_type.value() == CLOCK){
		_hr.setValue(hour);
		_mn.setValue(min);
  }
	else if (_type.value() == SS){
		short Time[3];
		Time[HOUR] = sunSet[HOUR];
		Time[MINUT] = sunSet[MINUT] + min;
		convertDecimalToTime(&Time[HOUR], &Time[MINUT]);
		_hr.setValue(Time[HOUR]);
		_mn.setValue(Time[MINUT]);
	}
}

void Timepoint::setHeatTemp(float heatingTemp){
	_heatingTemp.setValue(heatingTemp);
}

void Timepoint::setCoolTemp(float coolingTemp){
	_coolingTemp.setValue(coolingTemp);
}

void Timepoint::setType(byte type){
	_type.setValue(type);
  //If type is anything but CLOCK, minimum value for "mn" parameter is -60. Otherwise it's 0.
  if (_type.value() == CLOCK){_mn.minimum(0);}
  else{_mn.minimum(-60);}
}
/*
void Timepoint::loadEEPROMParameters(){ndex+HEAT_INDEX));
  setCoolTemp((float)EEPROM.read(_localIndex+COOL_INDEX));
}

void Timepoint::setParametersInEEPROM(byte type, short hour, short min, float heatingTemp, float coolingTemp){
	setTimeInEEPROM(type, hour, min);
	setHeatTempInEEPROM(heatingTemp);
	setCoolTempInEEPROM(coolingTemp);
}

void Timepoint::setTimeInEEPROM(byte type, short hour, short min){
		EEPROM.update(_localIndex+HOUR_INDEX, hour);
		EEPROM.update(_localIndex+MIN_INDEX, min);
		EEPROM.update(_localIndex+TYPE_INDEX, type);
}

void Timepoint::setHeatTempInEEPROM(float heatingTemp){
		EEPROM.update(_localIndex+COOL_INDEX, heatingTemp);
}
void Timepoint::setCoolTempInEEPROM(float coolingTemp){
		EEPROM.update(_localIndex+COOL_INDEX, coolingTemp);
}
#include "GreenhouseLib_actuators.h"

/*
void Timepoint::EEPROMUpdate(){
  unsigned long currentMillis = millis();
  if (EEPROMTimer > 10000) {
    EEPROMTimer = 0;

    if(_type.value() != EEPROM.read(_localIndex+ TYPE_INDEX)){
    		EEPROM.update(_localIndex+TYPE_INDEX, _type.value());
    }
    if(_hr.value() != EEPROM.read(_localIndex+ HOUR_INDEX)){
    		EEPROM.update(_localIndex+HOUR_INDEX, _hr.value());
    }
    if(_mn.value() != EEPROM.read(_localIndex+ MIN_INDEX)){
    		EEPROM.update(_localIndex+MIN_INDEX, _mn.value());
    }
    if(_heatingTemp.value() != EEPROM.read(_localIndex+ HEAT_INDEX)){
    		EEPROM.update(_localIndex+HEAT_INDEX, _heatingTemp.value());
    }
    if(_coolingTemp.value() != EEPROM.read(_localIndex+ COOL_INDEX)){
    		EEPROM.update(_localIndex+COOL_INDEX, _coolingTemp.value());
    }
 }
}
*/
unsigned short Timepoint::hr(){
  return _hr.value();
}
unsigned short Timepoint::mn(){
  return _mn.value();
}
float Timepoint::heatingTemp(){
  return _heatingTemp.value();
}
float Timepoint::coolingTemp(){
  return _coolingTemp.value();
}

static unsigned short Timepoint::_index = 0;

static short Timepoint::sunRise[3] = {0};
static short Timepoint::sunSet[3] = {0};
