/*
  Parameters.cpp
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

#include "Parameters.h"

//Each  parameter have 3 associated values, a minimumimum value, a maximumimum value and an actual value
//float parameters


floatParameter::floatParameter(){}
floatParameter::~floatParameter(){}

void floatParameter::setAddress(int address){
  _address = address;
}
int floatParameter::address(){
  return _address;
}
void floatParameter::setValue(float value){
//If parameter's value is between minimum and maximum values, change it, else, report error
  setLastValue(_value);
  if((value >= _minimum)&&(value <= _maximum)){
    _value = value;
  }
  else{
    _value = 333;
  }
}

void floatParameter::setLastValue(float lastValue){
  if((lastValue >= _minimum)&&(lastValue <= _maximum)){
    _lastValue = lastValue;
  }
  else{
    _lastValue = 333;
  }
}

void floatParameter::updateLastValue(){
  setLastValue(_value);
}

boolean floatParameter::valueHasChanged(){
  if(_value == _lastValue){
    return false;
  }
  else{
    return true;
  }
}

void floatParameter::setLimits(float minimum, float maximum){
  _minimum = minimum;
  _maximum = maximum;
}

float floatParameter::minimum(){
  return _minimum;
}
float floatParameter::maximum(){
  return _maximum;
}

float floatParameter::value(){
  return _value;
}
float floatParameter::lastValue(){
  return _lastValue;
}

//short parameters

shortParameter::shortParameter(){}
shortParameter::~shortParameter(){}

void shortParameter::setAddress(int address){
  _address = address;
}
int shortParameter::address(){
  return _address;
}
void shortParameter::setValue(short value){
//If parameter's value is between minimum and maximum values, change it, else, report error
  setLastValue(_value);
  if((value >= _minimum)&&(value <= _maximum)){
    _value = value;
  }
  else{
    _value = 333;
  }
}
void shortParameter::setLastValue(short lastValue){
  if((lastValue >= _minimum)&&(lastValue <= _maximum)){
    _lastValue = lastValue;
  }
  else{

    _lastValue = 333;
  }
}

void shortParameter::updateLastValue(){
  setLastValue(_value);
}

boolean shortParameter::valueHasChanged(){
  if(_value == _lastValue){
    return false;
  }
  else{
    return true;
  }
}
void shortParameter::setLimits(short minimum, short maximum){
  _minimum = minimum;
  _maximum = maximum;
}

short shortParameter::minimum(){
  return _minimum;
}
short shortParameter::maximum(){
  return _maximum;
}

short shortParameter::value(){
  return _value;
}

short shortParameter::lastValue(){
  return _lastValue;
}

//unsigned short parameters

uShortParameter::uShortParameter(){}
uShortParameter::~uShortParameter(){}

void uShortParameter::setAddress(int address){
  _address = address;
}
int uShortParameter::address(){
  return _address;
}
void uShortParameter::setValue(unsigned short value){
//If parameter's value is between minimum and maximum values, change it, else, report error
  setLastValue(_value);
  if((value >= _minimum)&&(value <= _maximum)){
    _value = value;
  }
  else{
    _value = 333;
  }
}
void uShortParameter::setLastValue(unsigned short lastValue){
  if((lastValue >= _minimum)&&(lastValue <= _maximum)){
    _lastValue = lastValue;
  }
  else{
    _lastValue = 333;
  }
}

void uShortParameter::updateLastValue(){
  setLastValue(_value);
}

boolean uShortParameter::valueHasChanged(){
  if(_value == _lastValue){
    return false;
  }
  else{
    return true;
  }
}

void uShortParameter::setLimits(unsigned short minimum, unsigned short maximum){
  _minimum = minimum;
  _maximum = maximum;
}

unsigned short uShortParameter::minimum(){
  return _minimum;
}
unsigned short uShortParameter::maximum(){
  return _maximum;
}
unsigned short uShortParameter::value(){
  return _value;
}

unsigned short uShortParameter::lastValue(){
  return _lastValue;
}

//Byte parameter

byteParameter::byteParameter(){}
byteParameter::~byteParameter(){}

void byteParameter::setAddress(int address){
  _address = address;
}
int byteParameter::address(){
  return _address;
}
void byteParameter::setValue(byte value){
//If parameter's value is between minimum and maximum values, change it, else, report error
  setLastValue(_value);
  if((value >= _minimum)&&(value <= _maximum)){
    _value = value;
  }
  else{
    _value = 155;
  }
}

void byteParameter::setLastValue(byte lastValue){
  if((lastValue >= _minimum)&&(lastValue <= _maximum)){
    _lastValue = lastValue;
  }
  else{
    _lastValue = 155;
  }
}

void byteParameter::updateLastValue(){
  setLastValue(_value);
}

boolean byteParameter::valueHasChanged(){
  if(_value == _lastValue){
    return false;
  }
  else{
    return true;
  }
}

void byteParameter::setLimits (byte minimum, byte maximum){
  _minimum = minimum;
  _maximum = maximum;
}

byte byteParameter::minimum(){
  return _minimum;
}
byte byteParameter::maximum(){
  return _maximum;
}
byte byteParameter::value(){
  return _value;
}

byte byteParameter::lastValue(){
  return _lastValue;
}

//Time parameter which you can add and substract hours and minuts
timeParameter::timeParameter(){}
timeParameter::timeParameter(short hour, short minut){
  _hour = hour;
  _minut = minut;
}
timeParameter::~timeParameter(){}

void timeParameter::setTime(short hour, short minut){
    _hour = hour;
    _minut = minut;
}

void timeParameter::addTime(short hour, short minut){
  _hour += hour;
  _minut += minut;
  adjustTime();

}
void timeParameter::subTime(short hour, short minut){
    _hour -= hour;
    _minut -= minut;
    adjustTime();
}
short timeParameter::hour(){
  return _hour;
}
short timeParameter::minut(){
  return _minut;
}
void timeParameter::adjustTime(){
  while(_hour >= 24){
    _hour -= 24;
  }
  while(_minut >= 60){
    _hour += 1;
    _minut -= 60;
  }
  while(_hour < 0){
    _hour += 24;
  }
  while(_minut < 0){
    _hour -= 1;
    _minut += 60;
  }
}
