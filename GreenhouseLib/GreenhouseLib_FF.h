/*
  GreenhouseLib_FF.h
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




#ifndef GreenhouseLib_FF_h
#define GreenhouseLib_FF_h


/*
OBJECT : Fan (cooling unit)

Parameters :
- activation temperature
- hysteresis
- safety mode
*/
class Fan
{
  public:

      //initialization functions
      Fan();
      ~Fan();
      void initFan();
      void initOutput(byte mode, byte relayType, byte pin);

      //action functions
      void routine(float temp);
      void routine(float target, float temp);
      void stop();
      void start();
      void desactivateRoutine();
      void activateRoutine();

      //programmation functions
      void setParameters(float temp, float hyst, boolean safety);
      void setHyst(float hyst);
      void setTemp(float temp);
      void setSafety(boolean safety);
/*
			//EEPROM functions
      void setParametersInEEPROM(short temp, byte hyst, boolean safety);
      void loadEEPROMParameters();
      void setTempInEEPROM(short temp);
      void setHystInEEPROM(byte hyst);
      void setSafetyInEEPROM(boolean safety);
			void EEPROMUpdate();
*/
			//return private variables
			byte mode();
			byte pin();
			float hyst();
			float tempParameter();
			boolean safety();
			boolean debug();

    private:
			//Parameters
			byte _mode;
      boolean _relayType;
      boolean _activate;
      boolean _desactivate;
			byte _pin;
			floatParameter _hyst;
			floatParameter _tempParameter;

			boolean _safety;
			//Logic variables
			boolean _debug;
      boolean _routine;
			//Indexes
			unsigned short _localIndex;
      static unsigned short _index;
			//Timer
		elapsedMillis EEPROMTimer;

};

/*
OBJECT : Heater (heating unit)

Parameters :
- hysteresis
- activation temperature
- safety mode
*/

class Heater
{
  public:
    //initialization functions
      Heater();
      ~Heater();
      void initHeater();
      void initOutput(byte mode, byte relayType, byte pin);

      //action functions
      void routine(float temp);
      void routine(float target, float temp);
      void stop();
      void start();
      void desactivateRoutine();
      void activateRoutine();

          //programmation functions
      void setParameters(float temp, float hyst, boolean safety);
      void setHyst(float hyst);
      void setTemp(float temp);
      void setSafety(boolean safety);
      /*
				//EEPROM functions
      void setParametersInEEPROM(short temp, byte hyst, boolean safety);
      void loadEEPROMParameters();
      void setTempInEEPROM(short temp);
      void setHystInEEPROM(byte hyst);
      void setSafetyInEEPROM(boolean safety);
			void EEPROMUpdate();
*/
			//return private variables
			byte mode();
			byte pin();
			float hyst();
			float tempParameter();
			boolean safety();
			boolean debug();

    private:
			//const parameters
			byte _mode;
      boolean _relayType;
      boolean _activate;
      boolean _desactivate;
		  byte _pin;

      //Parameters
		  floatParameter _hyst;
		  floatParameter _tempParameter;

		  boolean _safety;
			//logic
			boolean _debug;
      boolean _routine;
			//indexes
      unsigned short _localIndex;
      static unsigned short _index;
			//timer
			elapsedMillis EEPROMTimer;

};

#endif
