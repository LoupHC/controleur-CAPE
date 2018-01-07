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


Stage::Stage(){

}
Stage::~Stage(){}

void Stage::setStage(float mod, byte target){
  setMod(mod);
  setTarget(target);
}

void Stage::setModLimits(float minimum, float maximum){
  _mod.setLimits(minimum, maximum);
}

void Stage::setTargetLimits(byte minimum, byte maximum){
  _target.setLimits(minimum, maximum);
}

void Stage::setMod(float mod){
  _mod.setValue(mod);
}
void Stage::setTarget(byte target){
  _target.setValue(target);
}
float Stage::mod(){
  return _mod.value();
}
byte Stage::target(){
  return _target.value();
}
//****************************************************************
//*******************ROLLUP FUNCTIONS*****************************
//****************************************************************

static unsigned short Rollup::_index = 0;

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
Rollup::Rollup(){
  _routine = true;
  _closing = false;
  _opening = false;
  _closingCycle = false;
  _openingCycle = false;
  _safetyCycle = false;

  _hyst.setLimits(0, 5);
  _tempParameter.setLimits(-5, 50);
  _rotationUp.setLimits(0,500);
  _rotationDown.setLimits(0,500);
  _pause.setLimits(0,240);
  _increments.setLimits(0,100);
  _stages = MAX_STAGES;

  for(int x = 0; x < 4; x++){
    stage[x].setModLimits(0,5);
    stage[x].setTargetLimits(0,100);
  }
  _incrementCounter = OFF_VAL;
  _stage = OFF_VAL;
  _localIndex = ROLLUP_INDEX + _index;
  _index += 7;
  EEPROMTimer = 0;
}

/*
Destructor
*/
Rollup::~Rollup(){}
/*
INIT OUTPUTS
mode:
- FIX_TEMP : target temperature is set within the class
- VAR_TEMP : target temperature is set outside the class
- MAN_TEMP : target temperature is set outisde the class and differ for each cooling stage
relayType:
- ACT_HIGH : relay is active when pin is high
- ACT_LOW : relay is active when pin is low
rOpen (pin connected to opening relay)
rClose (pin connected to closing relay)
*/
void Rollup::initRollup(byte stages){

    _routine = true;
    _closing = false;
    _opening = false;
    _closingCycle = false;
    _openingCycle = false;
    _safetyCycle = false;

    _hyst.setLimits(0, 5);
    _tempParameter.setLimits(-5, 50);
    _rotationUp.setLimits(0,500);
    _rotationDown.setLimits(0,500);
    _pause.setLimits(0,240);
    _increments.setLimits(0,100);
    _stages = stages;

    for(int x = 0; x < 4; x++){
      stage[x].setModLimits(0,5);
      stage[x].setTargetLimits(0,100);
    }
    _incrementCounter = OFF_VAL;
    _stage = OFF_VAL;
    _localIndex = ROLLUP_INDEX + _index;
    _index += 7;
    EEPROMTimer = 0;
}
void Rollup::initOutputs(byte mode, boolean relayType, byte rOpen, byte rClose){
  //set minimum/maximum values for tempParameter
  _mode = mode;

  if (_mode == FIX_TEMP){
    _tempParameter.minimum(0);
    _tempParameter.maximum(50);
  }
  else if(_mode == VAR_TEMP){
    _tempParameter.minimum(-5);
    _tempParameter.maximum(10);
  }
  else if(_mode == MAN_TEMP){
    _tempParameter.minimum(0);
    _tempParameter.maximum(0);
  }
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

  #ifdef MCP_I2C_OUTPUTS;
    mcp.pinMode(_openingPin, OUTPUT);
    mcp.pinMode(_closingPin, OUTPUT);
    mcp.digitalWrite(_openingPin, LOW);
    mcp.digitalWrite(_closingPin, LOW);
  #endif

}

