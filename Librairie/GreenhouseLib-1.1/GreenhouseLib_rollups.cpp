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

void Stage::setStage(float mod, unsigned short target){
  setMod(mod);
  setTarget(target);
}

void Stage::setModLimits(float minimum, float maximum){
  _mod.setLimits(minimum, maximum);
}
void Stage::setModAddress(int address){
  _mod.setAddress(address);
}

void Stage::setTargetLimits(unsigned short minimum, unsigned short maximum){
  _target.setLimits(minimum, maximum);
}
void Stage::setTargetAddress(int address){
  _target.setAddress(address);
}

void Stage::setMod(float mod){
  _mod.setValue(mod);
}
void Stage::setTarget(unsigned short target){
  _target.setValue(target);
}
float Stage::mod(){
  return _mod.value();
}
float Stage::modMin(){
  return _mod.minimum();
}
float Stage::modMax(){
  return _mod.maximum();
}
unsigned short Stage::modAddress(){
  return _mod.address();
}
unsigned short Stage::target(){
  return _target.value();
}
unsigned short Stage::targetMin(){
  return _target.minimum();
}
unsigned short Stage::targetMax(){
  return _target.maximum();
}
unsigned short Stage::targetAddress(){
  return _target.address();
}
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
  _closing = false;
  _opening = false;
  _closingCycle = false;
  _openingCycle = false;
  _safetyCycle = false;

  _hyst.setLimits(0, 5);
  _hyst.setAddress(_localIndex);
  _localIndex += sizeof(float);

  _rotationUp.setLimits(0,500);
  _rotationUp.setAddress(_localIndex);
  _localIndex += sizeof(unsigned short);

  _rotationDown.setLimits(0,500);
  _rotationDown.setAddress(_localIndex);
  _localIndex += sizeof(unsigned short);

  _pause.setLimits(0,240);
  _pause.setAddress(_localIndex);
  _localIndex += sizeof(unsigned short);

  _stages = MAX_STAGES;

  for(int x = 0; x < MAX_STAGES; x++){
    stage[x].setModLimits(0,5);
    stage[x].setModAddress(_localIndex);
    _localIndex += sizeof(float);
    stage[x].setTargetLimits(0,100);
    stage[x].setTargetAddress(_localIndex);
    _localIndex += sizeof(unsigned short);
  }

  _incrementCounter = OFF_VAL;
  _stage = OFF_VAL;

  EEPROMTimer = 0;
  rollupTimer = 0;
}

/*
Destructor
*/
Rollup::~Rollup(){}
/*
INIT OUTPUTS
- MAN_TEMP : target temperature is set outisde the class and differ for each cooling stage
relayType:
- ACT_HIGH : relay is active when pin is high
- ACT_LOW : relay is active when pin is low
rOpen (pin connected to opening relay)
rClose (pin connected to closing relay)
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
    #ifdef DEBUG_SETUP
      //Serial.println("Relay is active_high...");
    #endif
  }
  else if (_relayType == ACT_LOW){
    _activate = false;
    _desactivate = true;
    #ifdef DEBUG_SETUP
      //Serial.println("Relay is active_low...");
    #endif
  }

  //initial state
  #ifdef IOS_OUTPUTS
    pinMode(_openingPin, OUTPUT);
    pinMode(_closingPin, OUTPUT);
    digitalWrite(_openingPin, _desactivate);
    digitalWrite(_closingPin, _desactivate);
    //Serial.println("Outputs are regular I/Os...");
  #endif

  #ifdef MCP_I2C_OUTPUTS
    mcp.pinMode(_openingPin, OUTPUT);
    mcp.pinMode(_closingPin, OUTPUT);
    mcp.digitalWrite(_openingPin, _desactivate);
    mcp.digitalWrite(_closingPin, _desactivate);
    //Serial.println("Outputs are I2C I/Os from MCP23008...");
  #endif

}

void Rollup::initStage(Stage stage, float mod, byte inc){
  stage.setMod(mod);
  stage.setTarget(inc);
}
/*
Open or close the rollups to specific increment, using a multiple cooling stages logic
Adjust to an external target temperature (Mode MAN_TEMP)
*/


