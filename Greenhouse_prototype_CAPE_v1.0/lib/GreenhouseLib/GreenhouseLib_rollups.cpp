/*
  GreenhouseLib_rollups.cpp
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
#include "GreenhouseLib_rollups.h"

unsigned short Stage::_counter = 0;

Stage::Stage(){
  _localCounter = _counter;
  _counter++;
  if(_counter == MAX_STAGES){
    _counter = 0;
  }
}
Stage::~Stage(){}

unsigned short Stage::nb(){
  return _localCounter;
}
//****************************************************************
//*******************ROLLUP FUNCTIONS*****************************
//****************************************************************



/*
Constructor : Define Opening and Closing pins on the arduino, to connect to the relay box
_routine (normal logic for the rollup applies if true)
_closing (closing pin is active if true)
_opening (same with opening pin)
_closingCycle (a closing cycle is going on, including pause time)
_closingCycle (same with closing)
_incrementCounter (what increment the rollup has reached)
incrementCounter is set to OFF_VAL so the program recognize the first opening/closing run.
_stage (what cooling stage the rollup has reached)
_localIndex (starting point for EEPROM savings)
_index (starting point for next object of the same class)
EEPROMTimer (checks every 10 seconds if EEPROM has to be updated)

*/

unsigned short Rollup::_EEPROMindex = 0;
unsigned short Rollup::_counter = 0;

Rollup::Rollup(){

  _localIndex = ROLLUP_INDEX + _EEPROMindex;
  _EEPROMindex += 50;
  _localCounter = _counter;
  _counter++;


  _routine = true;
  _fixOverride = false;
  _relativeOverride = false;
  _routineCycle = false;

  _closing = false;
  _opening = false;

  hyst.setLimits(0, 5);
  hyst.setAddress(_localIndex);
  _localIndex += sizeof(float);

  rotationUp.setLimits(0,500);
  rotationUp.setAddress(_localIndex);
  _localIndex += sizeof(unsigned short);

  rotationDown.setLimits(0,500);
  rotationDown.setAddress(_localIndex);
  _localIndex += sizeof(unsigned short);

  pause.setLimits(1,240);
  pause.setAddress(_localIndex);
  _localIndex += sizeof(unsigned short);
  _increments = 100;
  _stages = MAX_STAGES;

  for(int x = 0; x < MAX_STAGES; x++){
    stage[x].mod.setLimits(0,5);
    stage[x].mod.setAddress(_localIndex);
    _localIndex += sizeof(float);
    stage[x].target.setLimits(0,100);
    stage[x].target.setAddress(_localIndex);
    _localIndex += sizeof(unsigned short);
  }
  #ifdef TEST_MODE
  _incrementCounter = 0;
  _stage = 0;
  #endif
  #ifndef TEST_MODE
  _incrementCounter = OFF_VAL;
  _stage = OFF_VAL;
  #endif

  debugTimer = 0;
  rollupTimer = 0;
}

/*
Destructor
*/
Rollup::~Rollup(){}
/*
INIT OUTPUTS
Relay type :
- ACT_HIGH : relay is active when pin is high
- ACT_LOW : relay is active when pin is low
Pins :
- rOpen (pin connected to opening relay)
- rClose (pin connected to closing relay)

*/
void Rollup::initOutputs(boolean relayType, byte rOpen, byte rClose){
  //define opening/closing pins
  _openingPin = rOpen;
  _closingPin = rClose;

  //set actions related to digitalWrite state, according to relay type
  _relayType = relayType;
  if (_relayType == ACT_HIGH){
    _activate = true;
    _desactivate = false;
  }
  else if (_relayType == ACT_LOW){
    _activate = false;
    _desactivate = true;
  }

  //initial state
  #ifdef IOS_OUTPUTS
    pinMode(_openingPin, OUTPUT);
    pinMode(_closingPin, OUTPUT);
    digitalWrite(_openingPin, _desactivate);
    digitalWrite(_closingPin, _desactivate);
  #endif

  #ifdef MCP_I2C_OUTPUTS
    mcp.pinMode(_openingPin, OUTPUT);
    mcp.pinMode(_closingPin, OUTPUT);
    mcp.digitalWrite(_openingPin, _desactivate);
    mcp.digitalWrite(_closingPin, _desactivate);
  #endif

}

