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
#include "Parameters.h"



//****************************************************************
//*******************TIMEZONES FUNCTIONS**************************
//****************************************************************

unsigned short Timepoint::_EEPROMindex = 0;
unsigned short Timepoint::_counter = 0;

Timepoint::Timepoint(){

  _localIndex = TIMEPOINT_INDEX + _EEPROMindex;
  _EEPROMindex += 24;
  _localCounter = _counter;
  _counter++;


  type.setLimits(0, 2);
  type.setAddress(_localIndex);
  _localIndex += 1;

  mnMod.setLimits(0, 59);
  mnMod.setAddress(_localIndex);
  _localIndex += sizeof(short);

  hrMod.setLimits(0,23);
  hrMod.setAddress(_localIndex);
  _localIndex += sizeof(short);

  coolingTemp.setLimits(0,50);
  coolingTemp.setAddress(_localIndex);
  _localIndex += sizeof(float);

  heatingTemp.setLimits(0,50);
  heatingTemp.setAddress(_localIndex);
  _localIndex += sizeof(float);

  coolingTempCloud.setLimits(0,50);
  coolingTempCloud.setAddress(_localIndex);
  _localIndex += sizeof(float);

  heatingTempCloud.setLimits(0,50);
  heatingTempCloud.setAddress(_localIndex);
  _localIndex += sizeof(float);

  ramping.setLimits(0,60);
  ramping.setAddress(_localIndex);
  _localIndex += sizeof(unsigned short);

  EEPROMTimer = 0;
}

Timepoint::~Timepoint(){}

void Timepoint::setParameters(byte typ, short hr, short mn, float heatTemp, float coolTemp, float heatTempCloud, float coolTempCloud, unsigned short ramp){
    type.setValue(typ);
    setTimepoint(hr, mn);
    coolingTemp.setValue(coolTemp);
    coolingTempCloud.setValue(coolTempCloud);
    heatingTemp.setValue(heatTemp);
    heatingTempCloud.setValue(heatTempCloud);
    ramping.setValue(ramp);

    #ifdef DEBUG_TIMEPOINTS
      Serial.println(F("-----"));
      Serial.println(F("Timepoint"));
      Serial.print(F("Time : "));
      Serial.print(_hr);
      Serial.print(F(":"));
      Serial.println(_mn);
      Serial.print(F("Cooling temp : "));
      Serial.println(coolingTemp.value());
      Serial.print(F("Heating temp : "));
      Serial.println(heatingTemp.value());
      Serial.print(F("Ramping : "));
      Serial.println(ramping);
    #endif

}

void Timepoint::setTimepoint(short hourMod, short minutMod){

  if(type.value() == SR || type.value() == SS){
    hrMod.setLimits(-23,23);
    mnMod.setLimits(-59,59);
  }
  else{
    hrMod.setLimits(0,23);
    mnMod.setLimits(0,59);
  }

  if (type.value() == SR){
		_time.setTime(sunRise[HOUR], sunRise[MINUT]);
    _time.addTime(hourMod, minutMod);
		_hr = _time.hour();
		_mn = _time.minut();
	}
	else if (type.value() == CLOCK){
		_hr = hourMod;
		_mn = minutMod;
  }

  else if (type.value() == SS){
		_time.setTime(sunSet[HOUR], sunSet[MINUT]);
    _time.addTime(hourMod, minutMod);
    _hr = _time.hour();
		_mn = _time.minut();
	}
  hrMod.setValue(hourMod);
  mnMod.setValue(minutMod);

}
void Timepoint::updateTimepoint(){

  if (type.value() == SR){
		_time.setTime(sunRise[HOUR], sunRise[MINUT]);
    _time.addTime(hrMod.value(), mnMod.value());
		_hr = _time.hour();
		_mn = _time.minut();
	}
	else if (type.value() == CLOCK){
		_hr = hrMod.value();
		_mn = mnMod.value();
  }

  else if (type.value() == SS){
		_time.setTime(sunSet[HOUR], sunSet[MINUT]);
    _time.addTime(hrMod.value(), mnMod.value());
    _hr = _time.hour();
		_mn = _time.minut();
	}
}
unsigned short Timepoint::hr(){
  return _hr;
}

