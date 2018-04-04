/*
  GreenhouseLib_FF.cpp
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
#include "GreenhouseLib_FF.h"

//****************************************************
//******************FAN FUNCTIONS************************
//****************************************************************/
unsigned short Fan::_EEPROMindex = 0;
unsigned short Fan::_counter = 0;

Fan::Fan(){
    _localIndex = FAN_INDEX + _EEPROMindex;
    _EEPROMindex += 8;
    _localCounter = _counter;
    _counter++;

    mod.setLimits(0, 10);
    mod.setAddress(_localIndex);
    _localIndex += sizeof(float);

    hyst.setLimits(0,5);
    hyst.setAddress(_localIndex);
    _localIndex += sizeof(float);

    _routine = true;
    EEPROMTimer = 0;
    overrideTimer = 0;
}


Fan::~Fan(){}

void Fan::initOutput(byte relayType, byte pin){

  _pin = pin;

  //set action linked to digitalWrite state, according to relay type
 _relayType = relayType;

 if (_relayType == ACT_HIGH){
   _activate = HIGH;
   _desactivate = LOW;
 }
 else if (_relayType == ACT_LOW){
   _activate = LOW;
   _desactivate = HIGH;
 }

 //initial state
  #ifdef IOS_OUTPUTS
    pinMode(pin, OUTPUT);
    digitalWrite(pin, _desactivate);
  #endif

  #ifdef MCP_I2C_OUTPUTS
    mcp.pinMode(pin, OUTPUT);
    mcp.digitalWrite(pin, _desactivate);
  #endif

 }

//action functions

/*
Start or stop the fan when a certain temperature is reached
Adjust to an external target temperature (Mode VAR_TEMP)
*/
void Fan::routine(float target, float temp){
  if(_routine == true){
      float activationTemp = target + mod.value();
      if (temp < (activationTemp - hyst.value())) {
        	stop();
      }
      else if (temp > activationTemp) {
        	start();
      }
  }
  else if(_fixOverride == true){
    watchFixOverride();
  }
}
void Fan::routine(boolean condition, float target, float temp){
  if(_routine == true){
      float activationTemp = target + mod.value();
      if (temp < (activationTemp - hyst.value())) {
        	stop();
      }
      else if (temp > activationTemp) {
        	start();
      }
  }
  else if(_fixOverride == true){
    watchFixOverride();
  }
  else if(_relativeOverride == true){
    watchRelativeOverride(condition);
  }
}

//FIXED OVERRIDE ACTION TRIGGER
void Fan::forceAction(unsigned short duration, boolean state){
  if(_routine == true){
    desactivateRoutine();
    _fixOverride = true;
    _overrideDuration = (unsigned long)duration*1000;
    overrideTimer = 0;
    if(state == true){
      start();
    }
    else if(state == false){
      stop();
    }
  }
}

//RELATIVE OVERRIDE ACTION TRIGGER
void Fan::forceAction(boolean state){
  if(_routine == true){
    desactivateRoutine();
    _relativeOverride = true;
    if(state == true){
      start();
    }
    else if(state == false){
      stop();
    }
  }
}

//FIX OVERRIDE WATCH
void Fan::watchFixOverride(){
  if (overrideTimer > _overrideDuration){
    activateRoutine();
    _fixOverride = false;
  }
}

void Fan::watchRelativeOverride(boolean condition){
  if(condition == false){
    activateRoutine();
    _relativeOverride = false;
  }
}



void Fan::stop(){
  #ifdef IOS_OUTPUTS

  if(digitalRead(_pin) == _activate){
    digitalWrite(_pin, _desactivate);
  }
  #endif

  #ifdef MCP_I2C_OUTPUTS

  if(mcp.digitalRead(_pin) == _activate){
    mcp.digitalWrite(_pin, _desactivate);
  }
  #endif
  #ifdef DEBUG_FAN
    Serial.println(F("-------------"));
    Serial.println(F("Stop fan"));
    Serial.println(F("-------------"));
  #endif
}

void Fan::start(){
    #ifdef IOS_OUTPUTS

      if(digitalRead(_pin) == _desactivate){
        digitalWrite(_pin, _activate);
      }
    #endif

    #ifdef MCP_I2C_OUTPUTS

      if(mcp.digitalRead(_pin) == _desactivate){
        mcp.digitalWrite(_pin, _activate);
      }
    #endif
    #ifdef DEBUG_FAN
      Serial.println(F("-------------"));
      Serial.println(F("Start fan"));
      Serial.println(F("-------------"));
    #endif
  }