void Rollup::manualRoutine(float targetTemp, float temp){
    if(_routine == true){
      switch(_stage){
        case OFF_VAL:
          calibrateStages();
          break;
        case 0:
          openOrClose(0, temp, targetTemp);
          break;
        case 1:
          openOrClose(1, temp, targetTemp);
          break;
        case 2:
          openOrClose(2, temp, targetTemp);
          break;
        case 3:
          openOrClose(3, temp, targetTemp);
          break;
        case 4:
          openOrClose(4, temp, targetTemp);
          break;
      }
    //safetyCycle();
    }
    else{
      if(_stage == OFF_VAL){
        calibrateStages();
      }
    }
  }
  void Rollup::calibrateStages(){
    if(_stage == OFF_VAL){
      if(_incrementCounter == OFF_VAL){
        #ifdef TEST_MODE
        _incrementCounter = 0;
        #endif
        #ifndef TEST_MODE
        forceMove(_rotationDown.value(), 0);
        #endif
      }
      else{
        //If youre at bottom increment, consider youre at lowest stage
        if(_incrementCounter == stage[0].target()){
          _stage = 0;
        }
        //Anywhere between first and second stage increment, consider youre in the second stage
        else if((_incrementCounter > stage[0].target())&&(_incrementCounter <= stage[1].target())){
          _stage = 1;
        }
        //Between second stage and third, consider youre at second stage;
        //Between third stage and fourth, considier youre at third stage;
        //etc.
        else{
          for(byte x = 1; x < _stages+1;x++){
            if((_incrementCounter> stage[x].target())&& (_incrementCounter <= stage[x+1].target())){
              _stage = x;
            }
          }
        }
      }

    }
  }
  void Rollup::openOrClose(unsigned short actualStage, float temp, float targetTemp){
    //Set next stage coming up or down after current stage
    if(actualStage != _stages){_upperStage = actualStage+1;}
    else{_upperStage = _stages;}
    if(actualStage != 0){_lowerStage = actualStage-1;}
    else{_lowerStage = 0;
    }
    //Temp is higher than treshold : go to next stage up
    if((temp >= (targetTemp + stage[_upperStage].mod()))||(_openingCycle == true)){
        openToInc(_upperStage, stage[_upperStage].target());
    }
    //Temp is lower than treshold : go to next stage down
    else if((temp < (targetTemp + stage[actualStage].mod() - _hyst.value()))||(_closingCycle == true)){
        openToInc(_lowerStage, stage[_lowerStage].target());
    }
  }

  void Rollup::openToInc(unsigned short targetStage, unsigned short targetIncrement){

      //INITIAL CONDITIONS
      //Motor must not be already moving or in pause
      if((_opening == false)&&(_closing == false)&&(_openingCycle == false)&&(_closingCycle == false)){
        startMove(targetStage, targetIncrement);
      }
      //MOVE
      //Until motor reaches the target increment
      if((rollupTimer > _moveTime)){
        stopMove();
      }
      //PAUSE
      //To allow air exchange
      //Back to routine
      if(rollupTimer > (_moveTime + _pauseTime)){
        resumeCycle();
      }
            #ifdef DEBUG_ROLLUP_TIMING
              Serial.print("rollupTimer : ");
              Serial.println(rollupTimer);
              Serial.print("moveTime : ");
              Serial.println(_moveTime);
              Serial.print("pauseTime : ");
              Serial.println(_moveTime + _pauseTime);
            #endif
  }



void Rollup::forceMove(boolean condition, unsigned short targetIncrement){

    //INITIAL CONDITIONS
    //Condition must be true (could be sensor, button...)
    //Motor must not be already moving
    if((_opening == false)&&(_closing == false)&&(_override == false)&&(condition == true)){
        startMove(OFF_VAL, targetIncrement);
    }
    //MOVE
    //Until motor reaches targetIncrement
    if((_routine == false)&&(rollupTimer > _moveTime)){
        stopMove();
    }
    //PAUSE
    //Until condition turn false
    if((_override == true)&&(rollupTimer > _moveTime)&&(condition == false)){
      resumeCycle();
    }
    #ifdef DEBUG_ROLLUP_TIMING
      Serial.print("rollupTimer : ");
      Serial.println(rollupTimer);
      Serial.print("moveTime : ");
      Serial.println(_moveTime);
      Serial.print("condition : ");
      Serial.println(condition);
    #endif
}