void Rollup::initStage(Stage stage, float modif, byte inc){
  stage.mod.setValue(modif);
  stage.target.setValue(inc);
}
/*ROLLUP ROUTINE
  Open or close the rollups to specific increment, using a multiple cooling stages logic
  Adjust to an external target temperature
  (Optional) Turn in override mode when a certain condition is true
*/
void Rollup::routine(float targetTemp, float temp){
    checkTimings();
    if(_routine == true){
      if(_routineCycle == false){
        if(_stage == OFF_VAL){
          calibrateStages();
        }
        else{
          openOrClose(temp, targetTemp);
        }
      }
      else {
        watchRoutine();
      }
    }
    else if(_fixOverride == true){
      watchFixOverride();
    }
  debugPrints();
  }

void Rollup::routine(boolean condition, float targetTemp, float temp){
    checkTimings();
    if(_routine == true){
      if(_routineCycle == false){
        if(_stage == OFF_VAL){
          calibrateStages();
        }
        else{
          openOrClose(temp, targetTemp);
        }
      }
      else {
          watchRoutine();
      }
    }
    else if(_relativeOverride == true){
      watchRelativeOverride(condition);
    }
    else if(_fixOverride == true){
      watchFixOverride();
    }
  debugPrints();
  }

  /*ROUTINE ACTION TRIGGER
    INITIAL CONDITIONS:
    - Motor must not be already moving nor in pause
    ACTION:
    - If temp is higher than treshold : go to next stage up
    - If temp is lower than treshold : go to next stage down
  */
  void Rollup::openOrClose(float temp, float targetTemp){
    if (temp >= (targetTemp + stage[_upperStage].mod.value())){
        if((_opening == false)&&(_closing == false)&&(_routineCycle == false)){
          startMove(_upperStage, stage[_upperStage].target.value());
        }
    }
    else if(temp < (targetTemp + stage[_stage].mod.value() - hyst.value())){
        if((_opening == false)&&(_closing == false)&&(_routineCycle == false)){
          startMove(_lowerStage,  stage[_lowerStage].target.value());
        }
    }
  }



/*OVERRIDES ACTION TRIGGER
    INITIAL CONDITIONS:
    - Motor must not be already moving
    - No other overrides of the same type are active
    ACTION:
    - reach target increment
*/
void Rollup::forceMove(unsigned short targetIncrement){
    if((_opening == false)&&(_closing == false)&&(_relativeOverride == false)){
      startMove("RELATIVE", targetIncrement);
    }
  }
  void Rollup::forceMove(unsigned short duration, unsigned short targetIncrement){
    if((_opening == false)&&(_closing == false)&&(_fixOverride == false)){
      _overrideDuration = (unsigned long)duration*1000;
      startMove("FIX", targetIncrement);
    }
  }
  /*ROUTINE WATCH
  MOVE : until motor reaches target increment
  PAUSE : until end of pause time
  */
  void Rollup::watchRoutine(){
    if((rollupTimer > _moveTime)){
      stopMove();
    }
    if(rollupTimer > (_moveTime + _pauseTime)){
      resumeCycle("ROUTINE");
    }
  }


  /*RELATIVE OVERRIDE WATCH
  MOVE : until motor reaches target increment
  PAUSE : until condition turn false
  */
  void Rollup::watchRelativeOverride(boolean condition){
    if(rollupTimer > _moveTime){
        stopMove();
    }
    if((rollupTimer > _moveTime)&&(condition == false)){
      resumeCycle("RELATIVE");
    }
}

