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

#ifndef Parameters_h
#define Parameters_h
//Each greenhouse parameter have 3 associated values, a minimumimum value, a maximumimum value and an actual value
class floatParameter
{
  public:
    floatParameter();
    ~floatParameter();
    void setValue(float value);
    void setLastValue(float value);
    void updateLastValue();
    void setLimits(float minimum, float maximum);
    void minimum(float minimum);
    void maximum(float maximum);

    float value();
    float lastValue();
    boolean valueHasChanged();

  private:
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
    void setValue(unsigned short value);
    void setLastValue(unsigned short value);
    void updateLastValue();
    void setLimits(unsigned short minimum, unsigned short maximum);
    void minimum(unsigned short minimum);
    void maximum(unsigned short minimum);
    unsigned short value();
    unsigned short lastValue();
    boolean valueHasChanged();

  private:
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
    void setValue(short  value);
    void setLastValue(short value);
    void updateLastValue();
    void setLimits(short minimum, short maximum);
    void minimum(short  minimum);
    void maximum(short  maximum);
    short value();
    short lastValue();
    boolean valueHasChanged();

  private:
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
    void setLastValue(byte value);
    void updateLastValue();
    void setLimits(byte minimum, byte maximum);
    void minimum(byte minimum);
    void maximum(byte maximum);
    byte value();
    byte lastValue();
    boolean valueHasChanged();

  private:
    byte _minimum;
    byte _maximum;
    byte _value;
    byte _lastValue;
};


//Generic macros
byte negativeToByte(int data, byte mod);
int byteToNegative(int data, byte mod);
void convertDecimalToTime(int * heure, int * minut);

#endif