void Rollup::forceMove(unsigned short duration, unsigned short targetIncrement){

  //INITIAL CONDITIONS
  //Motor must not be already moving
  if((_opening == false)&&(_closing == false)&&(_override == false)){
      startMove(OFF_VAL, targetIncrement);
  }
  //MOVE
  //Until motor reaches targetIncrement
  if((_routine == false)&&(rollupTimer > _moveTime)){
      stopMove();
  }
  //PAUSE
  //Until end of override cycle
  //Motor must have reached target
  //And back to routine
  if((_override == true)&&(rollupTimer > _moveTime)&&(rollupTimer > (unsigned long)duration*1000)){
    resumeCycle();
  }
  #ifdef DEBUG_ROLLUP_TIMING
    Serial.print("rollupTimer : ");
    Serial.println(rollupTimer);
    Serial.print("moveTime : ");
    Serial.println(_moveTime);
    Serial.print("duration : ");
    Serial.println(duration*1000);
  #endif
}
void Rollup::startMove(short stageMove, short targetIncrement){
  //Set difference between actual and target increment
  if(_incrementCounter == OFF_VAL){
    _incrementMove = targetIncrement - 100;
  }
  else {
    _incrementMove = targetIncrement - _incrementCounter;
  }
  //If theres a difference...
  if (_incrementMove != 0){
    //...resetTimer...
    rollupTimer = 0;
    //If target stage is unknown, thats because youre executing an override
    if(stageMove == OFF_VAL){
      _stageMove = OFF_VAL;
      desactivateRoutine();
      _override = true;
    }
    //Otherwise set difference between actual and target stage
    else{
      _stageMove = stageMove - _stage;
    }
    //If motor goes up, calculate for how much time
    if (_incrementMove > 0){
      startOpening();
      _moveTime = _upStep * abs(_incrementMove);
      if(_routine == true){
        _openingCycle = true;
      }
    }
    //If motor goes down, calculate for how much time
    if (_incrementMove < 0){
      startClosing();
      _moveTime = _downStep * abs(_incrementMove);
      if(_routine == true){
        _closingCycle = true;
      }
    }
  }
  //If thers no move, keep counter down to zero
  else{rollupTimer = 0;}
}

void Rollup::stopMove(){
    if(_incrementCounter == OFF_VAL){
      _incrementCounter = 0;
      _incrementMove = 0;
    }
    else {
      _incrementCounter = _incrementCounter + _incrementMove;
      _incrementMove = 0;
    }
    if(_opening == true){
      stopOpening();
    }
    if(_closing == true){
      stopClosing();
    }
}

void Rollup::resumeCycle(){
  if(_override == true){
    _override = false;
    _stage = OFF_VAL;
    activateRoutine();
  }
  else{
    if(_openingCycle == true){
      _openingCycle = false;
      _stage = _stage+_stageMove;
      _stageMove = 0;
      printEndPause();
    }
    if(_closingCycle == true){
      _closingCycle = false;
      _stage = _stage+_stageMove;
      _stageMove = 0;
      printEndPause();
    }
  }

}

void Rollup::safetyCycle(){
  //Make sure rollup is fully open or fully close when controller think it is (following _incrementCounter)
  if((_safetyCycle == false)&&((_incrementCounter == _increments) || (_incrementCounter == 0))){
    _safetyCycle = true;
    safetyTimer = 0;

    if(safetyTimer >= SAFETY_DELAY){

      if((_opening == false)&&(_closing == false)){
        if(_incrementCounter == _increments){
          startMove(OFF_VAL, _increments);
        }
        if(_incrementCounter == 0){
          short move = 0 - _increments;
          startMove(OFF_VAL, move);
        }
      }
    }
    if(safetyTimer >= SAFETY_DELAY + _moveTime){
        if((_opening == false)&&(_closing == false)){
          if(_incrementCounter == _increments){
            startMove(OFF_VAL, _increments);
            resumeCycle();
          }
          if(_incrementCounter == 0){
            startMove(OFF_VAL, 0);
            resumeCycle();
          }
        }
        _safetyCycle = false;
      }
    }
  }

