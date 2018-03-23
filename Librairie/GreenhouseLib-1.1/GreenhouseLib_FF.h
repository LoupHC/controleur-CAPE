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
      void initOutput(byte relayType, byte pin);

      //action functions
      void routine(float target, float temp);
      void forceAction(unsigned short duration, boolean state);
      void forceAction(boolean condition, boolean state);

      //programmation functions
      void setParameters(float temp, float hyst, boolean safety);
      void setHyst(float hyst);
      void setMod(float temp);
      void setSafety(boolean safety);
      void loadEEPROMParameters();

			//return private variables
			byte pin();
			float hyst();
			float hystMin();
			float hystMax();
			float mod();
			float modMin();
			float modMax();
			boolean safety();
			boolean debug();
      unsigned short nb();


    private:
      void stop();
      void start();
      void desactivateRoutine();
      void activateRoutine();
			//Parameters
      boolean _relayType;
      boolean _activate;
      boolean _desactivate;
			byte _pin;

      floatParameter _hyst;
      floatParameter _mod;
			boolean _safety;
			//Logic variables
			boolean _debug;
      boolean _routine;
			//Indexes
			unsigned short _localIndex;
      static unsigned short _EEPROMindex;
  		unsigned short _localCounter;
  		static unsigned short _counter;
			//Timer
      elapsedMillis overrideTimer;
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
      void initOutput(byte relayType, byte pin);

      //action functions
      void routine(float target, float temp);
      void forceAction(unsigned short duration, boolean state);
      void forceAction(boolean condition, boolean state);

          //programmation functions
      void setParameters(float temp, float hyst, boolean safety);
      void setHyst(float hyst);
      void setMod(float temp);
      void setSafety(boolean safety);
      void loadEEPROMParameters();

			//return private variables
			byte pin();
      float hyst();
      float hystMin();
			float hystMax();
			float mod();
			float modMin();
			float modMax();
			boolean safety();
			boolean debug();
      unsigned short nb();

    private:

      void stop();
      void start();
      void desactivateRoutine();
      void activateRoutine();
			//const parameters
      boolean _relayType;
      boolean _activate;
      boolean _desactivate;
		  byte _pin;

      //Parameters
		  floatParameter _hyst;
		  floatParameter _mod;
		  boolean _safety;
			//logic
			boolean _debug;
      boolean _routine;
			//indexes
      unsigned short _localIndex;
      static unsigned short _EEPROMindex;
  		unsigned short _localCounter;
  		static unsigned short _counter;
			//timer

      elapsedMillis overrideTimer;
			elapsedMillis EEPROMTimer;
};

#endif