/*RELATIVE OVERRIDE WATCH
MOVE : until motor reaches target increment
PAUSE :for all override's duration
*/
void Rollup::watchFixOverride(){
  if(rollupTimer > _moveTime){
      stopMove();
  }
  if((rollupTimer > _moveTime)&&(rollupTimer > _overrideDuration)){
    resumeCycle("FIX");
  }
}
/*ROUTINE MOVING LOGIC
    INITIAL CONDITIONS:
    -increment counter is calibrated
    - theres a difference between actual increment and target increment
    ACTION:
    - calculate time of the move
    - start moving
    - set moving timer to 0
*/
void Rollup::startMove(short targetStage, short targetIncrement){
  if(_incrementCounter == OFF_VAL){
    return;
  }
  _incrementMove = targetIncrement - (short)_incrementCounter;
  _stageMove = targetStage - _stage;

  //If motor goes up, calculate for how much time
  if (_incrementMove > 0){
    _routineCycle = true;
    _moveTime = _upStep * (unsigned long)(abs(_incrementMove));
    startOpening();
  }
  //If motor goes down, calculate for how much time
  else if (_incrementMove < 0){
    _routineCycle = true;
    _moveTime = _downStep * (unsigned long)(abs(_incrementMove));
    startClosing();
  }
  rollupTimer = 0;
}

/*OVERRIDE MOVING LOGIC
    INITIAL CONDITIONS:
    -increment counter is calibrated
    ACTION:
    - desactivate routine
    - If theres a difference between actual increment and target increment:
      - calculate time of the move
      - start moving
      - set moving timer to 0
*/
void Rollup::startMove(String type, short targetIncrement){
  if(_incrementCounter == OFF_VAL){
    return;
  }
  _incrementMove = targetIncrement - (short)_incrementCounter;
  if(type == "FIX"){
    desactivateRoutine("FIX");
  }
  if(type == "RELATIVE"){
    desactivateRoutine("RELATIVE");
  }
  if (_incrementMove > 0){
    startOpening();
    _moveTime = _upStep * (unsigned long)(abs(_incrementMove));
  }
  else if (_incrementMove < 0){
    startClosing();
    _moveTime = _downStep * (unsigned long)(abs(_incrementMove));
  }
  rollupTimer = 0;
}

/*STOP MOVING
    INITIAL CONDITION:
    -increment counter is calibrated
    ACTION:
    - stop moving
    - set new value for incrementCounter
    - reset _incrementMove
*/
void Rollup::stopMove(){
    if(_incrementCounter == OFF_VAL){
      return;
    }
    if(_opening == true){
      stopOpening();
    }
    if(_closing == true){
      stopClosing();
    }
    _incrementCounter = _incrementCounter + _incrementMove;
    _incrementMove = 0;

}
/*RESUME CYCLE
    INITIAL CONDITION:
    -override is active
    OR
    - routine cycle is active
    ACTION:

    ACTION:
    - desactivate override and activate routine
    OR
    - desactivate routine cycle
    - reset times, moving time and stage move
*/
void Rollup::resumeCycle(String type){
  if(type == "FIX"){
    if(_fixOverride == true){
      _fixOverride = false;
      activateRoutine();
    }
  }
  else if(type == "RELATIVE"){
    if(_relativeOverride == true){
      _relativeOverride = false;
      activateRoutine();
    }
  }
  else if(type == "ROUTINE"){
      if((_routine == true)&&(_routineCycle == true)){
      _stage = _stage+_stageMove;
      _routineCycle = false;
    }
  }
  _moveTime = 0;
  _stageMove = 0;
  rollupTimer = 0;
}