void Rollup::initStage(Stage stage, float mod, byte inc){
  stage.setMod(mod);
  stage.setTarget(inc);
}
/*
Open or close the rollups by increments when a certain temperature is reached
Adjust to the internal target temperature (Mode FIX_TEMP)
*/
void Rollup::routine(float temp){
  if (_mode == FIX_TEMP){
  	 if(_routine == true){
     #ifdef DEBUG_ROLLUP_TEMP
       Serial.println(F("------------"));
       Serial.print(F("Temperature :  "));
       Serial.println(temp);
       Serial.print(F("Opening temp : "));
       Serial.println(_tempParameter.value());
       Serial.print(F("Closing temp : "));
       Serial.println(_tempParameter.value()-_hyst.value());
       Serial.println(F("-------------"));
     #endif
      if ((temp < (_tempParameter.value() - _hyst.value()))||(_closingCycle == true)) {
        	closingSides();
      } else if ((temp > _tempParameter.value())||(_openingCycle == true)) {
        	openSides();
      }
    }
    safetyCycle();
  }
}
/*
Open or close the rollups by increments when a certain temperature is reached
Adjust to an external target temperature (Mode VAR_TEMP)
*/
void Rollup::routine(float target, float temp){
  if (_mode == VAR_TEMP){
  	 if(_routine == true){
      float activationTemp = target + _tempParameter.value();
      #ifdef DEBUG_ROLLUP_TEMP
        Serial.println(F("------------"));
        Serial.print(F("Temperature :  "));
        Serial.println(temp);
        Serial.print(F("Opening temp : "));
        Serial.println(target+_tempParameter.value());
        Serial.print(F("Closing temp : "));
        Serial.println(target+_tempParameter.value()-_hyst.value());
        Serial.println(F("-------------"));
      #endif

      if ((temp < (activationTemp - _hyst.value()))||(_closingCycle == true)) {
        	closingSides();
      } else if ((temp > activationTemp)||(_openingCycle == true)) {
        	openSides();
      }
    }
    safetyCycle();
  }
}
/*
Open or close the rollups to specific increment, using a maltiple cooling stages logic
Adjust to an external target temperature (Mode MAN_TEMP)
*/
void Rollup::manualRoutine(float target, float temp){
  if (_mode == MAN_TEMP){
  	if(_routine == true){
      float targetTemp = target + _tempParameter.value();

      #ifdef DEBUG_ROLLUP_TEMP
        Serial.println(F("------------"));
        Serial.print(F("Temperature :  "));
        Serial.println(temp);
        Serial.print(F("Stage 1 : Opening temp : "));
        Serial.println(targetTemp + stage[0].mod());
        Serial.print(F("Stage 1 : Closing temp : "));
        Serial.println(targetTemp + stage[0].mod() - _hyst.value());
        Serial.print(F("Stage 2 : Opening temp : "));
        Serial.println(targetTemp + stage[1].mod());
        Serial.print(F("Stage 2 : Closing temp : "));
        Serial.println(targetTemp + stage[1].mod() - _hyst.value());
        Serial.print(F("Stage 3 : Opening temp : "));
        Serial.println(targetTemp + stage[2].mod());
        Serial.print(F("Stage 3 : Closing temp : "));
        Serial.println(targetTemp + stage[2].mod() - _hyst.value());
        Serial.print(F("Stage 4 : Opening temp : "));
        Serial.println(targetTemp + stage[3].mod());
        Serial.print(F("Stage 4 : Closing temp : "));
        Serial.println(targetTemp + stage[3].mod() - _hyst.value());
        Serial.println(F("-------------"));
      #endif

      if (((temp >= targetTemp + stage[3].mod())&&(_stage == OFF_VAL))||(_stage == 4)){
        if (_stages >= 4){
          openToInc(4, stage[3].target());
        }
      }
      else if (((temp >= targetTemp + stage[2].mod())&&(_stage == OFF_VAL))||(_stage == 3)){
        if (_stages >= 3){
          openToInc(3, stage[2].target());
        }
      }
      else if (((temp >= targetTemp + stage[1].mod())&&(_stage == OFF_VAL))||(_stage == 2)){
        if (_stages >= 2){
          openToInc(2, stage[1].target());
        }
      }
      else if (((temp >= targetTemp + stage[0].mod())&&(_stage == OFF_VAL))||(_stage == 1)){
        if (_stages >= 1){
          openToInc(1, stage[0].target());
        }
      }

      if (((temp < targetTemp + stage[0].mod() - _hyst.value())&&(_stage == OFF_VAL))||(_stage == -1)){
        if (_stages >= 1){
          closeToInc(-1, 0);
        }
      }
      else if (((temp < targetTemp + stage[1].mod() - _hyst.value())&&(_stage == OFF_VAL))||(_stage == -2)){
        if (_stages >= 2){
          closeToInc(-2, stage[0].target());
        }
      }
      else if (((temp < targetTemp + stage[2].mod() - _hyst.value())&&(_stage == OFF_VAL))||(_stage == -3)){
        if (_stages >= 3){
          closeToInc(-3, stage[1].target());
        }
      }
      else if (((temp < targetTemp + stage[3].mod() - _hyst.value())&&(_stage == OFF_VAL))||(_stage == -4)){
        if (_stages >= 4){
          closeToInc(-4, stage[2].target());
        }
      }
    }
    safetyCycle();
      }
}