/*
Activate or desactivate the routine function
*/
void Fan::desactivateRoutine(){
  _routine = false;
}
void Fan::activateRoutine(){
  _routine = true;
}

//programmation functions

void Fan::setParameters(float modif, float hysteresis){
  hyst.setValue(hysteresis);
  mod.setValue(modif);
}
/*
Or one by one...
*/

byte Fan::pin(){
  return _pin;
}
unsigned short Fan::nb(){
  return _localCounter;
}
boolean Fan::override(){
  if(_routine == true){
    return false;
  }
  else{
    return true;
  }
}
boolean Fan::isActive(){
  #ifdef IOS_OUTPUTS
  if(digitalRead(_pin) == _activate){
    return true;
  }
  else{
    return false;
  }
  #endif

  #ifdef MCP_I2C_OUTPUTS

  if(mcp.digitalRead(_pin) == _activate){
    return true;
  }
  else{
    return false;
  }
  #endif
}

void Fan::EEPROMPut(){
  hyst.loadInEEPROM();
  mod.loadInEEPROM();
}

void Fan::EEPROMGet(){

  float hysteresis;
  EEPROM.get(hyst.address(), hysteresis);
  hyst.setValue(hysteresis);

  float modif;
  EEPROM.get(mod.address(), modif);
  mod.setValue(modif);

  #ifdef DEBUG_EEPROM
    Serial.println(F("-------------------"));
    Serial.print(F("--------FAN "));
    Serial.print(_localCounter);
    Serial.println(F("--------"));
    Serial.print(F("Address: "));
    Serial.print(hyst.address());
    Serial.print(F(" - Value :"));
    Serial.print(hyst.value());
    Serial.println(F(" - (Hysteresis)"));
    Serial.print(F("Address: "));
    Serial.print(mod.address());
    Serial.print(F(" - Value :"));
    Serial.print(mod.value());
    Serial.println(F("   - (Mod)"));
  #endif
}

//****************************************************************
//******************HEATER FUNCTIONS************************
//****************************************************************
unsigned short Heater::_EEPROMindex = 0;
unsigned short Heater::_counter = 0;

Heater::Heater(){
    _localIndex = HEATER_INDEX + _EEPROMindex;
    _EEPROMindex += 8;
    _localCounter = _counter;
    _counter++;

    mod.setLimits(-10, 0);
    mod.setAddress(_localIndex);
    _localIndex += sizeof(float);

    hyst.setLimits(0,5);
    hyst.setAddress(_localIndex);
    _localIndex += sizeof(float);

    _routine = true;
    EEPROMTimer = 0;
    overrideTimer = 0;
}

Heater::~Heater(){}

void Heater::initOutput(byte relayType, byte pin){
  _pin = pin;

 //set action linked to digitalWrite state, according to relay type
 _relayType = relayType;
 if (_relayType == ACT_HIGH){
   _activate = HIGH;
   _desactivate = LOW;
 }
 else if (_relayType == ACT_LOW){
   _activate = LOW;
   _desactivate = HIGH;
 }

 //initial state
  #ifdef IOS_OUTPUTS
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, _desactivate);
  #endif

  #ifdef MCP_I2C_OUTPUTS
    mcp.pinMode(_pin, OUTPUT);
    mcp.digitalWrite(_pin, _desactivate);
  #endif

 }