/*CALIBRATE STAGE
    INITIAL CONDITION:
    - stage is uncalibrated
    ACTION:
    - if incrementCounter is uncalibrateddesactivate routine cycle
    - reset times, moving time and stage move
*/
void Rollup::calibrateStages(){
  if(_stage == OFF_VAL){
    if(_incrementCounter == OFF_VAL){
      _incrementCounter = _increments;
      forceMove(rotationDown.value(),0);
    }
    else if(_incrementCounter != OFF_VAL){
      //If youre at top increment, consider youre at top stage
        if(_incrementCounter >= stage[_stages].target.value()){
          _stage = _stages;
        }
        //If youre at bottom increment, consider youre at lowest stage
        else if(_incrementCounter == stage[0].target.value()){
          _stage = 0;
        }
        //Anywhere between first and second stage increment, consider youre in the second stage
        else if((_incrementCounter > stage[0].target.value())&&(_incrementCounter <= stage[1].target.value())){
          _stage = 1;
        }
        //Between second stage and third, consider youre at second stage;
        //Between third stage and fourth, considier youre at third stage;
        //etc.
        else{
          for(byte x = 1; x < _stages;x++){
            if((_incrementCounter >= stage[x].target.value())&& (_incrementCounter < stage[x+1].target.value())){
              _stage = x;
            }
          }
        }
      }
      checkStageSuccession();
    }
  }

/*
Activate or desactivate the routine function (before overriding)
*/
void Rollup::desactivateRoutine(String type){
  if((_opening == false)&&(_closing == false)){
    _routineCycle = false;
    _routine = false;
    _stageMove = OFF_VAL;
    _stage = OFF_VAL;

    if(type == "FIX"){
        _fixOverride = true;
    }
    else if(type == "RELATIVE"){
        _relativeOverride = true;
    }
  }
}

void Rollup::activateRoutine(){
  if(_fixOverride == true){
      _overrideDuration = 0;
      _fixOverride = false;
  }
  else if(_relativeOverride == true){
      _relativeOverride = false;
  }
  if((_opening == false)&&(_closing == false)){
    _routine = true;
  }
}

/*
Activate or desactivate the opening or closing relay
*/

void Rollup::action(byte pin, boolean state){
  #ifdef IOS_OUTPUTS
    if(state == _activate){
      digitalWrite(pin, _activate);
    }
    else if (state == _desactivate){
      digitalWrite(pin, _desactivate);
    }
  #endif

  #ifdef MCP_I2C_OUTPUTS
    if(state == _activate){
      mcp.digitalWrite(pin, HIGH);
    }
    else if (state == _desactivate){
      mcp.digitalWrite(pin, LOW);
    }
  #endif
}


void Rollup::startOpening(){
	if(_closing == false){
		_opening = true;
    action(_openingPin, _activate);
	}
}

void Rollup::startClosing(){
	if(_opening == false){
		_closing = true;
    action(_closingPin, _activate);
	}
}

void Rollup::stopOpening(){
	if(_opening == true){
		_opening = false;
    action(_openingPin, _desactivate);
	}
}

void Rollup::stopClosing(){
	if(_closing == true){
		_closing = false;
    action(_closingPin, _desactivate);
	}
}

/*
Program all parameters all at once...
*/
void Rollup::setParameters(float rHyst, unsigned short rotUp, unsigned short rotDown, unsigned short paus){
  hyst.setValue(rHyst);
  rotationUp.setValue(rotUp);
  rotationDown.setValue(rotDown);
  pause.setValue(paus);
}
/*
Define
*/
void Rollup::setStages(byte stages){
  _stages = stages;
  if(_stages > MAX_STAGES){
    _stages = MAX_STAGES;
  }
}

void Rollup::checkTimings(){
  checkStageSuccession();
  float upStep = (float)rotationUp.value()*1000/(float)_increments;
  float downStep = (float)rotationDown.value()*1000/(float)_increments;
  _upStep = upStep;
  _downStep = downStep;
  _pauseTime = pause.value()*1000;
}

void Rollup::checkStageSuccession(){
  if(_stage != _stages){
    _upperStage = _stage+1;
  }
  else{
    _upperStage = _stages;
  }
  if(_stage != 0){
    _lowerStage = _stage-1;
  }
  else{
    _lowerStage = 0;
  }
}

void Rollup::setIncrementCounter(unsigned short increment){
  _incrementCounter = increment;
}