/*
Open to reach a specific increment
*/

void Rollup::openToInc(short stage, byte targetIncrement){

  if (_incrementCounter == OFF_VAL){
    _incrementCounter = 0;
  }
  if ((_opening == false)&&(_openingCycle == false)){
    _move = targetIncrement - _incrementCounter;

    if (_move > 0){
      rollupTimer = 0;
      _openingCycle = true;
      _stage = stage;
      startOpening();
    }
  }
  if (_move > 0){
    if(_openingCycle == true){

      #ifdef DEBUG_ROLLUP_TIMING
          Serial.println(F("-------------"));
          Serial.println(F("OPENING CYCLE"));
          Serial.print(F("Increment :     "));
          Serial.println(_incrementCounter);
          Serial.print(F("Target increment :     "));
          Serial.println(targetIncrement);
        	Serial.print(F("Timer :         "));
        	Serial.println(rollupTimer);
        	Serial.print(F("Rotation time : "));
        	Serial.println((unsigned long)_rotationUp.value()*1000/_increments.value()*_move);
        	Serial.print(F("Pause time :    "));
        	Serial.println((unsigned long)_rotationUp.value()*1000/_increments.value()*_move+_pause.value()*1000);
            Serial.println(F("-------------"));
      #endif

      if (rollupTimer >= ((unsigned long)_rotationUp.value()*1000/_increments.value()*_move)){
        if(_opening == true){
          _incrementCounter = _incrementCounter + _move;
          stopOpening();
          printPause();
        }
      }
      if (rollupTimer >= ((unsigned long)_rotationUp.value()*1000/_increments.value()*_move + (unsigned long)_pause.value()*1000)){
        _openingCycle = false;
        _stage = OFF_VAL;
        printEndPause();
      }
    }
  }
}
/*
Close to reach a specific increment
*/
void Rollup::closeToInc(short stage, byte targetIncrement){

  if (_incrementCounter == OFF_VAL){
    _incrementCounter = 0;
  }

  if ((_closing == false)&&(_closingCycle == false)){
      _move = (short)targetIncrement - (short)_incrementCounter;
      if (_move < 0){
        rollupTimer = 0;
        _closingCycle = true;
        _stage = stage;
        startClosing();
      }
  }
  if (_move < 0){
    if(_closingCycle == true){

      #ifdef DEBUG_ROLLUP_TIMING
        Serial.println(F("-------------"));
        Serial.println(F("CLOSING CYCLE"));
        Serial.print(F("Increment :     "));
        Serial.println(_incrementCounter);
        Serial.print(F("Target increment :     "));
        Serial.println(targetIncrement);
      	Serial.print(F("Timer :         "));
      	Serial.println(rollupTimer);
      	Serial.print(F("Rotation time : "));
      	Serial.println((unsigned long)_rotationDown.value()*1000/_increments.value()*(0-_move));
      	Serial.print(F("Pause time :    "));
      	Serial.println((unsigned long)_rotationDown.value()*1000/_increments.value()*(0-_move)+(unsigned long)_pause.value()*1000);
        Serial.println(F("-------------"));
      #endif

      if (rollupTimer >= ((unsigned long)_rotationDown.value()*1000/_increments.value()*(0-_move))){
        if(_closing == true){
          _incrementCounter = _incrementCounter + _move;
          stopClosing();
          printPause();
        }
      }
      if (rollupTimer >= ((unsigned long)_rotationDown.value()*1000/_increments.value()*(0-_move) + (unsigned long)_pause.value()*1000)){
        _closingCycle = false;
        _stage = OFF_VAL;
        printEndPause();
      }
    }
  }
}

/*
Safety cycle :
safety closing or safety closing every X milliseconds (SAFETY_DELAY) to ensure rollup is effectively shut or fully open
*/

