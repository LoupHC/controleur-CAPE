/*
  Parameters.h
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

#ifndef Parameters_h
#define Parameters_h
//Each greenhouse parameter have 4 associated values, a minimumimum value, a
//maximum value, an actual value and the last value recorded
class floatParameter
{
  public:
    floatParameter();
    ~floatParameter();
    void setAddress(int adress);
    void setValue(float value);
    void setLastValue(float value);
    void updateLastValue();
    void setLimits(float minimum, float maximum);
    void loadInEEPROM();
    float minimum();
    float maximum();

    int address();
    float value();
    float lastValue();
    boolean valueHasChanged();

  private:
    int _address;
    float _minimum;
    float _maximum;
    float _value;
    float _lastValue;
};

class uShortParameter
{
  public:
    uShortParameter();
    ~uShortParameter();
    void setAddress(int adress);
    void setValue(unsigned short value);
    void setLastValue(unsigned short value);
    void updateLastValue();
    void setLimits(unsigned short minimum, unsigned short maximum);
    void loadInEEPROM();
    unsigned short minimum();
    unsigned short maximum();
    unsigned short value();
    unsigned short lastValue();
    boolean valueHasChanged();
    int address();

  private:
    int _address;
    unsigned short _minimum;
    unsigned short _maximum;
    unsigned short _value;
    unsigned short _lastValue;
};

class shortParameter
{
  public:
    shortParameter();
    ~shortParameter();
    void setAddress(int adress);
    void setValue(short  value);
    void setLastValue(short value);
    void updateLastValue();
    void setLimits(short minimum, short maximum);
    void loadInEEPROM();
    short minimum();
    short maximum();
    short value();
    short lastValue();
    boolean valueHasChanged();
    int address();

  private:
    int _address;
    short _minimum;
    short _maximum;
    short _value;
    short _lastValue;
};

class byteParameter
{
  public:
    byteParameter();
    ~byteParameter();
    void setValue(byte value);
    void setAddress(int adress);
    void setLastValue(byte value);
    void updateLastValue();
    void setLimits(byte minimum, byte maximum);
    void loadInEEPROM();
    byte minimum();
    byte maximum();
    byte value();
    byte lastValue();
    boolean valueHasChanged();
    int address();

  private:
    int _address;
    byte _minimum;
    byte _maximum;
    byte _value;
    byte _lastValue;
};

class timeParameter
{
  public:
    timeParameter();
    timeParameter(short hour, short minut);
    ~timeParameter();
    void setTime(short hour, short minut);
    void addTime(short hour, short minut);
    void subTime(short hour, short minut);
    short hour();
    short minut();
    int address();

  private:
    void adjustTime();
    short _hour;
    short _minut;
};

#endif