/*
Activate or desactivate the routine function (before overriding)
*/
void Rollup::desactivateRoutine(){
  if((_opening == false)&&(_closing == false)){
    _closingCycle = false;
    _openingCycle = false;
    _routine = false;

  }
}
void Rollup::activateRoutine(){
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

    #ifdef DEBUG_ROLLUP_TIMING
      Serial.println(F("-------------"));
      Serial.println(F("opening"));
      Serial.println(F("-------------"));
    #endif
	}
}

void Rollup::startClosing(){
	if(_opening == false){
		_closing = true;
    action(_closingPin, _activate);

    #ifdef DEBUG_ROLLUP_TIMING
      Serial.println(F("-------------"));
      Serial.println(F("closing"));
      Serial.println(F("-------------"));
    #endif
	}
}

void Rollup::stopOpening(){
	if(_opening == true){
		_opening = false;
    action(_openingPin, _desactivate);

    #ifdef DEBUG_ROLLUP_TIMING
      Serial.println(F("-------------"));
      Serial.println(F("stop opening"));
      Serial.println(F("-------------"));
    #endif
	}
}

void Rollup::stopClosing(){
	if(_closing == true){
		_closing = false;
    action(_closingPin, _desactivate);

    #ifdef DEBUG_ROLLUP_TIMING
      Serial.println(F("-------------"));
      Serial.println(F("stop closing"));
      Serial.println(F("-------------"));
    #endif
	}
}

void Rollup::printPause(){
  #ifdef DEBUG_ROLLUP_TIMING
    Serial.println(F("-------------"));
    Serial.println(F("start pause"));
    Serial.println(F("-------------"));
  #endif
}
void Rollup::printEndPause(){
  #ifdef DEBUG_ROLLUP_TIMING
    Serial.println(F("-------------"));
    Serial.println(F("end of the pause"));
    Serial.println(F("-------------"));
  #endif
}
/*
Program all parameters all at once...
*/
void Rollup::setParameters(float rHyst, unsigned short rotationUp, unsigned short rotationDown, unsigned short increments,unsigned short pause, boolean safety){
  setHyst(rHyst);
  setRotationUp(rotationUp);
  setRotationDown(rotationDown);
  setIncrements(increments);
  setPause(pause);
  setSafety(safety);
}
/*
Or one by one...
*/
void Rollup::setStages(byte stages){
  _stages = stages;
  if(_stages > MAX_STAGES){
    _stages = MAX_STAGES;
  }
}

void Rollup::setHyst(float rHyst){
  _hyst.setValue(rHyst);
  EEPROM.put(_hyst.address(),rHyst);
}
void Rollup::setRotationUp(unsigned short rotationUp){
  _rotationUp.setValue(rotationUp);
  EEPROM.put(_rotationUp.address(),rotationUp);
  _upStep = (float)_rotationUp.value()*1000/(float)_increments;
}

void Rollup::setRotationDown(unsigned short rotationDown){
  _rotationDown.setValue(rotationDown);
  EEPROM.put(_rotationDown.address(),rotationDown);
  _downStep = (float)_rotationDown.value()*1000/(float)_increments;
}

void Rollup::setIncrements(unsigned short increments){
  _increments = increments;
  _upStep = (float)_rotationUp.value()*1000/(float)_increments;
  _downStep = (float)_rotationDown.value()*1000/(float)_increments;
}

void Rollup::setPause(unsigned short pause){
  _pause.setValue(pause);
  EEPROM.put(_pause.address(),pause);
  unsigned long pauseTime = _pause.value();
  _pauseTime = pauseTime*1000;
}

void Rollup::setSafety(boolean safety){
  _safety = safety;
}

void Rollup::setIncrementCounter(unsigned short increment){
  _incrementCounter = increment;
}

void Rollup::setStageParameters(byte number, float mod, byte target){
    setStageMod(number,mod);
    setStageTarget(number,target);
}