void Rollup::safetyCycle(){

  if(_safety == true){
        if(_incrementCounter == _increments.value()){
      safetyOpen();

    }
    else if (_incrementCounter == 0){
      safetyClose();
    }
  }
}

void Rollup::safetyOpen(){
  if ((_safetyCycle == false)||(_stage != OFF_VAL)){
    _safetyCycle = true;
    safetyTimer = 0;
  }
  if(safetyTimer >= SAFETY_DELAY){
    if(_opening == false){
      desactivateRoutine();
      startOpening();
    }
  }
  if(safetyTimer >= SAFETY_DELAY+_rotationUp.value()*1000){
    if(_opening == true){
      stopOpening();
      activateRoutine();
      _safetyCycle = false;
    }
  }
}

void Rollup::safetyClose(){
  if ((_safetyCycle == false)||(_stage != OFF_VAL)){
    _safetyCycle = true;
    safetyTimer = 0;
  }
  if(safetyTimer >= SAFETY_DELAY){
    if(_closing == false){
      desactivateRoutine();
      startClosing();
    }
  }
  if(safetyTimer >= SAFETY_DELAY+_rotationDown.value()*1000){
    if(_closing == true){
      stopClosing();
      activateRoutine();
      _safetyCycle = false;
    }
  }

}
/*
Activate or desactivate the routine function (can then be activated manually)
*/
void Rollup::desactivateRoutine(){
  _routine = false;
}
void Rollup::activateRoutine(){
  _routine = true;
}

/*
Activate or desactivate the opening or closing relay
*/

void Rollup::action(byte pin, boolean state){
  #ifdef IOS_OUTPUTS
    if(state == ON){
      digitalWrite(pin, _activate);
    }
    else if (state == OFF){
      digitalWrite(pin, _desactivate);
    }
  #endif

  #ifdef MCP_I2C_OUTPUTS
    if(state == ON){
      mcp.digitalWrite(pin, HIGH);
    }
    else if (state == OFF){
      mcp.digitalWrite(pin, LOW);
    }
  #endif
}


void Rollup::startOpening(){
	if(_closing == false){
		_opening = true;
    action(_openingPin, ON);

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
    action(_closingPin, ON);

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
    action(_openingPin, OFF);

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
    action(_closingPin, OFF);

    #ifdef DEBUG_ROLLUP_TIMING
      Serial.println(F("-------------"));
      Serial.println(F("stop closing"));
      Serial.println(F("-------------"));
    #endif
	}
}
/*
Open or close the rollups by one increment
*/
void Rollup::openSides(){
	#ifdef DEBUG_ROLLUP_TIMING
  if(_openingCycle == true){
    Serial.println(F("-------------"));
    Serial.println(F("OPENING CYCLE"));
  	Serial.print(F("Timer :         "));
  	Serial.println(rollupTimer);
  	Serial.print(F("Rotation time : "));
  	Serial.println(((unsigned long)_rotationUp.value()*1000/_increments.value()));
  	Serial.print(F("Pause time :    "));
  	Serial.println(((unsigned long)_rotationUp.value()*1000/_increments.value()+(unsigned long)_pause.value()*1000));
    Serial.print(F("Increment :     "));
    Serial.println(_incrementCounter);
    Serial.println(F("-------------"));
  }
	#endif

  if (_incrementCounter == OFF_VAL){
    _incrementCounter = 0;
  }

  if ((_incrementCounter < _increments.value())||(_openingCycle == true)) {
    if ((_opening == false) && (_openingCycle == false)){		//Si le rollup était précédemment inactif et qu'aucune cycle d'ouverture n'est en cours
    	rollupTimer = 0;														//On initie le compteur
    	_openingCycle = true;												//Un cycle est en cours
    	startOpening();														//On active le relais d'ouverture
    }
  }

  if(_openingCycle == true){
    if(rollupTimer >= (((unsigned long)_rotationUp.value()*1000/_increments.value()))){					//Si le compteur atteint le temps d'ouverture/le nombre d'incréments...
      if(_opening == true){
        _incrementCounter += 1;												//L'incrément augmente de +1
        stopOpening();															//On désactive le relais
        printPause();
      }
    }
    if(rollupTimer >= ((unsigned long)_rotationUp.value()*1000/_increments.value() + (unsigned long)_pause.value()*1000)){		//Après le temps de pause...
    	_openingCycle = false;												//On peut recommencer le cycle
      printEndPause();
    }
  }
}


