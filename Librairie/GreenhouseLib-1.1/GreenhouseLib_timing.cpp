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
  _EEPROMindex += 15;
  _localCounter = _counter;
  _counter++;


  _type.setLimits(0, 2);
  _type.setAddress(_localIndex);
  _localIndex += 1;

  _mnMod.setLimits(0, 59);
  _mnMod.setAddress(_localIndex);
  _localIndex += sizeof(short);

  _hrMod.setLimits(0,23);
  _hrMod.setAddress(_localIndex);
  _localIndex += sizeof(short);

  _coolingTemp.setLimits(0,50);
  _coolingTemp.setAddress(_localIndex);
  _localIndex += sizeof(float);

  _heatingTemp.setLimits(0,50);
  _heatingTemp.setAddress(_localIndex);
  _localIndex += sizeof(float);

  _ramping.setLimits(0,60);
  _ramping.setAddress(_localIndex);
  _localIndex += sizeof(unsigned short);

  EEPROMTimer = 0;
}

Timepoint::~Timepoint(){}

void Timepoint::checkTime(){
  if (_type.value() == SR){
    _time.setTime(sunRise[HOUR], sunRise[MINUT]);
    _time.addTime(_hrMod.value(), _mnMod.value());
    _hr = _time.hour();
    _mn = _time.minut();
  }
  else if (_type.value() == CLOCK){
    _hr = _hrMod.value();
    _mn = _mnMod.value();
  }

  else if (_type.value() == SS){
    _time.setTime(sunSet[HOUR], sunSet[MINUT]);
    _time.addTime(_hrMod.value(), _mnMod.value());
    _hr = _time.hour();
    _mn = _time.minut();
  }
}

void Timepoint::setParameters(byte type, short hour, short min, float heatingTemp, float coolingTemp, unsigned short ramping){
    setTime(type, hour, min);
    setHeatTemp(heatingTemp);
    setCoolTemp(coolingTemp);
    setRamping(ramping);

    #ifdef DEBUG_TIMEPOINTS
      Serial.println(F("-----"));
      Serial.println(F("Timepoint"));
      Serial.print(F("Time : "));
      Serial.print(_hr);
      Serial.print(F(":"));
      Serial.println(_mn);
      Serial.print(F("Cooling temp : "));
      Serial.println(_coolingTemp.value());
      Serial.print(F("Heating temp : "));
      Serial.println(_heatingTemp.value());
      Serial.print(F("Ramping : "));
      Serial.println(ramping);
    #endif

}

void Timepoint::setTime(byte type, short hourMod, short minutMod){

  if(type == SR || type == SS){
    _hrMod.setLimits(-23,23);
    _mnMod.setLimits(-59,59);
  }
  else{
    _hrMod.setLimits(0,23);
    _mnMod.setLimits(0,59);
  }

  if (type == SR){
		_time.setTime(sunRise[HOUR], sunRise[MINUT]);
    _time.addTime(hourMod, minutMod);
		_hr = _time.hour();
		_mn = _time.minut();
	}
	else if (type == CLOCK){
		_hr = hourMod;
		_mn = minutMod;
  }

  else if (type == SS){
		_time.setTime(sunSet[HOUR], sunSet[MINUT]);
    _time.addTime(hourMod, minutMod);
    _hr = _time.hour();
		_mn = _time.minut();
	}

  _type.setValue(type);
  EEPROM.update(_type.address(),type);
  _hrMod.setValue(hourMod);
  EEPROM.put(_hrMod.address(), hourMod);
  _mnMod.setValue(minutMod);
  EEPROM.put(_mnMod.address(),minutMod);
}

void Timepoint::setHeatTemp(float heatingTemp){
	_heatingTemp.setValue(heatingTemp);
  EEPROM.put(_heatingTemp.address(),heatingTemp);
}

