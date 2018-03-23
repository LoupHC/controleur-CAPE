/*
  GreenhouseLib_timing.h
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


#ifndef GreenhouseLib_timing_h
#define GreenhouseLib_timing_h


class Timepoint
{
	public:

		Timepoint();
    ~Timepoint();
		void setParameters(byte type, short hour, short min, float heatingTemp, float coolingTemp, unsigned short ramping);
		void setTime(byte type, short hour, short min);
		void setHeatTemp(float heatingTemp);
		void setCoolTemp(float coolingTemp);
		void setRamping(unsigned short ramping);
    void loadEEPROMParameters();
		void checkTime();
		byte type();
		unsigned short hr();
		short hrMod();
		short hrModMin();
		short hrModMax();
 		unsigned short mn();
 		short mnMod();
 		short mnModMin();
 		short mnModMax();
 		float heatingTemp();
 		float heatingTempMin();
 		float heatingTempMax();
 		float coolingTemp();
 		float coolingTempMin();
 		float coolingTempMax();
		unsigned short ramping();
		unsigned short rampingMin();
		unsigned short rampingMax();
    unsigned short nb();

		static short sunRise[3];
		static short sunSet[3];


	private:
		short _hr;
		short _mn;
		byteParameter _type;
		shortParameter _hrMod;
		shortParameter _mnMod;
		floatParameter _heatingTemp;
		floatParameter _coolingTemp;
		uShortParameter _ramping;
		timeParameter _time;

  	unsigned short _localIndex;
  	static unsigned short _EEPROMindex;
		unsigned short _localCounter;
		static unsigned short _counter;
		elapsedMillis EEPROMTimer;
};



#endif