unsigned short Timepoint::mn(){
  return _mn;
}

unsigned short Timepoint::nb(){
  return _localCounter;
}

void Timepoint::EEPROMPut(){
  type.loadInEEPROM();
  mnMod.loadInEEPROM();
  hrMod.loadInEEPROM();
  heatingTemp.loadInEEPROM();
  coolingTemp.loadInEEPROM();
  coolingTempCloud.loadInEEPROM();
  heatingTempCloud.loadInEEPROM();
  ramping.loadInEEPROM();
}

void Timepoint::EEPROMGet(){

  byte typ = EEPROM.read(type.address());
  short hr;
  EEPROM.get(hrMod.address(), hr);
  short mn;
  EEPROM.get(mnMod.address(), mn);
  float heatTemp;
  EEPROM.get(heatingTemp.address(), heatTemp);
  float coolTemp;
  EEPROM.get(coolingTemp.address(), coolTemp);
  float heatTempCloud;
  EEPROM.get(heatingTempCloud.address(), heatTempCloud);
  float coolTempCloud;
  EEPROM.get(coolingTempCloud.address(), coolTempCloud);
  unsigned short ramp;
  EEPROM.get(ramping.address(), ramp);

  type.setValue(typ);
  setTimepoint(hr, mn);
  coolingTemp.setValue(coolTemp);
  coolingTempCloud.setValue(coolTempCloud);
  heatingTemp.setValue(heatTemp);
  heatingTempCloud.setValue(heatTempCloud);
  ramping.setValue(ramp);

  #ifdef DEBUG_EEPROM
    Serial.println(F("-------------------"));
    Serial.print(F("-----TIMEPOINT "));
    Serial.print(_localCounter);
    Serial.println(F("------"));
    Serial.print(F("Address: "));
    Serial.print(type.address());
    Serial.print(F(" - Value :"));
    Serial.print(type.value());
    Serial.println(F("   - (Type)"));
    Serial.print(F("Address: "));
    Serial.print(hrMod.address());
    Serial.print(F(" - Value :"));
    Serial.print(hr);
    Serial.println(F("   - (Hour)"));
    Serial.print(F("Address: "));
    Serial.print(mnMod.address());
    Serial.print(F(" - Value :"));
    Serial.print(mn);
    Serial.println(F("   - (Min)"));
    Serial.print(F("Address: "));
    Serial.print(heatingTemp.address());
    Serial.print(F(" - Value :"));
    Serial.print(heatingTemp.value());
    Serial.println(F(" - (Heating Temperature - Sun)"));
    Serial.print(F("Address: "));
    Serial.print(coolingTemp.address());
    Serial.print(F(" - Value :"));
    Serial.print(coolingTemp.value());
    Serial.println(F(" - (Cooling Temperature - Sun)"));
    Serial.print(F("Address: "));
    Serial.print(heatingTempCloud.address());
    Serial.print(F(" - Value :"));
    Serial.print(heatingTempCloud.value());
    Serial.println(F(" - (Heating Temperature - Cloud)"));
    Serial.print(F("Address: "));
    Serial.print(coolingTempCloud.address());
    Serial.print(F(" - Value :"));
    Serial.print(coolingTempCloud.value());
    Serial.println(F(" - (Cooling Temperature - Cloud)"));
    Serial.print(F("Address: "));
    Serial.print(ramping.address());
    Serial.print(F(" - Value :"));
    Serial.print(ramping.value());
    Serial.println(F("   - (Ramping)"));
  #endif
}


short Timepoint::sunRise[3] = {0};
short Timepoint::sunSet[3] = {0};