void Rollup::closingSides(){
	#ifdef DEBUG_ROLLUP_TIMING
  if(_closingCycle == true){
    Serial.println(F("-------------"));
    Serial.println(F("ClOSING CYCLE"));
  	Serial.print(F("Timer :         "));
  	Serial.println(rollupTimer);
  	Serial.print(F("Rotation time : "));
  	Serial.println(((unsigned long)_rotationDown.value()*1000/_increments.value()));
  	Serial.print(F("Pause time : "));
  	Serial.println(((unsigned long)_rotationDown.value()*1000/_increments.value()+(unsigned long)_pause.value()*1000));
    Serial.print(F("Increment :       "));
    Serial.println(_incrementCounter);
    Serial.println(F("-------------"));
  }
  #endif
  if (_incrementCounter == OFF_VAL){
    _incrementCounter = _increments.value();
  }

  if (_incrementCounter > 0) {
    if ((_closing == false) && (_closingCycle == false)){		//Si le rollup était précédemment inactif et qu'aucune cycle d'ouverture n'est en cours
    	rollupTimer = 0;														//On initie le compteur
    	_closingCycle = true;												//Un cycle est en cours
    	startClosing();														//On active le relais d'ouverture
    }
  }
  if(_closingCycle == true){
    if(rollupTimer >= ((unsigned long)_rotationDown.value()*1000/_increments.value())){					//Si le compteur atteint le temps d'ouverture/le nombre d'incréments...
      if(_closing == true){
        _incrementCounter -= 1;												//L'incrément augmente de +1
        stopClosing();															//On désactive le relais
        printPause();
      }
    }
    if(rollupTimer >= ((unsigned long)_rotationDown.value()*1000/_increments.value() + (unsigned long)_pause.value()*1000)){		//Après le temps de pause...
    	_closingCycle = false;												//On peut recommencer le cycle
      printEndPause();
    }
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
void Rollup::setParameters(float rTemp, float rHyst, unsigned short rotationUp, unsigned short rotationDown, unsigned short increments,unsigned short pause, boolean safety){
  setTemp(rTemp);
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
void Rollup::setTemp(float temp){
  _tempParameter.setValue(temp);
}

void Rollup::setHyst(float rHyst){
  _hyst.setValue(rHyst);
}
void Rollup::setRotationUp(unsigned short rotationUp){
  _rotationUp.setValue(rotationUp);
}

void Rollup::setRotationDown(unsigned short rotationDown){
  _rotationDown.setValue(rotationDown);
}

void Rollup::setIncrements(unsigned short increments){
  _increments.setValue(increments);
}

void Rollup::setPause(unsigned short pause){
  _pause.setValue(pause);
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
  stage[number-1].setMod(value);
}
void Rollup::setStageTarget(byte number, byte value){
  stage[number-1].setTarget(value);


}
/*
upload new parameters in EEPROM memory


void Rollup::setParametersInEEPROM(short rTemp, byte rHyst, byte rotationUp, byte rotationDown, byte increments,byte pause, boolean safety){
  setTempInEEPROM(rTemp);
  setHystInEEPROM(rHyst);
  setRotationUpInEEPROM(rotationUp);
  setRotationDownInEEPROM(rotationDown);
  setIncrementsInEEPROM(increments);
  setPauseInEEPROM(pause);
  setSafetyInEEPROM(safety);
}
/*
load last saved parameters

void Rollup::loadEEPROMParameters(){
  setTemp((float)EEPROM.read(_localIndex+TEMP_INDEX));
  setHyst((float)EEPROM.read(_localIndex+HYST_INDEX));
  setRotationUp((unsigned short)EEPROM.read(_localIndex+ROTATION_UP_INDEX));
  setRotationDown((unsigned short)EEPROM.read(_localIndex+ROTATION_DOWN_INDEX));
  setIncrements((unsigned short)EEPROM.read(_localIndex+INCREMENTS_INDEX));
  setPause((unsigned short)EEPROM.read(_localIndex+PAUSE_INDEX));
  setSafety((boolean)EEPROM.read(_localIndex+SAFETY_INDEX));
}

void Rollup::setTempInEEPROM(short temp){
    switch (_mode){
      case FIX_TEMP:
          EEPROM.update(_localIndex+TEMP_INDEX, (byte)temp);
      break;
      case VAR_TEMP:
      case MAN_TEMP:
        //As tempParameter can be between -10 to 10, we add 10 to its value to convert it to "byte"
          byte adjustTemp = (byte)(temp+10);
          EEPROM.update(_localIndex+TEMP_INDEX, adjustTemp);
      break;
    }
}

void Rollup::setHystInEEPROM(byte hyst){
EEPROM.update(_localIndex+HYST_INDEX, hyst);
}

void Rollup::setRotationUpInEEPROM(byte rotationUp){
EEPROM.update(_localIndex+ROTATION_UP_INDEX, rotationUp);
}

void Rollup::setRotationDownInEEPROM(byte rotationDown){
EEPROM.update(_localIndex+ROTATION_DOWN_INDEX, rotationDown);
}

void Rollup::setIncrementsInEEPROM(byte increments){
EEPROM.update(_localIndex+INCREMENTS_INDEX, increments);
}

void Rollup::setPauseInEEPROM(byte pause){
EEPROM.update(_localIndex+PAUSE_INDEX, pause);
}

void Rollup::setSafetyInEEPROM(boolean safety){
EEPROM.update(_localIndex+SAFETY_INDEX, safety);
}

void Rollup::EEPROMUpdate(){
  	 #ifdef DEBUG_EEPROM
      Serial.println("ROLLUP TIMER : ");
      Serial.println(EEPROMTimer);
  	 #endif
     //Each 10 seconds
  if (EEPROMTimer > 10000) {
    //Reset timer
    EEPROMTimer = 0;
    //Check if variables match EEPROM saved values
    switch (_mode){
      case FIX_TEMP:
        if (_tempParameter.value() != (float)EEPROM.read(_localIndex)){
          setTempInEEPROM(_tempParameter.value());
        }
      break;
      case VAR_TEMP:
      case MAN_TEMP:
        if (_tempParameter.value() != (float)EEPROM.read(_localIndex)-10){
          byte adjustTempParameter = (byte)(_tempParameter.value() + 10);
          setTempInEEPROM(adjustTempParameter);
        }
      break;
    }

    if (_hyst.value() != (float)EEPROM.read(_localIndex+HYST_INDEX)){
      setHystInEEPROM(_hyst.value());
    }
    if (_rotationUp.value() != (unsigned short)EEPROM.read(_localIndex+ROTATION_UP_INDEX)){
      setRotationUpInEEPROM(_rotationUp.value());
    }
    if (_rotationDown.value() != (unsigned short)EEPROM.read(_localIndex+ROTATION_DOWN_INDEX)){
      setRotationDownInEEPROM(_rotationDown.value());
    }
    if (_increments.value() != (unsigned short)EEPROM.read(_localIndex+INCREMENTS_INDEX)){
      setIncrementsInEEPROM(_increments.value());
    }
    if (_pause.value() != (unsigned short)EEPROM.read(_localIndex+PAUSE_INDEX)){
      setPauseInEEPROM(_pause.value());
    }
    if (_safety != (boolean)EEPROM.read(_localIndex+SAFETY_INDEX)){
      setSafetyInEEPROM(_safety);
    }

    #ifdef DEBUG_EEPROM
    Serial.println(F("Rollup parameters saved in EEPROM"));
    //Print actual EEPROM values
    for(int x = _localIndex; x < _localIndex+PAUSE_INDEX; x++){
     Serial.print(F("Byte #")); Serial.print(x); Serial.print(": ");
     Serial.println(EEPROM.read(x));
    }
    #endif
  }
}
*/
//Return private variables

float Rollup::hyst(){
  return _hyst.value();
}
float Rollup::tempParameter(){
  return _tempParameter.value();
}
unsigned short Rollup::rotationUp(){
  return _rotationUp.value();
}
unsigned short Rollup::rotationDown(){
  return _rotationDown.value();
}
unsigned short Rollup::increments(){
  return _increments.value();
}
unsigned short Rollup::pause(){
  return _pause.value();
}
boolean Rollup::safety(){
  return _safety;
}

float Rollup::stageMod(byte number){
  return stage[number-1].mod();
}
byte Rollup::stageTarget(byte number){
  return stage[number-1].target();
}

unsigned short Rollup::incrementCounter(){
  return _incrementCounter;
}
byte Rollup::mode(){
  return _mode;
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
