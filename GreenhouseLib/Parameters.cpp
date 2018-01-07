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

void floatParameter::setValue(float value){
//If parameter's value is between minimum and maximum values, change it, else, report error
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

void floatParameter::minimum(float minimum){
  _minimum = minimum;
}
void floatParameter::maximum(float maximum){
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

void shortParameter::setValue(short value){
//If parameter's value is between minimum and maximum values, change it, else, report error
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

void shortParameter::minimum(short minimum){
  _minimum = minimum;
}
void shortParameter::maximum(short maximum){
  _maximum = maximum;
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

void uShortParameter::setValue(unsigned short value){
//If parameter's value is between minimum and maximum values, change it, else, report error
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

void uShortParameter::minimum(unsigned short minimum){
  _minimum = minimum;
}
void uShortParameter::maximum(unsigned short maximum){
  _maximum = maximum;
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

void byteParameter::setValue(byte value){
//If parameter's value is between minimum and maximum values, change it, else, report error
  if((value >= _minimum)&&(value <= _maximum)){
    _value = value;
  }
  else{
    _value = 333;
  }
}

void byteParameter::setLastValue(byte lastValue){
  if((lastValue >= _minimum)&&(lastValue <= _maximum)){
    _lastValue = lastValue;
  }
  else{
    _lastValue = 333;
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

void byteParameter::minimum(byte minimum){
  _minimum = minimum;
}
void byteParameter::maximum(byte maximum){
  _maximum = maximum;
}

byte byteParameter::value(){
  return _value;
}

byte byteParameter::lastValue(){
  return _lastValue;
}


//****************************************************************
//*******************TOOLBOX**************************************
//****************************************************************

byte negativeToByte(int data, byte mod){
  return data+mod;
}

int byteToNegative(int data, byte mod){
  return data-mod;
}
//Programme pour convertir l'addition de nombres décimales en format horaire
void convertDecimalToTime(int * heure, int * minut){
  //Serial.println(m);
  if ((*minut > 59) && (*minut < 120)){
    *heure += 1;
    *minut -= 60;
  }
  else if ((*minut < 0) && (*minut >= -60)){
    *heure -= 1;
    *minut +=60;
  }
}
