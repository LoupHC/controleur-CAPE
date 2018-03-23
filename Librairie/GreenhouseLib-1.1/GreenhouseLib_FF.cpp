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

    _mod.setLimits(0, 10);
    _mod.setAddress(_localIndex);
    _localIndex += sizeof(float);

    _hyst.setLimits(0,5);
    _hyst.setAddress(_localIndex);
    _localIndex += sizeof(float);

    _debug = false;
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
      float activationTemp = target + _mod.value();
      if (temp < (activationTemp - _hyst.value())) {
        	stop();
      }
      else if (temp > activationTemp) {
        	start();
      }
  }
}

void Fan::forceAction(unsigned short duration, boolean state){
  if(_routine == true){
    desactivateRoutine();
    overrideTimer = 0;
    if(state == true){
      start();
    }
    else if(state == false){
      stop();
    }
  }
  if (overrideTimer > (unsigned long)duration*1000){
    activateRoutine();
  }
}

void Fan::forceAction(boolean condition, boolean state){
  if((_routine == true)&&(condition == true)){
    desactivateRoutine();
    if(state == true){
      start();
    }
    else if(state == false){
      stop();
    }
  }
  if(condition == false){
    activateRoutine();
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

void Fan::setParameters(float temp, float hyst, boolean safety){
  setHyst(hyst);
  setMod(temp);
  setSafety(safety);
}
/*
Or one by one...
*/
void Fan::setHyst(float hyst){
  _hyst.setValue(hyst);
  EEPROM.put(_hyst.address(),hyst);
}

void Fan::setMod(float temp){
  _mod.setValue(temp);
  EEPROM.put(_mod.address(),temp);
}

void Fan::setSafety(boolean safety){
  _safety = safety;
}
byte Fan::pin(){
  return _pin;
}
float Fan::hyst(){
  return _hyst.value();
}
float Fan::hystMin(){
  return _hyst.minimum();
}
float Fan::hystMax(){
  return _hyst.maximum();
}
float Fan::mod(){
  return _mod.value();
}
float Fan::modMin(){
  return _mod.minimum();
}
float Fan::modMax(){
  return _mod.maximum();
}
boolean Fan::safety(){
  return _safety;
}
boolean Fan::debug(){
  return _debug;
}
unsigned short Fan::nb(){
  return _localCounter;
}


void Fan::loadEEPROMParameters(){

  float hyst;
  EEPROM.get(_hyst.address(), hyst);
  setHyst(hyst);

  float mod;
  EEPROM.get(_mod.address(), mod);
  setMod(mod);

  #ifdef DEBUG_EEPROM
    Serial.println(F("-------------------"));
    Serial.print(F("--------FAN "));
    Serial.print(_localCounter);
    Serial.println(F("--------"));
    Serial.print(F("Address: "));
    Serial.print(_hyst.address());
    Serial.print(F(" - Value :"));
    Serial.print(hyst);
    Serial.println(F(" - (Hysteresis)"));
    Serial.print(F("Address: "));
    Serial.print(_mod.address());
    Serial.print(F(" - Value :"));
    Serial.print(mod);
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

    _mod.setLimits(-10, 0);
    _mod.setAddress(_localIndex);
    _localIndex += sizeof(float);

    _hyst.setLimits(0,5);
    _hyst.setAddress(_localIndex);
    _localIndex += sizeof(float);


	  _debug = false;
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
      float activationTemp = target + _mod.value();
      if (temp > (activationTemp + _hyst.value())) {
        	stop();
      } else if (temp < activationTemp) {
        	start();
      }
    }
  }

  void Heater::forceAction(unsigned short duration, boolean state){
    if(_routine == true){
      desactivateRoutine();
      overrideTimer = 0;
      if(state == true){
        start();
      }
      else if(state == false){
        stop();
      }
    }
    if (overrideTimer > (unsigned long)duration*1000){
      activateRoutine();
    }
  }

  void Heater::forceAction(boolean condition, boolean state){
    if((_routine == true)&&(condition == true)){
      desactivateRoutine();
      if(state == true){
        start();
      }
      else if(state == false){
        stop();
      }
    }
    if(condition == false){
      activateRoutine();
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


void Heater::setParameters(float temp, float hyst, boolean safety){
  setHyst(hyst);
  setMod(temp);
  setSafety(safety);
}
/*
Or one by one...
*/
void Heater::setHyst(float hyst){
  _hyst.setValue(hyst);
  EEPROM.put(_hyst.address(),hyst);
}

void Heater::setMod(float temp){
  _mod.setValue(temp);
  EEPROM.put(_mod.address(),temp);
}

void Heater::setSafety(boolean safety){
  _safety = safety;
}

byte Heater::pin(){
  return _pin;
}
float Heater::hyst(){
  return _hyst.value();
}
float Heater::hystMin(){
  return _hyst.minimum();
}
float Heater::hystMax(){
  return _hyst.maximum();
}
float Heater::mod(){
  return _mod.value();
}
float Heater::modMin(){
  return _mod.minimum();
}
float Heater::modMax(){
  return _mod.maximum();
}
boolean Heater::safety(){
  return _safety;
}
boolean Heater::debug(){
  return _debug;
}
unsigned short Heater::nb(){
  return _localCounter;
}

void Heater::loadEEPROMParameters(){

  float hyst;
  EEPROM.get(_hyst.address(), hyst);
  setHyst(hyst);

  float mod;
  EEPROM.get(_mod.address(), mod);
  setMod(mod);

  #ifdef DEBUG_EEPROM
    Serial.println(F("-------------------"));
    Serial.print(F("-------HEATER "));
    Serial.print(_localCounter);
    Serial.println(F("------"));
    Serial.print(F("Address: "));
    Serial.print(_hyst.address());
    Serial.print(F(" - Value :"));
    Serial.print(hyst);
    Serial.println(F(" - (Hysteresis)"));
    Serial.print(F("Address: "));
    Serial.print(_mod.address());
    Serial.print(F(" - Value :"));
    Serial.print(mod);
    Serial.println(F("   - (Mod)"));
  #endif
}