void Rollup::EEPROMPut(){
  pause.loadInEEPROM();
  rotationUp.loadInEEPROM();
  rotationDown.loadInEEPROM();
  hyst.loadInEEPROM();
  for(unsigned short x = 0; x < _stages+1; x++){
    stage[x].mod.loadInEEPROM();
    stage[x].target.loadInEEPROM();
  }
}
void Rollup::EEPROMGet(){

  float hysteresis;
  EEPROM.get(hyst.address(), hysteresis);
  hyst.setValue(hysteresis);

  unsigned short rotUp;
  EEPROM.get(rotationUp.address(), rotUp);
  rotationUp.setValue(rotUp);

  unsigned short rotDown;
  EEPROM.get(rotationDown.address(), rotDown);
  rotationDown.setValue(rotDown);

  unsigned short paus;
  EEPROM.get(pause.address(), paus);
  pause.setValue(paus);

  #ifdef DEBUG_EEPROM
    Serial.println(F("-------------------"));
    Serial.print(F("-------ROLLUP "));
    Serial.print(_localCounter);
    Serial.println(F("------"));
    Serial.print(F("Address: "));
    Serial.print(hyst.address());
    Serial.print(F(" - Value :"));
    Serial.print(hyst.value());
    Serial.println(F(" - (Hysteresis)"));
    Serial.print(F("Address: "));
    Serial.print(rotationUp.address());
    Serial.print(F(" - Value :"));
    Serial.print(rotationUp.value());
    Serial.println(F("   - (Rotation up)"));
    Serial.print(F("Address: "));
    Serial.print(rotationDown.address());
    Serial.print(F(" - Value :"));
    Serial.print(rotationDown.value());
    Serial.println(F("   - (Rotation down)"));
    Serial.print(F("Address: "));
    Serial.print(pause.address());
    Serial.print(F(" - Value :"));
    Serial.print(pause.value());
    Serial.println(F("   - (Pause)"));
  #endif

  for(unsigned short x = 0; x < _stages+1; x++){
    float modif;
    EEPROM.get(stage[x].mod.address(), modif);
    stage[x].mod.setValue(modif);

    unsigned short targ;
    EEPROM.get(stage[x].target.address(), targ);
    stage[x].target.setValue(targ);

    #ifdef DEBUG_EEPROM
      Serial.print(F("-------STAGE  "));
      Serial.print(stage[x].nb());
      Serial.println(F("------"));
      Serial.print(F("Address: "));
      Serial.print(stage[x].mod.address());
      Serial.print(F(" - Value :"));
      Serial.print(stage[x].mod.value());
      Serial.print(F(" - (Stage mod "));
      Serial.print(x);
      Serial.println(F(")"));
      Serial.print(F("Address: "));
      Serial.print(stage[x].target.address());
      Serial.print(F(" - Value :"));
      Serial.print(stage[x].target.value());
      Serial.print(F("   - (Stage target "));
      Serial.print(x);
      Serial.println(F(")"));
    #endif
  }
}
//Return private variables

unsigned short Rollup::increments(){
  return _increments;
}

unsigned short Rollup::incrementCounter(){
  return _incrementCounter;
}
boolean Rollup::opening(){
  return _opening;
}
boolean Rollup::closing(){
  return _closing;
}
boolean Rollup::override(){
  if((_relativeOverride == true)||(_fixOverride == true)){
    return true;
  }
  else{
    return false;
  }
}
boolean Rollup::isMoving(){
  if ((_opening == true)||(_closing == true)){
    return true;
  }
  else{
    return false;
  }
}
boolean Rollup::isWaiting(){
    if (((_opening == false)||(_closing == false))&&(_routineCycle == true)){
      return true;
    }
    else{
      return false;
    }
}

unsigned short Rollup::nb(){
  return _localCounter;
}