void Timepoint::setCoolTemp(float coolingTemp){
	_coolingTemp.setValue(coolingTemp);
  EEPROM.put(_coolingTemp.address(),coolingTemp);
}

void Timepoint::setRamping(unsigned short ramping){
  _ramping.setValue(ramping);
  EEPROM.put(_ramping.address(),ramping);
}

byte Timepoint::type(){
  return _type.value();
}

short Timepoint::hrMod(){
  return _hrMod.value();
}
unsigned short Timepoint::hr(){
  return _hr;
}
short Timepoint::hrModMin(){
  return _hrMod.minimum();
}
short Timepoint::hrModMax(){
  return _hrMod.maximum();
}
short Timepoint::mnMod(){
  return _mnMod.value();
}

unsigned short Timepoint::mn(){
  return _mn;
}

short Timepoint::mnModMin(){
  return _mnMod.minimum();
}
short Timepoint::mnModMax(){
  return _mnMod.maximum();
}
float Timepoint::heatingTemp(){
  return _heatingTemp.value();
}
float Timepoint::heatingTempMin(){
  return _heatingTemp.minimum();
}
float Timepoint::heatingTempMax(){
  return _heatingTemp.maximum();
}
float Timepoint::coolingTemp(){
  return _coolingTemp.value();
}
float Timepoint::coolingTempMin(){
  return _coolingTemp.minimum();
}
float Timepoint::coolingTempMax(){
  return _coolingTemp.maximum();
}

unsigned short Timepoint::ramping(){
  return _ramping.value();
}
unsigned short Timepoint::rampingMin(){
  return _ramping.minimum();
}
unsigned short Timepoint::rampingMax(){
  return _ramping.maximum();
}

unsigned short Timepoint::nb(){
  return _localCounter;
}

void Timepoint::loadEEPROMParameters(){

  byte type = EEPROM.read(_type.address());
  short hr;
  EEPROM.get(_hrMod.address(), hr);
  short mn;
  EEPROM.get(_mnMod.address(), mn);
  setTime(type, hr, mn);

  float heatingTemp;
  EEPROM.get(_heatingTemp.address(), heatingTemp);
  setHeatTemp(heatingTemp);

  float coolingTemp;
  EEPROM.get(_coolingTemp.address(), coolingTemp);
  setCoolTemp(coolingTemp);

  unsigned short ramping;
  EEPROM.get(_ramping.address(), ramping);
  setRamping(ramping);

  #ifdef DEBUG_EEPROM
    Serial.println(F("-------------------"));
    Serial.print(F("-----TIMEPOINT "));
    Serial.print(_localCounter);
    Serial.println(F("------"));
    Serial.print(F("Address: "));
    Serial.print(_type.address());
    Serial.print(F(" - Value :"));
    Serial.print(type);
    Serial.println(F("   - (Type)"));
    Serial.print(F("Address: "));
    Serial.print(_hrMod.address());
    Serial.print(F(" - Value :"));
    Serial.print(hr);
    Serial.println(F("   - (Hour)"));
    Serial.print(F("Address: "));
    Serial.print(_mnMod.address());
    Serial.print(F(" - Value :"));
    Serial.print(mn);
    Serial.println(F("   - (Min)"));
    Serial.print(F("Address: "));
    Serial.print(_heatingTemp.address());
    Serial.print(F(" - Value :"));
    Serial.print(heatingTemp);
    Serial.println(F(" - (Heating Temperature)"));
    Serial.print(F("Address: "));
    Serial.print(_coolingTemp.address());
    Serial.print(F(" - Value :"));
    Serial.print(coolingTemp);
    Serial.println(F(" - (Cooling Temperature)"));
    Serial.print(F("Address: "));
    Serial.print(_ramping.address());
    Serial.print(F(" - Value :"));
    Serial.print(ramping);
    Serial.println(F("   - (Ramping)"));
  #endif
}


short Timepoint::sunRise[3] = {0};
short Timepoint::sunSet[3] = {0};
