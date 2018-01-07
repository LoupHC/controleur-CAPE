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

#define MAX_STAGES 4

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
    void setTargetLimits(byte minimum, byte maximum);
    void setModLimits(float minimum, float maximum);
    void setStage(float mod, byte target);
    void setMod(float mod);
    void setTarget(byte target);
    byte target();
    float mod();

  private:
    byteParameter _target;
    floatParameter _mod;
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
    void initRollup(byte stages);
    void initOutputs(byte mode, boolean relayType, byte rOpen, byte rClose);
    void initStage(Stage stage, float mod, byte inc);

    //action functions
    void routine(float temp);												//Mode FIX_TEMP
    void routine(float target, float temp);					//Mode VAR_TEMP
		void manualRoutine(float target, float temp);		//Mode MAN_TEMP
    void startOpening();
    void stopOpening();
    void startClosing();
    void stopClosing();
		void openToInc(short stage, byte targetIncrement);
		void closeToInc(short stage, byte targetIncrement);
		void desactivateRoutine();
    void activateRoutine();
		void setIncrementCounter(unsigned short increment);

    //Parameters functions
    void setParameters(float temp, float hyst, unsigned short rotationUp, unsigned short rotationDown, unsigned short increments,unsigned short pause, boolean safety);
    void setHyst(float hyst);
    void setTemp(float temp);
    void setRotationUp(unsigned short rotation);
    void setRotationDown(unsigned short rotation);
    void setIncrements(unsigned short increments);
    void setPause(unsigned short pause);
    void setStageParameters(byte number, float mod, byte target);
    void setStageMod(byte number, float value);
    void setStageTarget(byte number, byte value);
    void setSafety(boolean safety);
/*

		//EEPROM management functions
    void setParametersInEEPROM(short temp, byte hyst, byte rotationUp, byte rotationDown, byte increments, byte pause, boolean safety);
    void loadEEPROMParameters();
    void setTempInEEPROM(short temp);
    void setHystInEEPROM(byte rHyst);
    void setRotationUpInEEPROM(byte rotationUp);
    void setRotationDownInEEPROM(byte rotationDown);
    void setIncrementsInEEPROM(byte increments);
    void setPauseInEEPROM(byte pause);
    void setSafetyInEEPROM(boolean safety);

    void EEPROMUpdate();
*/
		//return private variables
    float hyst();
    float tempParameter();
    unsigned short rotationUp();
    unsigned short rotationDown();
    unsigned short increments();
    unsigned short pause();
    boolean safety();
    float stageMod(byte number);
    byte stageTarget(byte number);

    unsigned short incrementCounter();

    byte mode();
    boolean opening();
    boolean closing();
    boolean openingCycle();
    boolean closingCycle();

  private:
    //Parameters
    floatParameter _hyst;
    floatParameter _tempParameter;
    uShortParameter _rotationUp;
    uShortParameter _rotationDown;
    uShortParameter _pause;
    byteParameter _increments;
    boolean _safety;
    Stage stage[MAX_STAGES];


    //const parameters
    byte _mode;
    byte _openingPin;
    byte _closingPin;
    boolean _relayType;
    boolean _activate;
    boolean _desactivate;

		//Logic variables
		unsigned short _incrementCounter;
    boolean _opening;
    boolean _closing;
    boolean _openingCycle;
    boolean _closingCycle;
    boolean _routine;
		boolean _safetyCycle;
		short _move;
		short _stage;
		byte _stages;

		//Timers
		elapsedMillis rollupTimer;
		elapsedMillis EEPROMTimer;
		elapsedMillis safetyTimer;

		//Indexes
    unsigned short _localIndex;
    static unsigned short _index;

		//private functions
    void action(byte pin, boolean state);
    void openSides();
    void closingSides();
		void safetyCycle();
		void safetyOpen();
		void safetyClose();
		void printPause();
		void printEndPause();

};


#endif