void Rollup::debugPrints(){
  if(debugTimer > 1000){
    if(_localCounter == 0 ){
      #ifdef DEBUG_ROLLUP1_CYCLE
        Serial.println(F("---------"));
        if(_relativeOverride == true){
          Serial.println(F("RELATIVE OVERRIDE : ON"));
        }
        if(_fixOverride == true){
          Serial.println(F("FIX OVERRIDE : ON"));
        }
        if(_routineCycle == true){
          Serial.println(F("ROUTINE CYCLE : ON"));
        }
      #endif
      #ifdef DEBUG_ROLLUP1_POSITION
          Serial.println(F("---------"));
          Serial.print(F("up stage : "));
          Serial.println(_upperStage);
          Serial.print(F("up target : "));
          Serial.println(stage[_upperStage].target.value());
          Serial.print(F("Stage : "));
          Serial.println(_stage);
          Serial.print(F("low stage : "));
          Serial.println(_lowerStage);
          Serial.print(F("low target : "));
          Serial.println(stage[_lowerStage].target.value());
      #endif
      #ifdef DEBUG_ROLLUP1_TIMING
        Serial.println(F("---------"));
        if(_routineCycle == true){
          Serial.println(F("ROUTINE1"));
          Serial.print(F("rollupTimer : "));
          Serial.println(rollupTimer);
          Serial.print(F("moveTime : "));
          Serial.println(_moveTime);
          Serial.print(F("pauseTime : "));
          Serial.println(_moveTime + _pauseTime);
        }
        else if(_fixOverride == true){
          Serial.println(F("FIX OVERRIDE1"));
          Serial.print(F("rollupTimer : "));
          Serial.println(rollupTimer);
          Serial.print(F("moveTime : "));
          Serial.println(_moveTime);
          Serial.print(F("duration : "));
          Serial.println(_overrideDuration);
        }
        else if(_relativeOverride == true){
          Serial.println(F("RELATIVE OVERRIDE1"));
          Serial.print(F("rollupTimer : "));
          Serial.println(rollupTimer);
          Serial.print(F("moveTime : "));
          Serial.println(_moveTime);
        }
      #endif
    }
  if(_localCounter == 1 ){
    #ifdef DEBUG_ROLLUP2_CYCLE
      Serial.println(F("---------"));
      if(_relativeOverride == true){
        Serial.println(F("RELATIVE OVERRIDE : ON"));
      }
      if(_fixOverride == true){
        Serial.println(F("FIX OVERRIDE : ON"));
      }
      if(_relativeOverride == true){
        Serial.println(F("ROUTINE CYCLE : ON"));
      }
    #endif

    #ifdef DEBUG_ROLLUP2_POSITION
      Serial.println(F("---------"));
      Serial.print(F("up stage : "));
      Serial.println(_upperStage);
      Serial.print(F("up target : "));
      Serial.println(stage[_upperStage].target.value());
      Serial.print(F("Stage : "));
      Serial.println(_stage);
      Serial.print(F("low stage : "));
      Serial.println(_lowerStage);
      Serial.print(F("low target : "));
      Serial.println(stage[_lowerStage].target.value());
    #endif
    #ifdef DEBUG_ROLLUP2_TIMING
      Serial.println(F("---------"));
      if(_routineCycle == true){
        Serial.println(F("ROUTINE1"));
        Serial.print(F("rollupTimer : "));
        Serial.println(rollupTimer);
        Serial.print(F("moveTime : "));
        Serial.println(_moveTime);
        Serial.print(F("pauseTime : "));
        Serial.println(_moveTime + _pauseTime);
      }
      else if(_fixOverride == true){
        Serial.println(F("FIX OVERRIDE1"));
        Serial.print(F("rollupTimer : "));
        Serial.println(rollupTimer);
        Serial.print(F("moveTime : "));
        Serial.println(_moveTime);
        Serial.print(F("duration : "));
        Serial.println(_overrideDuration);
      }
      else if(_relativeOverride == true){
        Serial.println(F("RELATIVE OVERRIDE1"));
        Serial.print(F("rollupTimer : "));
        Serial.println(rollupTimer);
        Serial.print(F("moveTime : "));
        Serial.println(_moveTime);
      }
    #endif
    }
    debugTimer = 0;
  }
}
