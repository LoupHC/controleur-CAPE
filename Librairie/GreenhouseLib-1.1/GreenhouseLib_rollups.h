/*
  GreenhouseLib_rollups.h
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

#include "Arduino.h"
#include "EEPROM.h"
#include "elapsedMillis.h"
#include "Parameters.h"
#include "Defines.h"

#ifndef GreenhouseLib_rollups_h
#define GreenhouseLib_rollups_h




/*
//A counting template...

template <typename T>
struct counter{
    counter(){count++;}
    virtual ~counter(){--count;}
    int count;
};

template <typename T> int counter<T>::count( 0 );
*/


class Stage
{
  public:
    Stage();
    ~Stage();
    void setTargetLimits(unsigned short minimum, unsigned short maximum);
    void setModLimits(float minimum, float maximum);
    void setTargetAddress(int address);
    void setModAddress(int address);
    void setStage(float mod, unsigned short target);
    void setMod(float mod);
    void setTarget(unsigned short target);
    unsigned short target();
    unsigned short targetAddress();
    unsigned short targetMin();
    unsigned short targetMax();
    float mod();
    unsigned short modAddress();
    float modMin();
    float modMax();
    unsigned short nb();

  private:
    uShortParameter _target;
    floatParameter _mod;
		unsigned short _localCounter;
		static unsigned short _counter;
};

/*
OBJECT : Rollup
Parameters :
- activation temperature
- hysteresis
- time of rotation for full opening (max. 250 sec.)
- time of rotation for a full closing (max. 250 sec.)
- number of increments (max. 99)
- pause time between increments (max. 250 sec.)
- safety mode (flag for further conditions)
*/
class Rollup
{
  public:
    //initialization functions
    Rollup();
    ~Rollup();
    void initOutputs(boolean relayType, byte rOpen, byte rClose);
    void initStage(Stage stage, float mod, byte inc);

    //action functions
		void manualRoutine(float target, float temp);
    //Override
    void forceMove(unsigned short duration, unsigned short targetIncrement);
    void forceMove(boolean condition, unsigned short targetIncrement);

    //Parameters functions
    void setParameters(float hyst, unsigned short rotationUp, unsigned short rotationDown, unsigned short increments,unsigned short pause, boolean safety);
    void setStages(byte stages);
    void setHyst(float hyst);
    void setRotationUp(unsigned short rotation);
    void setRotationDown(unsigned short rotation);
    void setIncrements(unsigned short increments);
    void setPause(unsigned short pause);
    void setStageParameters(byte number, float mod, byte target);
    void setStageMod(byte number, float value);
    void setStageTarget(byte number, unsigned short value);
    void setSafety(boolean safety);
    void setIncrementCounter(unsigned short increment);
    void loadEEPROMParameters();

		//return private variables
    float hyst();
    float hystMin();
    float hystMax();
    unsigned short rotationUp();
    unsigned short rotationUpMin();
    unsigned short rotationUpMax();
    unsigned short rotationDown();
    unsigned short rotationDownMin();
    unsigned short rotationDownMax();
    unsigned short pause();
    unsigned short pauseMin();
    unsigned short pauseMax();
    unsigned short increments();
    boolean safety();
    float stageMod(byte number);
    float stageModMin(byte number);
    float stageModMax(byte number);
    unsigned short stageTarget(byte number);
    unsigned short stageTargetMin(byte number);
    unsigned short stageTargetMax(byte number);
    unsigned short incrementCounter();
    boolean opening();
    boolean closing();
    boolean openingCycle();
    boolean closingCycle();
    unsigned short nb();



    //Parameters
  private:
    floatParameter _hyst;
    uShortParameter _rotationUp;
    uShortParameter _rotationDown;
    uShortParameter _pause;
    byte _increments;
    boolean _safety;
    Stage stage[MAX_STAGES];


    //const parameters
    byte _openingPin;
    byte _closingPin;
    boolean _relayType;
    boolean _activate;
    boolean _desactivate;

		//Logic variables
		unsigned short _incrementCounter;
    unsigned long _upStep;
    unsigned long _downStep;
    unsigned long _pauseTime;
		unsigned long _moveTime;
    short _incrementMove;
    short _stageMove;
    boolean _opening;
    boolean _closing;
    boolean _openingCycle;
    boolean _closingCycle;
    boolean _routine;
    boolean _override;
		boolean _safetyCycle;
		unsigned short _stage;
    unsigned short _upperStage;
    unsigned short _lowerStage;
		unsigned short _stages;

		//Timers
		elapsedMillis rollupTimer;
		elapsedMillis EEPROMTimer;
		elapsedMillis safetyTimer;

		//Indexes
    static unsigned short _EEPROMindex;
    unsigned short _localIndex;
		unsigned short _localCounter;
		static unsigned short _counter;

    void openOrClose(unsigned short stage, float temp, float targetTemp);
    void openToInc(unsigned short stage, unsigned short targetIncrement);
		//private functions
    void action(byte pin, boolean state);
    //Mode MAN_TEMP
    //Actions
    void startMove(short stageMove, short incrementMove);
    void stopMove();
    void resumeCycle();
    void startOpening();
    void stopOpening();
    void startClosing();
    void stopClosing();
    void desactivateRoutine();
    void activateRoutine();
    void calibrateStages();
		void safetyCycle();
		void printPause();
		void printEndPause();

};


#endif