void Rollup::setStageMod(byte number, float value){
  stage[number].setMod(value);
  int address = stage[number].modAddress();
  EEPROM.put(address, value);
}
void Rollup::setStageTarget(byte number, unsigned short value){
  stage[number].setTarget(value);
  int address = stage[number].targetAddress();
  EEPROM.put(address, value);

}
void Rollup::loadEEPROMParameters(){

  float hyst;
  EEPROM.get(_hyst.address(), hyst);
  setHyst(hyst);

  unsigned short rotationUp;
  EEPROM.get(_rotationUp.address(), rotationUp);
  setRotationUp(rotationUp);

  unsigned short rotationDown;
  EEPROM.get(_rotationDown.address(), rotationDown);
  setRotationDown(rotationDown);

  unsigned short pause;
  EEPROM.get(_pause.address(), pause);
  setPause(pause);

  #ifdef DEBUG_EEPROM
    Serial.println(F("-------------------"));
    Serial.print(F("-------ROLLUP "));
    Serial.print(_localCounter);
    Serial.println(F("------"));
    Serial.print(F("Address: "));
    Serial.print(_hyst.address());
    Serial.print(F(" - Value :"));
    Serial.print(hyst);
    Serial.println(F(" - (Hysteresis)"));
    Serial.print(F("Address: "));
    Serial.print(_rotationUp.address());
    Serial.print(F(" - Value :"));
    Serial.print(rotationUp);
    Serial.println(F("   - (Rotation up)"));
    Serial.print(F("Address: "));
    Serial.print(_rotationDown.address());
    Serial.print(F(" - Value :"));
    Serial.print(rotationDown);
    Serial.println(F("   - (Rotation down)"));
    Serial.print(F("Address: "));
    Serial.print(_pause.address());
    Serial.print(F(" - Value :"));
    Serial.print(pause);
    Serial.println(F("   - (Pause)"));
  #endif

  for(int x = 0; x < _stages; x++){
    float mod;
    EEPROM.get(stage[x].modAddress(), mod);
    stage[x].setMod(mod);

    unsigned short target;
    EEPROM.get(stage[x].targetAddress(), target);
    stage[x].setTarget(target);
    #ifdef DEBUG_EEPROM
      Serial.print(F("-------STAGE  "));
      Serial.print(stage[x].nb());
      Serial.println(F("------"));
      Serial.print(F("Address: "));
      Serial.print(stage[x].modAddress());
      Serial.print(F(" - Value :"));
      Serial.print(mod);
      Serial.print(F(" - (Stage mod "));
      Serial.print(x);
      Serial.println(F(")"));
      Serial.print(F("Address: "));
      Serial.print(stage[x].targetAddress());
      Serial.print(F(" - Value :"));
      Serial.print(target);
      Serial.print(F("   - (Stage target "));
      Serial.print(x);
      Serial.println(F(")"));
    #endif
  }
}
//Return private variables

float Rollup::hyst(){
  return _hyst.value();
}
float Rollup::hystMin(){
  return _hyst.minimum();
}
float Rollup::hystMax(){
  return _hyst.maximum();
}
unsigned short Rollup::rotationUp(){
  return _rotationUp.value();
}
unsigned short Rollup::rotationUpMin(){
  return _rotationUp.minimum();
}
unsigned short Rollup::rotationUpMax(){
  return _rotationUp.maximum();
}
unsigned short Rollup::rotationDown(){
  return _rotationDown.value();
}
unsigned short Rollup::rotationDownMin(){
  return _rotationDown.minimum();
}
unsigned short Rollup::rotationDownMax(){
  return _rotationDown.maximum();
}
unsigned short Rollup::pause(){
  return _pause.value();
}
unsigned short Rollup::pauseMin(){
  return _pause.minimum();
}
unsigned short Rollup::pauseMax(){
  return _pause.maximum();
}
unsigned short Rollup::increments(){
  return _increments;
}
boolean Rollup::safety(){
  return _safety;
}
float Rollup::stageMod(byte number){
  return stage[number].mod();
}
float Rollup::stageModMin(byte number){
  return stage[number].modMin();
}
float Rollup::stageModMax(byte number){
  return stage[number].modMax();
}
unsigned short Rollup::stageTarget(byte number){
  return stage[number].target();
}
unsigned short Rollup::stageTargetMin(byte number){
  return stage[number].targetMin();
}
unsigned short Rollup::stageTargetMax(byte number){
  return stage[number].targetMax();
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
boolean Rollup::openingCycle(){
  return _openingCycle;
}
boolean Rollup::closingCycle(){
  return _closingCycle;
}
unsigned short Rollup::nb(){
  return _localCounter;
}
