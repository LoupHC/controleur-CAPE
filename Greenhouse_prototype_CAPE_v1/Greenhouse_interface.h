/*
  Greenhouse_prototype_CAPE_v.1.1.ino
  Copyright (C)2017 Loup Hébert-Chartrand. All right reserved

  This code has been made to interface with Arduino-like microcontrollers,
  for inclusion in greenhouse automation devices.

  Supported devices :
  - DS18B20 temperature sensor
  - DS3231 RTC module
  - 20x4 Serial LCD Display

  You can find the latest version of this code at :
  https://github.com/LoupHC/controleur-CAPE


  This code is free software; you can redistribute it and/or
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

//***************************************************
//********************GLOBAL**************************
//***************************************************
#define MODE_DISPLAY           1
#define MODE_PROGRAM           2
#define MODE_ACTION            3
#define SET_PARAMETER     21
#define ACTION            3

//********************LCD parameters *******************
//Features :
// - 20x4 LCD display
// - I2C backpack
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define I2CADDR_LCD    0x27
#define BACKLIGHT_PIN     3
LiquidCrystal_I2C  lcd(I2CADDR_LCD, 2, 1, 0, 4, 5, 6, 7);


//********************Keypad parameters *******************
//Features:
// - Matrix keypad 4x4
// - PCF8574 I/O expander

#define KEYPAD_DISPLAY

  #include <Keypad_I2C.h>
  #include <Keypad.h>
  #include <Wire.h>

#ifdef KEYPAD_DISPLAY
  #define I2CADDR_KEY 0x26
  const byte ROWS = 4; //four rows
  const byte COLS = 4; //four columns
  char keys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
  };

  // bit numbers of PCF8574 i/o port
  byte rowPins[ROWS] = {7,6,5,4}; //connect to the row pinouts of the keypad
  byte colPins[COLS] = {3,2,1,0}; //connect to the column pinouts of the keypad

  Keypad_I2C keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR_KEY);
#endif

#define I2C_OUTPUTS               //Comment this line if you don't use MCP23008 I/O Expander (Comment as well MCP_I2C_OUTPUTS on the Defines.h file of GreenhouseLib)

//********************Display variables*******************
char key = '1';
short menu = MODE_DISPLAY;
boolean firstPrint = true;
boolean fastPlus = false;
boolean fastMinus = false;
short line = 0;
short maxLine;
const byte Code_lenght = 5; // Give enough room for 4 chars + NULL char
char Data[Code_lenght] = "0000"; // 4 is the number of chars it can hold + the null char = 5
const char SETDAY[Code_lenght]    = "0001";
const char SETMONTH[Code_lenght]  = "0002";
const char SETYEAR[Code_lenght]   = "0003";
const char SETHOUR[Code_lenght]   = "0004";
const char SETMIN[Code_lenght]    = "0005";
const char R1HYST[Code_lenght]    = "1100";
const char R1ROTUP[Code_lenght]   = "1101";
const char R1ROTDOWN[Code_lenght] = "1102";
const char R1PAUSE[Code_lenght]   = "1103";
const char R1S1MOD[Code_lenght]   = "1110";
const char R1S1TARG[Code_lenght]  = "1111";
const char R1S2MOD[Code_lenght]   = "1120";
const char R1S2TARG[Code_lenght]  = "1121";
const char R1S3MOD[Code_lenght]   = "1130";
const char R1S3TARG[Code_lenght]  = "1131";
const char R1S4MOD[Code_lenght]   = "1140";
const char R1S4TARG[Code_lenght]  = "1141";
const char R1S5MOD[Code_lenght]   = "1150";
const char R1S5TARG[Code_lenght]  = "1151";
const char R2HYST[Code_lenght]    = "1200";
const char R2ROTUP[Code_lenght]   = "1201";
const char R2ROTDOWN[Code_lenght] = "1202";
const char R2PAUSE[Code_lenght]   = "1203";
const char R2S1MOD[Code_lenght]   = "1210";
const char R2S1TARG[Code_lenght]  = "1211";
const char R2S2MOD[Code_lenght]   = "1220";
const char R2S2TARG[Code_lenght]  = "1221";
const char R2S3MOD[Code_lenght]   = "1230";
const char R2S3TARG[Code_lenght]  = "1231";
const char R2S4MOD[Code_lenght]   = "1240";
const char R2S4TARG[Code_lenght]  = "1241";
const char R2S5MOD[Code_lenght]   = "1250";
const char R2S5TARG[Code_lenght]  = "1251";
const char F1HYST[Code_lenght]    = "2100";
const char F1MOD[Code_lenght]     = "2101";
const char F2HYST[Code_lenght]    = "2200";
const char F2MOD[Code_lenght]     = "2201";
const char H1HYST[Code_lenght]    = "3100";
const char H1MOD[Code_lenght]     = "3101";
const char H2HYST[Code_lenght]    = "3200";
const char H2MOD[Code_lenght]     = "3201";
const char T1TYPE[Code_lenght]    = "4100";
const char T1HOUR[Code_lenght]    = "4101";
const char T1MIN[Code_lenght]     = "4102";
const char T1HEATT[Code_lenght]   = "4103";
const char T1COOLT[Code_lenght]   = "4104";
const char T1RAMP[Code_lenght]    = "4105";
const char T2TYPE[Code_lenght]    = "4200";
const char T2HOUR[Code_lenght]    = "4201";
const char T2MIN[Code_lenght]     = "4202";
const char T2HEATT[Code_lenght]   = "4203";
const char T2COOLT[Code_lenght]   = "4204";
const char T2RAMP[Code_lenght]    = "4205";
const char T3TYPE[Code_lenght]    = "4300";
const char T3HOUR[Code_lenght]    = "4301";
const char T3MIN[Code_lenght]     = "4302";
const char T3HEATT[Code_lenght]   = "4303";
const char T3COOLT[Code_lenght]   = "4304";
const char T3RAMP[Code_lenght]    = "4305";
const char T4TYPE[Code_lenght]    = "4400";
const char T4HOUR[Code_lenght]    = "4401";
const char T4MIN[Code_lenght]     = "4402";
const char T4HEATT[Code_lenght]   = "4403";
const char T4COOLT[Code_lenght]   = "4404";
const char T4RAMP[Code_lenght]    = "4405";
const char T5TYPE[Code_lenght]    = "4500";
const char T5HOUR[Code_lenght]    = "4501";
const char T5MIN[Code_lenght]     = "4502";
const char T5HEATT[Code_lenght]   = "4503";
const char T5COOLT[Code_lenght]   = "4504";
const char T5RAMP[Code_lenght]    = "4505";
const char TESTRELAYS[Code_lenght]= "5555";
const char TESTIIC[Code_lenght]   = "6666";

elapsedMillis unpressedTimer;
elapsedMillis pressedTimer;
float fvariable;
unsigned short usvariable;
short svariable;
byte typeSet;
unsigned short hourSet;
unsigned short minSet;

boolean confirm = false;
byte action = 0;


byte data_count = 0, master_count = 0;
bool Pass_is_good;
char keyPressed;

//Flashing display
byte flashCount1 = 0;
byte flashCount2 = 0;

boolean relayTest = false;
const byte RyDriverI2CAddr = B0100000;//device ID
byte GPIO = B11111111;
byte validAddress[6] = {0};


//***************************************************
//********************MACROS**************************
//***************************************************

void serialPrintDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
  Serial.print("0");
}

void lcdPrintDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
  lcd.print(F("0"));
  lcd.print(digits);
}

void initLCD(byte length, byte width){

  lcd.begin(length, width);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.clear();
}

void lcdPrintOutput(String item, byte _column, byte _row, Fan fan){
    //Fan
    lcd.setCursor(_column, _row); lcd.print(F("          "));
    lcd.setCursor(_column, _row); lcd.print(item);
    if(fan.isActive() == true){
        lcd.print(F("ON "));
      }
    else{
      lcd.print(F("OFF"));
    }
    if(fan.override() == true){
      lcd.setCursor(_column+8, _row);
      lcd.print ("*");
    }
    else{
      lcd.setCursor(_column+8, _row);
      lcd.print (" ");
    }
}
void lcdPrintOutput(String item, byte _column, byte _row, Heater heater){
    //Fan
    lcd.setCursor(_column, _row); lcd.print(F("          "));
    lcd.setCursor(_column, _row); lcd.print(item);
    if(heater.isActive() == true){
        lcd.print(F("ON "));
      }
    else{
      lcd.print(F("OFF"));
    }
    if(heater.override() == true){
      lcd.setCursor(_column+8, _row);
      lcd.print ("*");
    }
    else{
        lcd.setCursor(_column+8, _row);
      lcd.print (" ");
    }
}
void lcdPrintRollups(String side, String opening, String closing, byte _column, byte _row, Rollup rollup, byte* counter){
  //East rollup
  lcd.setCursor(_column, _row); lcd.print(F("          "));
  lcd.setCursor(_column, _row);
  if(rollup.incrementCounter() == OFF_VAL){lcd.print(F(""));}
  else if (rollup.opening() == true){lcd.print(opening);}
  else if (rollup.closing() == true){lcd.print(closing);}
  else if(rollup.isWaiting()){
    if(*counter == 0){
      lcd.print(side); lcd.print(rollup.incrementCounter());lcd.print(F("%"));
      *counter += 1;
    }
    else if(*counter == 1){
      lcd.print(F("          "));
      *counter = 0;
    }
  }
  else{lcd.print(side); lcd.print(rollup.incrementCounter());lcd.print(F("%"));}
  if(rollup.override() == true){
    lcd.setCursor(_column+8, _row);
    lcd.print ("*");
  }
  else{
      lcd.setCursor(_column+8, _row);
      lcd.print (" ");
    }
  }
void lcdPrintTemp(byte _row){

    lcd.setCursor(0,_row); lcd.print(F("         "));
    lcd.setCursor(0,_row);
    if(sensorFailure == false){lcd.print(greenhouseTemperature.value()); lcd.print(F("C"));}
    else if(sensorFailure == true){lcd.print(F("!!!"));}
}

void lcdPrintTarget(){
  lcd.setCursor(9,0); lcd.print(F("           "));
  lcd.setCursor(9,0);
   lcd.print(F("|")); lcd.print((int)heatingTemperature); lcd.print(F("-"));lcd.print((int)coolingTemperature);

   if(greenhouse.weather() == SUN){
     lcd.print("(SUN)");
   }
   else{
     lcd.print("(CLD)");
   }


}

void lcdPrintTime(byte _row){
    lcd.setCursor(0,_row); lcdPrintDigits(greenhouse.rightNow(2)); lcd.print(F(":")); lcdPrintDigits(greenhouse.rightNow(1));lcd.print(F(":")); lcdPrintDigits(greenhouse.rightNow(0)); // lcd.print(F(" |")); lcdPrintDigits(greenhouse.rightNow(3)); lcd.print(F("/")); lcdPrintDigits(greenhouse.rightNow(4)); lcd.print(F("/")); lcdPrintDigits(2000+greenhouse.rightNow(5));
    lcd.setCursor(9,_row); lcd.print(F("|(TIMEP: ")); lcd.print(greenhouse.nowTimepoint()); lcd.print(F(") "));
}

void lcdPrintOutputs(){
  #if ROLLUPS >= 1
  lcdPrintRollups("R1:","OPENING", "CLOSING", 0, 2, R1, &flashCount1);
  #endif
  #if ROLLUPS == 2
    lcdPrintRollups("|R2:","|OPENING", "|CLOSING", 9, 2, R2, &flashCount2);
  #endif

  #if FANS == 1 && HEATERS == 0
    lcdPrintOutput("F1: ", 0, 3, F1);
  #elif FANS == 1 && HEATERS == 1
    lcdPrintOutput("F1: ", 0, 3, F1);
    lcdPrintOutput("|H1: ", 9, 3, H1);
  #elif FANS == 2 && HEATERS == 0
    lcdPrintOutput("F1: ", 0, 3, F1);
    lcdPrintOutput("|F2: ", 9, 3, F2);
  #elif FANS == 0 && HEATERS == 1
    lcdPrintOutput("H1: ", 0, 3, H1);
  #elif FANS == 0 && HEATERS == 2
    lcdPrintOutput("H1: ", 0, 3, H1);
    lcdPrintOutput("|H2: ", 9, 3, H2);
  #else
    lcd.setCursor(0, 3);
    lcd.print(F("                    "));
  #endif

}

void homeDisplay(){

  if(firstPrint == true){
    lcd.clear();
    lcdPrintTemp(0);
    lcdPrintTime(1);
    lcdPrintOutputs();
    firstPrint = false;
  }

  if(greenhouseTemperature.valueHasChanged()){
    lcdPrintTemp(0);
    greenhouseTemperature.updateLastValue();
  }
  for(int x = 0; x < 6; x++){
    if (rightNow[x].valueHasChanged()){
      lcdPrintTime(1);
      rightNow[x].updateLastValue();
    }
  }
  lcdPrintTarget();
  lcdPrintOutputs();
}

#ifdef KEYPAD_DISPLAY
void printHeader(String header, short maxLineOfMenu){

  //clear + print header
  if(firstPrint == true){
    unpressedTimer = 0;
    lcd.clear();
    lcd.setCursor(0,0);lcd.print(header);
    maxLine = maxLineOfMenu;
  }
}

void adjustLine(){
  //adjust line
  if (line > maxLine - 3){
    line = maxLine-3;
  }
  if(line<0){
    line = 0;
  }
}

void rollupDisplay(Rollup rollup){

  printHeader("ROLLUP - PARAMET ",3);
  if(firstPrint == true){
    lcd.print("(");
    lcd.print(rollup.nb()+1);
    lcd.print(")");
    firstPrint = false;
  }
  adjustLine();

  //print content
  for(int x = 1; x < 4; x++){
    int writeLine = x+line;
    if (writeLine > maxLine){
      writeLine = maxLine;
    }
    switch(writeLine){
      case 1: lcd.setCursor(0,x);lcd.print(F("UP: ")); lcd.print(rollup.rotationUp.value());lcd.print(F("s")); lcd.setCursor(9,x);lcd.print(F("|DOWN: ")); lcd.print(rollup.rotationDown.value());lcd.print(F("s"));break;
      case 2: lcd.setCursor(0,x);lcd.print(F("Pause: ")); lcd.print(rollup.pause.value());lcd.print(F("s"));break;
      case 3:  lcd.setCursor(0,x);lcd.print(F("Hyst:  "));  lcd.print(rollup.hyst.value());lcd.print(F("C"));break;
    }
  }
}

void stageDisplay(Rollup rollup){

  float stageTemp1 = greenhouse.coolingTemp() + rollup.stage[1].mod.value();
  float stageTemp2 = greenhouse.coolingTemp() + rollup.stage[2].mod.value();
  float stageTemp3 = greenhouse.coolingTemp() + rollup.stage[3].mod.value();
  float stageTemp4 = greenhouse.coolingTemp() + rollup.stage[4].mod.value();

  printHeader("ROLLUP - STAGES ",STAGES);
  if(firstPrint == true){
    lcd.print("(");
    lcd.print(rollup.nb()+1);
    lcd.print(")");
    firstPrint = false;
  }
  adjustLine();

  //print content
  int maxLines = STAGES+1;
  if(maxLines > 4){
    maxLines = 4;
  }
  for(int x = 1; x < maxLines; x++){
    int writeLine = x+line;
    if (writeLine > maxLine){
      writeLine = maxLine;
    }
    switch(writeLine){
      case 1:  lcd.setCursor(0,x);lcd.print(F("S1: ")); lcd.print(stageTemp1);lcd.print(F("-"));lcd.print(stageTemp2);lcd.print(F("-")); lcd.print(rollup.stage[1].target.value()); lcd.print(F("%"));break;
      case 2:  lcd.setCursor(0,x);lcd.print(F("S2: ")); lcd.print(stageTemp2);lcd.print(F("-"));lcd.print(stageTemp3);lcd.print(F("-")); lcd.print(rollup.stage[2].target.value()); lcd.print(F("%"));break;
      case 3:  lcd.setCursor(0,x);lcd.print(F("S3: ")); lcd.print(stageTemp3);lcd.print(F("-"));lcd.print(stageTemp4);lcd.print(F("-")); lcd.print(rollup.stage[3].target.value()); lcd.print(F("%"));break;
      case 4:  lcd.setCursor(0,x);lcd.print(F("S4: ")); lcd.print(stageTemp4);lcd.print(F("- ... "));lcd.print(F("-")); lcd.print(rollup.stage[4].target.value()); lcd.print(F("%"));break;
    }
  }
}

void fanDisplay(Fan fan){

  float onTemp = greenhouse.coolingTemp() + fan.mod.value();
  float offTemp = greenhouse.coolingTemp() + fan.mod.value() - fan.hyst.value();

  printHeader("FAN - PARAMETERS ",3);
  if(firstPrint == true){
    lcd.print("(");
    lcd.print(fan.nb()+1);
    lcd.print(")");
    firstPrint = false;
  }
  adjustLine();

  //print content
  for(int x = 1; x < 4; x++){
    int writeLine = x+line;
    if (writeLine > maxLine){
      writeLine = maxLine;
    }
    switch(writeLine){
      case 1: lcd.setCursor(0,x);lcd.print(F("Activ. temp: ")); lcd.print(onTemp);lcd.print(F("C"));break;
      case 2: lcd.setCursor(0,x);lcd.print(F("Shut. temp: ")); lcd.print(offTemp);lcd.print(F("C"));break;
      case 3: lcd.setCursor(0,x);lcd.print(F("MOD:")); if(fan.mod.value() >= 0){lcd.print(F(" "));}lcd.print(fan.mod.value()); lcd.setCursor(9,3);lcd.print(F("|HYST: ")); lcd.print(fan.hyst.value());break;
    }
  }
}

void heaterDisplay(Heater heater){

  float onTemp = greenhouse.heatingTemp() + heater.mod.value();
  float offTemp = greenhouse.heatingTemp() + heater.mod.value() + heater.hyst.value();

  printHeader("HEATER - PARAMET ",3);
  if(firstPrint == true){
    lcd.print("(");
    lcd.print(heater.nb()+1);
    lcd.print(")");
    firstPrint = false;
  }
  adjustLine();

  //print content
  for(int x = 1; x < 4; x++){
    int writeLine = x+line;
    if (writeLine > maxLine){
      writeLine = maxLine;
    }
    switch(writeLine){
      case 1: lcd.setCursor(0,x);lcd.print(F("Activ. temp: ")); lcd.print(onTemp);lcd.print(F("C"));break;
      case 2: lcd.setCursor(0,x);lcd.print(F("Shut. temp: ")); lcd.print(offTemp);lcd.print(F("C"));break;
      case 3: lcd.setCursor(0,x);lcd.print(F("MOD:")); if(heater.mod.value() >= 0){lcd.print(F(" "));}lcd.print(heater.mod.value()); lcd.setCursor(9,3);lcd.print(F("|HYST: ")); lcd.print(heater.hyst.value());break;
    }
  }
}

void timepointsDisplay(){
  printHeader("TIMEPOINTS - TIME",TIMEPOINTS);
  if(firstPrint == true){
    firstPrint = false;
  }
  adjustLine();

  //print content
  int maxLines = TIMEPOINTS+1;
  if(maxLines > 4){
    maxLines = 4;
  }
  for(int x = 1; x < maxLines; x++){
    int writeLine = x+line;
    if (writeLine > maxLine){
      writeLine = maxLine;
    }

    switch(writeLine){
      case 1:
      #if TIMEPOINTS > 1
        lcd.setCursor(0,x);lcd.print(F("TP1: ")); lcdPrintDigits(T1.hr());lcd.print(F(":"));lcdPrintDigits(T1.mn());lcd.print(F("-")); lcdPrintDigits(T2.hr());lcd.print(F(":"));lcdPrintDigits(T2.mn());
      #endif
      #if TIMEPOINTS >1
      switch (T1.type.value()){
        case 0: lcd.print("(SR)");break;
        case 1: lcd.print("(CL)");break;
        case 2: lcd.print("(SS)");break;
      }
      #endif
      break;
      case 2:
      #if TIMEPOINTS == 2
        lcd.setCursor(0,x);lcd.print(F("TP2: ")); lcdPrintDigits(T2.hr());lcd.print(F(":"));lcdPrintDigits(T2.mn());lcd.print(F("-")); lcdPrintDigits(T1.hr());lcd.print(F(":"));lcdPrintDigits(T1.mn());
      #elif TIMEPOINTS >2
        lcd.setCursor(0,x);lcd.print(F("TP2: ")); lcdPrintDigits(T2.hr());lcd.print(F(":"));lcdPrintDigits(T2.mn());lcd.print(F("-")); lcdPrintDigits(T3.hr());lcd.print(F(":"));lcdPrintDigits(T3.mn());
      #endif
      #if TIMEPOINTS >=2
      switch (T2.type.value()){
        case 0: lcd.print("(SR)");break;
        case 1: lcd.print("(CL)");break;
        case 2: lcd.print("(SS)");break;
      }
      #endif
      break;
      case 3:
      #if TIMEPOINTS ==3
      lcd.setCursor(0,x);lcd.print(F("TP3: ")); lcdPrintDigits(T3.hr());lcd.print(F(":"));lcdPrintDigits(T3.mn());lcd.print(F("-")); lcdPrintDigits(T1.hr());lcd.print(F(":"));lcdPrintDigits(T1.mn());
      #elif TIMEPOINTS > 3
      lcd.setCursor(0,x);lcd.print(F("TP3: ")); lcdPrintDigits(T3.hr());lcd.print(F(":"));lcdPrintDigits(T3.mn());lcd.print(F("-")); lcdPrintDigits(T4.hr());lcd.print(F(":"));lcdPrintDigits(T4.mn());
      #endif
      #if TIMEPOINTS >=3
      switch (T3.type.value()){
        case 0: lcd.print("(SR)");break;
        case 1: lcd.print("(CL)");break;
        case 2: lcd.print("(SS)");break;
      }
      #endif
      break;
      case 4:
      #if TIMEPOINTS ==4
      lcd.setCursor(0,x);lcd.print(F("TP4: ")); lcdPrintDigits(T4.hr());lcd.print(F(":"));lcdPrintDigits(T4.mn());lcd.print(F("-")); lcdPrintDigits(T1.hr());lcd.print(F(":"));lcdPrintDigits(T1.mn());
      #endif
      #if TIMEPOINTS >4
      lcd.setCursor(0,x);lcd.print(F("TP4: ")); lcdPrintDigits(T4.hr());lcd.print(F(":"));lcdPrintDigits(T4.mn());lcd.print(F("-")); lcdPrintDigits(T5.hr());lcd.print(F(":"));lcdPrintDigits(T5.mn());
      #endif
      #if TIMEPOINTS >=4
      switch (T4.type.value()){
        case 0: lcd.print("(SR)");break;
        case 1: lcd.print("(CL)");break;
        case 2: lcd.print("(SS)");break;
      }
      #endif
      break;
      case 5:
      #if TIMEPOINTS ==5
      lcd.setCursor(0,x);lcd.print(F("TP5: ")); lcdPrintDigits(T5.hr());lcd.print(F(":"));lcdPrintDigits(T5.mn());lcd.print(F("-")); lcdPrintDigits(T1.hr());lcd.print(F(":"));lcdPrintDigits(T1.mn());
      #endif
      #if TIMEPOINTS ==5
      switch (T5.type.value()){
        case 0: lcd.print("(SR)");break;
        case 1: lcd.print("(CL)");break;
        case 2: lcd.print("(SS)");break;
      }
      #endif
      break;
   }
  }
}

void temperaturesDisplay(){

  if(greenhouse.weather() == SUN){
    printHeader("TIMEP - TEMP(SUN)",TIMEPOINTS);
  }
  else if(greenhouse.weather() == CLOUD){
    printHeader("TIMEP - TEMP(CLOUD)",TIMEPOINTS);
  }
  if(firstPrint == true){
    firstPrint = false;
  }

  adjustLine();

  //print content
  int maxLines = TIMEPOINTS+1;
  if(maxLines > 4){
    maxLines = 4;
  }
  for(int x = 1; x < maxLines; x++){
    int writeLine = x+line;
    if (writeLine > maxLine){
      writeLine = maxLine;
    }
    if(greenhouse.weather() == SUN){
      switch(writeLine){
        #if TIMEPOINTS >= 1
          case 1: lcd.setCursor(0,x);lcd.print(F("TP1: ")); lcd.print(T1.heatingTemp.value());lcd.print(F("-")); lcd.print(T1.coolingTemp.value());lcd.print(F(" R")); lcd.print(T1.ramping.value());break;
        #endif
        #if TIMEPOINTS >= 2
          case 2: lcd.setCursor(0,x);lcd.print(F("TP2: ")); lcd.print(T2.heatingTemp.value());lcd.print(F("-")); lcd.print(T2.coolingTemp.value());lcd.print(F(" R")); lcd.print(T2.ramping.value());break;
        #endif
        #if TIMEPOINTS >= 3
          case 3: lcd.setCursor(0,x);lcd.print(F("TP3: ")); lcd.print(T3.heatingTemp.value());lcd.print(F("-")); lcd.print(T3.coolingTemp.value());lcd.print(F(" R")); lcd.print(T3.ramping.value());break;
        #endif
        #if TIMEPOINTS >= 4
          case 4: lcd.setCursor(0,x);lcd.print(F("TP4: ")); lcd.print(T4.heatingTemp.value());lcd.print(F("-")); lcd.print(T4.coolingTemp.value());lcd.print(F(" R")); lcd.print(T4.ramping.value());break;
        #endif
        #if TIMEPOINTS == 5
          case 5: lcd.setCursor(0,x);lcd.print(F("TP5: ")); lcd.print(T5.heatingTemp.value());lcd.print(F("-")); lcd.print(T5.coolingTemp.value());lcd.print(F(" R")); lcd.print(T5.ramping.value());break;
        #endif
      }
    }
    else if(greenhouse.weather() == CLOUD){
      switch(writeLine){
        #if TIMEPOINTS >= 1
          case 1: lcd.setCursor(0,x);lcd.print(F("TP1: ")); lcd.print(T1.heatingTempCloud.value());lcd.print(F("-")); lcd.print(T1.coolingTempCloud.value());lcd.print(F(" R")); lcd.print(T1.ramping.value());break;
        #endif
        #if TIMEPOINTS >= 2
          case 2: lcd.setCursor(0,x);lcd.print(F("TP2: ")); lcd.print(T2.heatingTempCloud.value());lcd.print(F("-")); lcd.print(T2.coolingTempCloud.value());lcd.print(F(" R")); lcd.print(T2.ramping.value());break;
        #endif
        #if TIMEPOINTS >= 3
          case 3: lcd.setCursor(0,x);lcd.print(F("TP3: ")); lcd.print(T3.heatingTemp.value());lcd.print(F("-")); lcd.print(T3.coolingTemp.value());lcd.print(F(" R")); lcd.print(T3.ramping.value());break;
        #endif
        #if TIMEPOINTS >= 4
          case 4: lcd.setCursor(0,x);lcd.print(F("TP4: ")); lcd.print(T4.heatingTemp.value());lcd.print(F("-")); lcd.print(T4.coolingTemp.value());lcd.print(F(" R")); lcd.print(T4.ramping.value());break;
        #endif
        #if TIMEPOINTS == 5
          case 5: lcd.setCursor(0,x);lcd.print(F("TP5: ")); lcd.print(T5.heatingTemp.value());lcd.print(F("-")); lcd.print(T5.coolingTemp.value());lcd.print(F(" R")); lcd.print(T5.ramping.value());break;
        #endif
      }
    }
  }
}
void geoDisplay(){

}

void sensorsDisplay(){

}
#endif
void menuDisplay(){
  switch (key){
      case '1' : homeDisplay(); break;
    #if ROLLUPS >= 1 && defined(KEYPAD_DISPLAY)
      case '2' : rollupDisplay(R1);break;
    #endif
    #if ROLLUPS == 2 && defined(KEYPAD_DISPLAY)
      case '3' : rollupDisplay(R2);break;
    #endif
    #if ROLLUPS >= 1 && defined(KEYPAD_DISPLAY)
      case '5' : stageDisplay(R1);break;
    #endif
    #if ROLLUPS == 2 && defined(KEYPAD_DISPLAY)
      case '6' : stageDisplay(R2);break;
    #endif
    #if FANS >= 1 && defined(KEYPAD_DISPLAY)
      case '4' : fanDisplay(F1);break;
    #elif HEATERS >= 1 && defined(KEYPAD_DISPLAY)
      case '4' : heaterDisplay(H1);break;
    #endif
    #if FANS == 2 && defined(KEYPAD_DISPLAY)
      case '7' : fanDisplay(F2);break;
    #elif FANS == 1 && HEATERS == 1 && defined(KEYPAD_DISPLAY)
      case '7' : heaterDisplay(H1);break;
    #elif HEATERS == 2 && defined(KEYPAD_DISPLAY)
      case '7' : heaterDisplay(H2);break;
    #endif
    #ifdef KEYPAD_DISPLAY
      case '8' : timepointsDisplay();break;
      case '9' : temperaturesDisplay();break;
    #endif
  }

}

void select(){

  lcd.setCursor(0,2);
  lcd.print("YES");
  lcd.setCursor(0,3);
  lcd.print("NO");

  if(line > 1){
    line = 0;
  }
  if(line < 0){
    line = 1;
  }
  if(line == 0){
    lcd.setCursor(5, 3);
    lcd.blink();
    confirm = false;
  }
  if(line == 1){
    lcd.setCursor(5, 2);
    lcd.blink();
    confirm = true;
  }
}

void menuAction(){
    if(keyPressed == 'C'){
      lcd.clear();
      lcd.print("---SELECT ACTION----");
    }
    if(keyPressed == '1'){
      action = 1;
      lcd.clear();
      if(greenhouse.weather() == SUN){
        lcd.print("CLOUDY DAY?");
      }
      if(greenhouse.weather() == CLOUD){
        lcd.print("SUNNY DAY?");
      }
    }
    if(keyPressed == '2'){
      action = 2;
      lcd.clear();
      lcd.print("FULL VENTILATION?");
    }
    if(keyPressed == '3'){
      action = 3;
      lcd.clear();
      if (deshum == true){
        lcd.print("DISABLE DESHUM?");
      }
      else if(deshum == false){
        lcd.print("ENABLE DESHUM?");
      }
    }
    if(action != 0){
      select();
    }
    if(keyPressed == 'D'){
      menu = MODE_DISPLAY;key = '1';firstPrint = true; unpressedTimer = 0; line = 0;
      lcd.noBlink();
      switch(action){
        case 1:
          if(confirm == true){
            if(greenhouse.weather() == SUN){
              greenhouse.setWeather(CLOUD);
            }
            else if(greenhouse.weather() == CLOUD){
              greenhouse.setWeather(SUN);
            }
            key = '9';
          }
        break;
        case 2:
          if(confirm == true){
              fullVentOverride = true;
            }
        break;
        case 3:
          if(confirm == true){
            if(deshum == true){
              deshum = false;
            }
            else if (deshum == false){
              deshum = true;
            }
          }
        break;
      }
      action = 0;
    }
  }


void menuProgram(){

    if(firstPrint == true){
      lcd.setCursor(0,0);
      unpressedTimer = 0;
      lcd.clear();
      lcd.print(F("--Enter #Parameter--"));
      firstPrint = false;
    }

    if ((keyPressed != NO_KEY) && (keyPressed != 'A')&&(keyPressed != 'B')&&(keyPressed != 'C')&&(keyPressed != 'D')&&(keyPressed != '*')&&(keyPressed != '#') ) // makes sure a key is actually pressed, equal to (customKey != NO_KEY)
    {
      Data[data_count] = keyPressed; // store char into data array
      lcd.setCursor(data_count+8,2); // move cursor to show each new char
      lcd.blink();
      lcd.print(Data[data_count]); // print char at said cursor
      data_count++; // increment data array by 1 to store new char, also keep track of the number of chars entered
      if(data_count == Code_lenght-1){
        data_count = 0;
        lcd.setCursor(data_count+8,2);
      }
    }

    if((data_count == 0) && (keyPressed == 'D')) // if the array index is equal to the number of expected chars, compare data to master
    {
      menu = SET_PARAMETER;
      line = 0;
      unpressedTimer = 0;
      firstPrint = true;
    }
}

void confirmVariable(String variableName, float min, float value, float max){
  fvariable = value+((float)line*0.10);
  if(fvariable > max){
    fvariable = max;
    line -= 1;
  }
  if(fvariable < min){
    fvariable = min;
    line += 1;
  }
  lcd.noBlink();
  lcd.setCursor(0,1);
  lcd.print(variableName);
  lcd.setCursor(0,2);
  lcd.print("[min] [value] [max]");
  lcd.setCursor(0,3);
  lcd.print("[");
  lcd.print(min);
  lcd.print("][");
  lcd.print(fvariable);
  lcd.print("][");
  lcd.print(max);
  lcd.print("]");


}
void confirmVariable(String variableName, unsigned short min, unsigned short value, unsigned short max){
  svariable = (short)value + line;
  if(svariable > (short)max){
    svariable = (short)max;
    line -= 1;
  }
  if(svariable < (short)min){
    svariable = (short)min;
    line += 1;
  }
  usvariable = (unsigned long)svariable;

  lcd.noBlink();
  lcd.setCursor(0,1);
  lcd.print(variableName);
  lcd.setCursor(0,2);
  lcd.print(" [min][value][max] ");
  lcd.setCursor(3,3);
  lcd.print("[");
  lcd.print(min);
  lcd.print("] [");
  lcd.print(usvariable);
  lcd.print("] [");
  lcd.print(max);
  lcd.print("]");
}
void confirmVariable(String variableName, short min, short value, short max){
  svariable = value + line;
  if(svariable > max){
    svariable = max;
    line -= 1;
  }
  if(svariable < min){
    svariable = min;
    line += 1;
  }
  lcd.noBlink();
  lcd.setCursor(0,1);
  lcd.print(variableName);
  lcd.setCursor(0,2);
  lcd.print(" [min][value][max] ");
  lcd.setCursor(3,3);
  lcd.print("[");
  lcd.print(min);
  lcd.print("] [");
  lcd.print(svariable);
  lcd.print("] [");
  lcd.print(max);
  lcd.print("]");
}
void confirmType(String variableName, byte typeValue){
  short type = (short)typeValue + line;
  if(type > 2){
    type = 2;
    line -= 1;
  }
  if(type < 0){
    type = 0;
    line += 1;
  }
  typeSet = type;

  lcd.noBlink();
  lcd.setCursor(0,1);
  lcd.print(variableName);
  lcd.setCursor(7,3);
  lcd.print("[");
  switch (typeSet){
    case 0: lcd.print("SR");break;
    case 1: lcd.print("CLOCK");break;
    case 2: lcd.print("SS");break;
  }
  lcd.print("]   ");
}


void checkIIC(){
  lcd.noBlink();
  lcd.setCursor(0,0);
  lcd.print("----I2C DEVICES-----");

  byte error, address;
  int nDevices = 0;

  for(address = 1; address < 127; address++ ) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      validAddress[nDevices] = address;
      nDevices++;
    }
  }
  lcd.setCursor(0,1);
  if(validAddress[0] != 0){
    lcd.print("0x");
    if (validAddress[0]<16)
      lcd.print("0");
    lcd.print(validAddress[0],HEX);
  }
  lcd.setCursor(0,2);
  if(validAddress[1] != 0){
    lcd.print("0x");
    if (validAddress[1]<16)
      lcd.print("0");
    lcd.print(validAddress[1],HEX);
  }
  lcd.setCursor(0,3);
  if(validAddress[2] != 0){
    lcd.print("0x");
    if (validAddress[2]<16)
      lcd.print("0");
    lcd.print(validAddress[2],HEX);
  }
  lcd.setCursor(10,1);
  if(validAddress[3] != 0){
    lcd.print("0x");
    if (validAddress[3]<16)
      lcd.print("0");
    lcd.print(validAddress[3],HEX);
  }
  lcd.setCursor(10,2);
  if(validAddress[4] != 0){
    lcd.print("0x");
    if (validAddress[4]<16)
      lcd.print("0");
    lcd.print(validAddress[4],HEX);
  }
  lcd.setCursor(10,3);
  if(validAddress[5] != 0){
    lcd.print("0x");
    if (validAddress[5]<16)
      lcd.print("0");
    lcd.print(validAddress[5],HEX);
  }
}

void checkRelays(){
  GPIO = B00000000;
  if(relayTest == false){
   Wire.begin();
   Wire.beginTransmission(0x20); //begins talking to the slave device
   Wire.write(0x00); //selects the IODIRA register
   Wire.write(0x00); //this sets all port A pins to outputs
   Wire.write(0x09);//select GPIO register
   Wire.write(GPIO);//set register value-all low
   Wire.endTransmission(); //stops talking to device
  }
  relayTest = true;
  GPIO = B11111111;

  Wire.beginTransmission(RyDriverI2CAddr);
  Wire.write(0x09);//select GPIO register
  Wire.write(GPIO);//set register value-all high
  Wire.endTransmission();

  delay(5000);

  GPIO = B00000000;

  Wire.beginTransmission(RyDriverI2CAddr);
  Wire.write(0x09);//select GPIO register
  Wire.write(GPIO);//set register value-all high
  Wire.endTransmission();

  delay(1000);

  for(int x = 0; x < 8; x++){
    bitWrite(GPIO, x, 1);
    Wire.beginTransmission(RyDriverI2CAddr);
    Wire.write(0x09);//select GPIO register
    Wire.write(GPIO);//set register value-all high
    Wire.endTransmission();
    delay(1000);
  }

  for(int x = 0; x < 8; x++){
    bitWrite(GPIO, x, 0);
    Wire.beginTransmission(RyDriverI2CAddr);
    Wire.write(0x09);//select GPIO register
    Wire.write(GPIO);//set register value-all high
    Wire.endTransmission();
    delay(1000);
  }
}

void menuSetParameter(){
  if(firstPrint == true){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("-Parameter Selected-"));
    firstPrint = false;
  }

//Général
  if(!strcmp(Data, SETDAY)){
    unsigned short daySet = (unsigned short)greenhouse.rightNow(3);
    confirmVariable("SET DAY",0,daySet,31);
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      #ifdef CLOCK_DS3231
        unsigned short year = 2000+(unsigned short)greenhouse.rightNow(5);
        rtc.setDate((byte)usvariable, (byte)greenhouse.rightNow(4), year);
        getDateAndTime();
        greenhouse.setNow(rightNowValue);
        greenhouse.solarCalculations();
      #endif
      menu = MODE_DISPLAY;key = '1';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, SETMONTH)){
    unsigned short monthSet = (unsigned short)greenhouse.rightNow(4);
    confirmVariable("SET MONTH",1,monthSet,12);
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      #ifdef CLOCK_DS3231
        unsigned short year = 2000+(unsigned short)greenhouse.rightNow(5);
        rtc.setDate( (byte)greenhouse.rightNow(3),(byte)usvariable, year);
        getDateAndTime();
        greenhouse.setNow(rightNowValue);
        greenhouse.solarCalculations();
      #endif
      menu = MODE_DISPLAY;key = '1';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, SETYEAR)){
    unsigned short yearSet = (unsigned short)greenhouse.rightNow(5);
    confirmVariable("SET YEAR",0,yearSet,99);
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      #ifdef CLOCK_DS3231
        unsigned short year = 2000 + usvariable;
        rtc.setDate( (byte)greenhouse.rightNow(3), (byte)greenhouse.rightNow(4), year);
        getDateAndTime();
        greenhouse.setNow(rightNowValue);
        greenhouse.solarCalculations();
      #endif
      menu = MODE_DISPLAY;key = '1';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, SETHOUR)){
    confirmVariable("SET HOUR",0,(unsigned short)greenhouse.rightNow(2),23);
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      if(rightNow[2].value() == greenhouse.rightNow(2)){
        hourSet = usvariable;
      }
      else{
        hourSet = usvariable;
        if(hourSet > 0){
          hourSet -= 1;
        }
        else{
          hourSet = 23;
        }
      }
      for(int x = 0; x < Code_lenght;x++){
        Data[x] = SETMIN[x];
      }
      unpressedTimer = 0;
      line = 0;
    }
  }
  else if(!strcmp(Data, SETMIN)){
    confirmVariable("SET MINUTS",0,(unsigned short)greenhouse.rightNow(1),59);
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      #ifdef CLOCK_DS3231
        minSet = usvariable;
        rtc.setTime(hourSet, minSet, 0);
        menu = MODE_DISPLAY;key = '1';firstPrint = true; unpressedTimer = 0; line = 0;
      #endif
    }
  }
  //Rollup 1
#if ROLLUPS >= 1
  else if(!strcmp(Data, R1HYST)){
    confirmVariable("R1 - HYSTERESIS",R1.hyst.minimum(),R1.hyst.value(),R1.hyst.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R1.hyst.setValue(fvariable);
      menu = MODE_DISPLAY;key = '2';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R1ROTUP)){
    confirmVariable(" R1 - ROTATION (UP)",R1.rotationUp.minimum(),R1.rotationUp.value(),R1.rotationUp.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R1.rotationUp.setValue(usvariable);
      menu = MODE_DISPLAY;key = '2';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R1ROTDOWN)){
    confirmVariable("R1 - ROTATION (DOWN)",R1.rotationDown.minimum(),R1.rotationDown.value(),R1.rotationDown.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R1.rotationDown.setValue(usvariable);
      menu = MODE_DISPLAY;key = '2';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R1PAUSE)){
    confirmVariable("  R1 - PAUSE TIME",R1.pause.minimum(),R1.pause.value(),R1.pause.maximum());
    if((keyPressed == 'D')&&(R1.pause.value()!= usvariable)){
      R1.pause.setValue(usvariable);
      menu = MODE_DISPLAY;key = '2';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if ROLLUPS >= 1 && STAGES >= 1
  else if(!strcmp(Data, R1S1MOD)){
    confirmVariable("  R1 - STAGE 1 MOD",R1.stage[1].mod.minimum(),R1.stage[1].mod.minimum(),R1.stage[1].mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R1.stage[1].mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '5';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R1S1TARG)){
    confirmVariable("R1 - STAGE 1 TARGET",R1.stage[1].target.minimum(),R1.stage[1].target.value(),R1.stage[1].target.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R1.stage[1].target.setValue(usvariable);
      menu = MODE_DISPLAY;key = '5';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if ROLLUPS >= 1 && STAGES >= 2
  else if(!strcmp(Data, R1S2MOD)){
    confirmVariable("  R1 - STAGE 2 MOD",R1.stage[2].mod.minimum(),R1.stage[2].mod.value(),R1.stage[2].mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R1.stage[2].mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '5';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R1S2TARG)){
    confirmVariable("R1 - STAGE 2 TARGET",R1.stage[2].target.minimum(),R1.stage[2].target.value(),R1.stage[2].target.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R1.stage[2].target.setValue(usvariable);
      menu = MODE_DISPLAY;key = '5';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if ROLLUPS >= 1 && STAGES >= 3
  else if(!strcmp(Data, R1S3MOD)){
    confirmVariable("  R1 - STAGE 3 MOD",R1.stage[3].mod.minimum(),R1.stage[3].mod.value(),R1.stage[3].mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R1.stage[3].mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '5';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R1S3TARG)){
    confirmVariable("R1 - STAGE 3 TARGET",R1.stage[3].target.minimum(),R1.stage[3].target.value(),R1.stage[3].target.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R1.stage[3].target.setValue(usvariable);
      menu = MODE_DISPLAY;key = '5';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if ROLLUPS >= 1 && STAGES >= 4
  else if(!strcmp(Data, R1S4MOD)){
    confirmVariable("  R1 - STAGE 4 MOD",R1.stage[4].mod.minimum(),R1.stage[4].mod.value(),R1.stage[4].mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R1.stage[4].mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '5';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R1S4TARG)){
    confirmVariable("R1 - STAGE 4 TARGET",R1.stage[4].target.minimum(),R1.stage[4].target.value(),R1.stage[4].target.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R1.stage[4].target.setValue(usvariable);
      menu = MODE_DISPLAY;key = '5';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if ROLLUPS >= 1 && STAGES >= 5
  else if(!strcmp(Data, R1S5MOD)){
    confirmVariable("  R1 - STAGE 5 MOD",R1.stage[5].mod.minimum(),R1.stage[5].mod.value(),R1.stage[5].mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R1.stage[5].mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '5';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R1S5TARG)){
    confirmVariable("R1 - STAGE 5 TARGET",R1.stage[5].target.minimum(),R1.stage[5].target(),R1.stage[5].target.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R1.stage[5].target.setValue(usvariable);
      menu = MODE_DISPLAY;key = '5';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if ROLLUPS == 2
  //Rollup 2
  else if(!strcmp(Data, R2HYST)){
    confirmVariable("  R2 - HYSTERESIS",R2.hyst.minimum(),R2.hyst.value(),R2.hyst.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.hyst.setValue(fvariable);
      menu = MODE_DISPLAY;key = '3';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R2ROTUP)){
    confirmVariable(" R2 - ROTATION (UP)",R2.rotationUp.minimum(),R2.rotationUp.value(),R2.rotationUp.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.rotationUp.setValue(usvariable);
      menu = MODE_DISPLAY;key = '3';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R2ROTDOWN)){
    confirmVariable("R2 - ROTATION (DOWN)",R2.rotationDown.minimum(),R2.rotationDown.value(),R2.rotationDown.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.rotationDown.setValue(usvariable);
      menu = MODE_DISPLAY;key = '3';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R2PAUSE)){
    confirmVariable("  R2 - PAUSE TIME",R2.pause.minimum(),R2.pause.value(),R2.pause.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.pause.setValue(usvariable);
      menu = MODE_DISPLAY;key = '3';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if ROLLUPS == 2 && STAGES >= 1
  else if(!strcmp(Data, R2S1MOD)){
    confirmVariable("  R2 - STAGE 1 MOD",R2.stage[1].mod.minimum(),R2.stage[1].mod.minimum(),R2.stage[1].mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.stage[1].mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '6';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R2S1TARG)){
    confirmVariable("R2 - STAGE 1 TARGET",R2.stage[1].target.minimum(),R2.stage[1].target.value(),R2.stage[1].target.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.stage[1].target.setValue(usvariable);
      menu = MODE_DISPLAY;key = '6';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if ROLLUPS == 2 && STAGES >= 2
  else if(!strcmp(Data, R2S2MOD)){
    confirmVariable("  R2 - STAGE 2 MOD",R2.stage[2].mod.minimum(),R2.stage[2].mod.value(),R2.stage[2].mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.stage[2].mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '6';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R2S2TARG)){
    confirmVariable("R2 - STAGE 2 TARGET",R2.stage[2].target.minimum(),R2.stage[2].target.value(),R2.stage[2].target.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.stage[2].target.setValue(usvariable);
      menu = MODE_DISPLAY;key = '6';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if ROLLUPS == 2 && STAGES >= 3
  else if(!strcmp(Data, R2S3MOD)){
    confirmVariable("  R2 - STAGE 3 MOD",R2.stage[3].mod.minimum(),R2.stage[3].mod.value(),R2.stage[3].mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.stage[3].mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '6';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R2S3TARG)){
    confirmVariable("R2 - STAGE 3 TARGET",R2.stage[3].target.minimum(),R2.stage[3].target.value(),R2.stage[3].target.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.stage[3].target.setValue(usvariable);
      menu = MODE_DISPLAY;key = '6';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if ROLLUPS == 2 && STAGES >= 4
  else if(!strcmp(Data, R2S4MOD)){
    confirmVariable("  R2 - STAGE 4 MOD",R2.stage[4].mod.minimum(),R2.stage[4].mod.value(),R2.stage[4].mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.stage[4].mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '6';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R2S4TARG)){
    confirmVariable("R2 - STAGE 4 TARGET",R2.stage[4].target.minimum(),R2.stage[4].target.value(),R2.stage[4].target.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.stage[4].target.setValue(usvariable);
      menu = MODE_DISPLAY;key = '6';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if ROLLUPS == 2 && STAGES >= 5
  else if(!strcmp(Data, R2S5MOD)){
    confirmVariable("  R2 - STAGE 5 MOD",R2.stage[5].mod.minimum(),R2.stage[5].mod.value(),R2.stage[5].mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.stage[5].mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '6';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, R2S5TARG)){
    confirmVariable("R2 - STAGE 5 TARGET",R2.stage[5].target.minimum(),R2.stage[5].target.value(),R2.stage[5].target.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      R2.stage[5].target.setValue(usvariable);
      menu = MODE_DISPLAY;key = '6';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if FANS >= 1
  else if(!strcmp(Data, F1HYST)){
    confirmVariable("  F1 - HYSTERESIS", F1.hyst.minimum(), F1.hyst.value(), F1.hyst.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      F1.hyst.setValue(fvariable);
      menu = MODE_DISPLAY;key = '4';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, F1MOD)){
    confirmVariable(" F1 - MODIFICATEUR", F1.mod.minimum(), F1.mod.value(), F1.mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      F1.mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '4';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if FANS == 2
  else if(!strcmp(Data, F2HYST)){
    confirmVariable("  F2 - HYSTERESIS", F2.hyst.minimum(), F2.hyst.value(), F2.hyst.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      F2.hyst.setValue(fvariable);
      menu = MODE_DISPLAY;key = '7';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, F2MOD)){
    confirmVariable(" F2 - MODIFICATEUR", F2.mod.minimum(), F2.mod.value(), F2.mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      F2.mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '7';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if HEATERS >= 1 && FANS == 0
  else if(!strcmp(Data, H1HYST)){
    confirmVariable("  H1 - HYSTERESIS", H1.hyst.minimum(), H1.hyst.value(), H1.hyst.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      H1.hyst.setValue(fvariable);
      menu = MODE_DISPLAY;key = '4';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, H1MOD)){
    confirmVariable(" H1 - MODIFICATEUR", H1.mod.minimum(), H1.mod.value(), H1.mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      H1.mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '4';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if HEATERS >= 1 && FANS == 1
  else if(!strcmp(Data, H1HYST)){
    confirmVariable("  H1 - HYSTERESIS", H1.hyst.minimum(), H1.hyst.value(), H1.hyst.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      H1.hyst.setValue(fvariable);
      menu = MODE_DISPLAY;key = '7';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, H1MOD)){
    confirmVariable(" H1 - MODIFICATEUR", H1.mod.minimum(), H1.mod.value(), H1.mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      H1.mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '7';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif


#if HEATERS == 2
  else if(!strcmp(Data, H2HYST)){
    confirmVariable("  H2 - HYSTERESIS", H2.hyst.minimum(), H2.hyst.value(), H2.hyst.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      H2.hyst.setValue(fvariable);
      menu = MODE_DISPLAY;key = '7';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, H2MOD)){
    confirmVariable(" H2 - MODIFICATEUR", H2.mod.minimum(), H2.mod.value(), H2.mod.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      H2.mod.setValue(fvariable);
      menu = MODE_DISPLAY;key = '7';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if TIMEPOINTS >= 1
  else if(!strcmp(Data, T1TYPE)){
    confirmType("     T1 - TYPE",T1.type.value());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      for(int x = 0; x < Code_lenght;x++){
        Data[x] = T1HOUR[x];
      }
      unpressedTimer = 0;
      line = 0;
    }
  }
  else if(!strcmp(Data, T1HOUR)){
    if(typeSet == CLOCK){
      confirmVariable("     T1 - HOUR", 0,T1.hrMod.value(),23);
    }
    else{
      confirmVariable("     T1 - HOUR", -23,T1.hrMod.value(),23);
    }
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      hourSet = svariable;
      for(int x = 0; x < Code_lenght;x++){
        Data[x] = T1MIN[x];
      }
      unpressedTimer = 0;
      line = 0;
    }
  }
  else if(!strcmp(Data, T1MIN)){
    if(typeSet == CLOCK){
      confirmVariable("    T1 - MINUTS", 0,T1.mnMod.value(),59);
    }
    else{
      confirmVariable("    T1 - MINUTS", -59,T1.mnMod.value(),59);
    }
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      minSet = svariable;
      T1.type.setValue(typeSet);
      T1.setTimepoint(hourSet, minSet);
      menu = MODE_DISPLAY;key = '8';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, T1HEATT)){
    if(greenhouse.weather() == SUN){
      confirmVariable("T1 - HEAT TEMP-SUN", T1.coolingTemp.minimum(),T1.heatingTemp.value(),T1.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T1.heatingTemp.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
    else if(greenhouse.weather() == CLOUD){
      confirmVariable("T1 - HEAT TEMP-CLOUD", T1.coolingTemp.minimum(),T1.heatingTempCloud.value(),T1.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T1.heatingTempCloud.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
  }
  else if(!strcmp(Data, T1COOLT)){
    if(greenhouse.weather() == SUN){
      confirmVariable("T1 - COOL TEMP-SUN", T1.coolingTemp.minimum(),T1.coolingTemp.value(),T1.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T1.coolingTemp.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
    else if(greenhouse.weather() == CLOUD){
      confirmVariable("T1 - COOL TEMP-CLOUD", T1.coolingTemp.minimum(),T1.coolingTempCloud.value(),T1.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T1.coolingTempCloud.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
  }
  else if(!strcmp(Data, T1RAMP)){
    confirmVariable("   T1 - RAMPING", T1.ramping.minimum(),T1.ramping.value(),T1.ramping.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      T1.ramping.setValue(usvariable);
      menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if TIMEPOINTS >= 2
  else if(!strcmp(Data, T2TYPE)){
    confirmType("     T2 - TYPE",T2.type.value());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      for(int x = 0; x < Code_lenght;x++){
        Data[x] = T2HOUR[x];
      }
      unpressedTimer = 0;
      line = 0;
    }
  }
  else if(!strcmp(Data, T2HOUR)){
    if(typeSet == CLOCK){
      confirmVariable("     T2 - HOUR", 0,T2.hrMod.value(),23);
    }
    else{
      confirmVariable("     T2 - HOUR", -23,T2.hrMod.value(),23);
    }
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      hourSet = svariable;
      for(int x = 0; x < Code_lenght;x++){
        Data[x] = T2MIN[x];
      }
      unpressedTimer = 0;
      line = 0;
    }
  }
  else if(!strcmp(Data, T2MIN)){
    if(typeSet == CLOCK){
      confirmVariable("    T2 - MINUTS", 0,T2.mnMod.value(),59);
    }
    else{
      confirmVariable("    T2 - MINUTS", -59,T2.mnMod.value(),59);
    }
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      minSet = svariable;
      T2.type.setValue(typeSet);
      T2.setTimepoint(hourSet, minSet);
      menu = MODE_DISPLAY;key = '8';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, T2HEATT)){
    if(greenhouse.weather() == SUN){
      confirmVariable("T2 - HEAT TEMP-SUN", T2.coolingTemp.minimum(),T2.heatingTemp.value(),T2.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T2.heatingTemp.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
    else if(greenhouse.weather() == CLOUD){
      confirmVariable("T2 - HEAT TEMP-CLOUD", T2.coolingTemp.minimum(),T2.heatingTempCloud.value(),T2.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T2.heatingTempCloud.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
  }
  else if(!strcmp(Data, T2COOLT)){
    if(greenhouse.weather() == SUN){
      confirmVariable("T2 - COOL TEMP-SUN", T2.coolingTemp.minimum(),T2.coolingTemp.value(),T2.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T2.coolingTemp.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
    else if(greenhouse.weather() == CLOUD){
      confirmVariable("T2 - COOL TEMP-CLOUD", T2.coolingTemp.minimum(),T2.coolingTempCloud.value(),T2.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T2.coolingTempCloud.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
  }
  else if(!strcmp(Data, T2RAMP)){
    confirmVariable("   T2 - RAMPING", T2.ramping.minimum(),T2.ramping.value(),T2.ramping.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      T2.ramping.setValue(usvariable);
      menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if TIMEPOINTS >= 3
  else if(!strcmp(Data, T3TYPE)){
    confirmType("     T3 - TYPE",T3.type.value());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      for(int x = 0; x < Code_lenght;x++){
        Data[x] = T3HOUR[x];
      }
      unpressedTimer = 0;
      line = 0;
    }
  }
  else if(!strcmp(Data, T3HOUR)){
    if(typeSet == CLOCK){
      confirmVariable("     T3 - HOUR", 0,T3.hrMod.value(),23);
    }
    else{
      confirmVariable("     T3 - HOUR", -23,T3.hrMod.value(),23);
    }
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      hourSet = svariable;
      for(int x = 0; x < Code_lenght;x++){
        Data[x] = T3MIN[x];
      }
      unpressedTimer = 0;
      line = 0;
    }
  }
  else if(!strcmp(Data, T3MIN)){
    if(typeSet == CLOCK){
      confirmVariable("    T3 - MINUTS", 0,T3.mnMod.value(),59);
    }
    else{
      confirmVariable("    T3 - MINUTS", -59,T3.mnMod.value(),59);
    }
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      minSet = svariable;
      T3.type.setValue(typeSet);
      T3.setTimepoint(hourSet, minSet);
      menu = MODE_DISPLAY;key = '8';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, T3HEATT)){
    if(greenhouse.weather() == SUN){
      confirmVariable("T3 - HEAT TEMP-SUN", T3.coolingTemp.minimum(),T3.heatingTemp.value(),T3.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T3.heatingTemp.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
    else if(greenhouse.weather() == CLOUD){
      confirmVariable("T3 - HEAT TEMP-CLOUD", T3.coolingTemp.minimum(),T3.heatingTempCloud.value(),T3.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T3.heatingTempCloud.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
  }
  else if(!strcmp(Data, T3COOLT)){
    if(greenhouse.weather() == SUN){
      confirmVariable("T3 - COOL TEMP-SUN", T3.coolingTemp.minimum(),T3.coolingTemp.value(),T3.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T3.coolingTemp.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
    else if(greenhouse.weather() == CLOUD){
      confirmVariable("T3 - COOL TEMP-CLOUD", T3.coolingTemp.minimum(),T3.coolingTempCloud.value(),T3.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T3.coolingTempCloud.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
  }
  else if(!strcmp(Data, T3RAMP)){
    confirmVariable("   T3 - RAMPING", T3.ramping.minimum(),T3.ramping.value(),T3.ramping.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      T3.ramping.setValue(usvariable);
      menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if TIMEPOINTS >= 4
  else if(!strcmp(Data, T4TYPE)){
    confirmType("     T4 - TYPE",T4.type.value());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      for(int x = 0; x < Code_lenght;x++){
        Data[x] = T4HOUR[x];
      }
      unpressedTimer = 0;
      line = 0;
    }
  }
  else if(!strcmp(Data, T4HOUR)){
    if(typeSet == CLOCK){
      confirmVariable("     T4 - HOUR", 0,T4.hrMod.value(),23);
    }
    else{
      confirmVariable("     T4 - HOUR", -23,T4.hrMod.value(),23);
    }
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      hourSet = svariable;
      for(int x = 0; x < Code_lenght;x++){
        Data[x] = T4MIN[x];
      }
      unpressedTimer = 0;
      line = 0;
    }
  }
  else if(!strcmp(Data, T4MIN)){
    if(typeSet == CLOCK){
      confirmVariable("    T4 - MINUTS", 0,T4.mnMod.value(),59);
    }
    else{
      confirmVariable("    T4 - MINUTS", -59,T4.mnMod.value(),59);
    }
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      minSet = svariable;
      T4.type.setValue(typeSet);
      T4.setTimepoint(hourSet, minSet);
      menu = MODE_DISPLAY;key = '8';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, T3HEATT)){
    if(greenhouse.weather() == SUN){
      confirmVariable("T3 - HEAT TEMP-SUN", T3.coolingTemp.minimum(),T3.heatingTemp.value(),T3.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T3.heatingTemp.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
    else if(greenhouse.weather() == CLOUD){
      confirmVariable("T3 - HEAT TEMP-CLOUD", T3.coolingTemp.minimum(),T3.heatingTempCloud.value(),T3.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T3.heatingTempCloud.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
  }
  else if(!strcmp(Data, T3COOLT)){
    if(greenhouse.weather() == SUN){
      confirmVariable("T3 - COOL TEMP-SUN", T3.coolingTemp.minimum(),T3.coolingTemp.value(),T3.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T3.coolingTemp.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
    else if(greenhouse.weather() == CLOUD){
      confirmVariable("T3 - COOL TEMP-CLOUD", T3.coolingTemp.minimum(),T3.coolingTempCloud.value(),T3.coolingTemp.maximum());
      if((keyPressed == 'D')&&(unpressedTimer > 1000)){
        T3.coolingTempCloud.setValue(fvariable);
        menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
      }
    }
  }
  else if(!strcmp(Data, T4RAMP)){
    confirmVariable("   T4 - RAMPING", T4.ramping.minimum(),T4.ramping.value(),T4.ramping.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      T4.ramping.setValue(usvariable);
      menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
#if TIMEPOINTS >= 5
  else if(!strcmp(Data, T5TYPE)){
    confirmType("     T5 - TYPE",T5.type.value());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      for(int x = 0; x < Code_lenght;x++){
        Data[x] = T5HOUR[x];
        Serial.print("");
        Serial.print(Data[x]);
      }
      Serial.println("");
      unpressedTimer = 0;
      line = 0;
    }
  }
  else if(!strcmp(Data, T5HOUR)){
    if(typeSet == CLOCK){
      confirmVariable("     T5 - HOUR", 0,T5.hrMod.value(),23);
    }
    else{
      confirmVariable("     T5 - HOUR", -23,T5.hrMod.value(),23);
    }
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      hourSet = svariable;
      for(int x = 0; x < Code_lenght;x++){
        Data[x] = T5MIN[x];
      }
      unpressedTimer = 0;
      line = 0;
    }
  }
  else if(!strcmp(Data, T5MIN)){
    if(typeSet == CLOCK){
      confirmVariable("    T5 - MINUTS", 0,T5.mnMod.value(),59);
    }
    else{
      confirmVariable("    T5 - MINUTS", -59,T5.mnMod.value(),59);
    }
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      minSet = svariable;
      T5.type.setValue(typeSet);
      T5.setTimepoint(hourSet, minSet);
      menu = MODE_DISPLAY;key = '8';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, T5HEATT)){
    confirmVariable("  T5 - HEAT TEMP", T5.coolingTemp.minimum(),T5.heatingTemp.value(),T5.coolingTemp.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      T5.heatingTemp.setValue(fvariable);
      menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, T5COOLT)){
    confirmVariable("  T5 - COOL TEMP", T5.coolingTemp.minimum(),T5.coolingTemp.value(),T5.coolingTemp.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      T5.coolingTemp.setValue(fvariable);
      menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
  else if(!strcmp(Data, T5RAMP)){
    confirmVariable("   T5 - RAMPING", T5.ramping.minimum(),T5.ramping.value(),T5.ramping.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      T5.ramping.setValue(usvariable);
      menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }

  else if(!strcmp(Data, T5RAMP)){
    confirmVariable("   T5 - RAMPING", T5.ramping.minimum(),T5.ramping.value(),T5.ramping.maximum());
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      T5.ramping.setValue(usvariable);
      menu = MODE_DISPLAY;key = '9';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }
#endif
  else if(!strcmp(Data, TESTRELAYS)){
    checkRelays();
  }

  else if(!strcmp(Data, TESTIIC)){
    checkIIC();
    if((keyPressed == 'D')&&(unpressedTimer > 1000)){
      menu = MODE_DISPLAY;key = '1';firstPrint = true; unpressedTimer = 0; line = 0;
    }
  }

  else{
  lcd.noBlink();menu = MODE_PROGRAM;firstPrint = true;line = 0;
  }
}

void clearData()
{
for(int x = 0; x < Code_lenght-1; x++)
  {   // This can be used for any array size,
    Data[x] = 0; //clear array for new data
  }
  return;
}


void lcdDisplay(){
   #ifdef KEYPAD_DISPLAY
    keyPressed = keypad.getKey();

    switch(keyPressed){
      case '1' : key = keyPressed;if(menu == MODE_DISPLAY){firstPrint = true;}break;
      case '2' : key = keyPressed;if(menu == MODE_DISPLAY){firstPrint = true;}break;
      case '3' : key = keyPressed;if(menu == MODE_DISPLAY){firstPrint = true;}break;
      case '4' : key = keyPressed;if(menu == MODE_DISPLAY){firstPrint = true;}break;
      case '5' : key = keyPressed;if(menu == MODE_DISPLAY){firstPrint = true;}break;
      case '6' : key = keyPressed;if(menu == MODE_DISPLAY){firstPrint = true;}break;
      case '7' : key = keyPressed;if(menu == MODE_DISPLAY){firstPrint = true;}break;
      case '8' : key = keyPressed;if(menu == MODE_DISPLAY){firstPrint = true;}break;
      case '9' : key = keyPressed;if(menu == MODE_DISPLAY){firstPrint = true;}break;
      case '*' : line--;break;
      case '0' : break;
      case '#' : line++;break;
      case 'A' : menu = MODE_DISPLAY;firstPrint = true; line = 0; key = '1'; lcd.noBlink(); clearData(); homeDisplay();break;
      case 'B' : menu = MODE_PROGRAM;firstPrint = true; line = 0; lcd.noBlink();clearData();data_count = 0; break;
      case 'C' : menu = MODE_ACTION;firstPrint = true; action = 0; line = 0;lcd.noBlink();clearData();data_count = 0;break;
    }
    if(keyPressed != NO_KEY && keyPressed != 'D'){
      unpressedTimer = 0;
    }

    if(menu == MODE_DISPLAY){
      menuDisplay();
      if(unpressedTimer > 15000){
        key = '1';firstPrint = true;
        unpressedTimer = 0;
      }
    }

    if(menu == MODE_PROGRAM){
      menuProgram();
      if(unpressedTimer > 15000){
        menu = MODE_DISPLAY;
        key = '1';firstPrint = true;
        unpressedTimer = 0;
        line = 0;
        lcd.noBlink();
        clearData();
      }

    }
    if(menu == SET_PARAMETER){
      menuSetParameter();
      if(unpressedTimer > 15000){
        menu = MODE_DISPLAY;
        key = '1';firstPrint = true;
        unpressedTimer = 0;
        line = 0;
        clearData();
      }
    }

    if(menu == MODE_ACTION){
      menuAction();
      if(unpressedTimer > 15000){
        menu = MODE_DISPLAY;
        key = '1';firstPrint = true;
      }
    }

  #endif

  #ifndef KEYPAD_DISPLAY
  firstPrint = true;
  lcd.clear();
    homeDisplay();
  #endif
}