//action functions
/*
Start or stop the heater when a certain temperature is reached
Adjust to an external target temperature (Mode VAR_TEMP)
*/
void Heater::routine(float target, float temp){
  	 if(_routine == true){
      float activationTemp = target + mod.value();
      if (temp > (activationTemp + hyst.value())) {
        	stop();
      } else if (temp < activationTemp) {
        	start();
      }
    }
    else if(_fixOverride == true){
      watchFixOverride();
    }
  }

  void Heater::routine(boolean condition, float target, float temp){
  	 if(_routine == true){
      float activationTemp = target + mod.value();
      if (temp > (activationTemp + hyst.value())) {
        	stop();
      } else if (temp < activationTemp) {
        	start();
      }
    }
    else if(_fixOverride == true){
      watchFixOverride();
    }
    else if(_relativeOverride == true){
      watchRelativeOverride(condition);
    }
  }

  //FIXED OVERRIDE ACTION TRIGGER
  void Heater::forceAction(unsigned short duration, boolean state){
    if(_routine == true){
      desactivateRoutine();
      _fixOverride = true;
      _overrideDuration = (unsigned long)duration*1000;
      overrideTimer = 0;
      if(state == true){
        start();
      }
      else if(state == false){
        stop();
      }
    }
  }

  //RELATIVE OVERRIDE ACTION TRIGGER
  void Heater::forceAction(boolean state){
    if(_routine == true){
      desactivateRoutine();
      _relativeOverride = true;
      if(state == true){
        start();
      }
      else if(state == false){
        stop();
      }
    }
  }

  //FIX OVERRIDE WATCH
  void Heater::watchFixOverride(){
    if (overrideTimer > _overrideDuration){
      activateRoutine();
      _fixOverride = false;
    }
  }

  void Heater::watchRelativeOverride(boolean condition){
    if(condition == false){
      activateRoutine();
      _relativeOverride = false;
    }
  }

void Heater::stop(){
  #ifdef IOS_OUTPUTS
    if(digitalRead(_pin) == HIGH){
      digitalWrite(_pin, _desactivate);
    }
  #endif
  #ifdef MCP_I2C_OUTPUTS
    if(mcp.digitalRead(_pin) == _activate){
      mcp.digitalWrite(_pin, _desactivate);
    }
  #endif

  #ifdef DEBUG_HEATER
    Serial.println(F("-------------"));
    Serial.println(F("Stop heater"));
    Serial.println(F("-------------"));
  #endif
  }

void Heater::start(){
  #ifdef IOS_OUTPUTS
    if(digitalRead(_pin) == _desactivate){
      digitalWrite(_pin, _activate);
    }
  #endif
  #ifdef MCP_I2C_OUTPUTS
    if(mcp.digitalRead(_pin) == _desactivate){
      mcp.digitalWrite(_pin, _activate);
    }
  #endif
  #ifdef DEBUG_HEATER
    Serial.println(F("-------------"));
    Serial.println(F("Start heater"));
    Serial.println(F("-------------"));
  #endif
  }

/*
Activate or desactivate the routine function
*/
void Heater::desactivateRoutine(){
  _routine = false;
}
void Heater::activateRoutine(){
  _routine = true;
}

//programmation functions


void Heater::setParameters(float modif, float hysteresis){
  hyst.setValue(hysteresis);
  mod.setValue(modif);
}
/*
Or one by one...
*/

byte Heater::pin(){
  return _pin;
}
unsigned short Heater::nb(){
  return _localCounter;
}
boolean Heater::override(){
  if(_routine == true){
    return false;
  }
  else{
    return true;
  }
}

boolean Heater::isActive(){
  #ifdef IOS_OUTPUTS
  if(digitalRead(_pin) == _activate){
    return true;
  }
  else{
    return false;
  }
  #endif

  #ifdef MCP_I2C_OUTPUTS

  if(mcp.digitalRead(_pin) == _activate){
    return true;
  }
  else{
    return false;
  }
  #endif
}
void Heater::EEPROMPut(){
  hyst.loadInEEPROM();
  mod.loadInEEPROM();
}
void Heater::EEPROMGet(){

  float hysteresis;
  EEPROM.get(hyst.address(), hysteresis);
  hyst.setValue(hysteresis);

  float modif;
  EEPROM.get(mod.address(), modif);
  mod.setValue(modif);

  #ifdef DEBUG_EEPROM
    Serial.println(F("-------------------"));
    Serial.print(F("-------HEATER "));
    Serial.print(_localCounter);
    Serial.println(F("------"));
    Serial.print(F("Address: "));
    Serial.print(hyst.address());
    Serial.print(F(" - Value :"));
    Serial.print(hyst.value());
    Serial.println(F(" - (Hysteresis)"));
    Serial.print(F("Address: "));
    Serial.print(mod.address());
    Serial.print(F(" - Value :"));
    Serial.print(mod.value());
    Serial.println(F("   - (Mod)"));
  #endif
}
