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

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define I2CADDR_LCD    0x27
#define BACKLIGHT_PIN     3
LiquidCrystal_I2C  lcd(I2CADDR_LCD, 2, 1, 0, 4, 5, 6, 7);


//#define KEYPAD_DISPLAY
#ifdef KEYPAD_DISPLAY
#include <Keypad_I2C.h>
#include <Keypad.h>
  #define I2CADDR_KEY 0x24
  const byte ROWS = 4; //four rows
  const byte COLS = 4; //three columns
  char keys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
  };
  
  // Digitran keypad, bit numbers of PCF8574 i/o port
  byte rowPins[ROWS] = {7,6,5,4}; //connect to the row pinouts of the keypad
  byte colPins[COLS] = {3,2,1,0}; //connect to the column pinouts of the keypad
  
  Keypad_I2C keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR_KEY, PCF8574 );
#endif

char key = '1';
boolean firstPrint = true;
short line = 0;
short maxLine;

//Flashing display
byte flashCount1 = 0;
byte flashCount2 = 0;

void serialPrintDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
  Serial.print("0");
}

void lcdPrintDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
  lcd.print("0");
  lcd.print(digits);
}

void initLCD(byte length, byte width){
  
  lcd.begin(length, width);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.clear();
}

void lcdPrintOutput(String item, byte _column, byte _row, byte pin){
    //Fan
    lcd.setCursor(_column, _row); lcd.print(F("          "));
    lcd.setCursor(_column, _row); lcd.print(item);
    
    #ifdef I2C_OUTPUTS
      if(mcp.digitalRead(pin) == HIGH){
        lcd.print("ON");
      }
    #endif

    #ifdef IOS_OUTPUTS
      if(digitalRead(pin) == HIGH){
        lcd.print("ON");
      }
    #endif
        
    else{
      lcd.print("OFF");
    
    }
}
void lcdPrintRollups(String side, String opening, String closing, byte _column, byte _row, Rollup rollup, byte* counter){
    //East rollup
    lcd.setCursor(_column, _row); lcd.print(F("          "));
    lcd.setCursor(_column, _row);
    if(rollup.incrementCounter() == OFF_VAL){lcd.print(F(""));}
    else if (rollup.opening() == true){lcd.print(opening);}
    else if (rollup.closing() == true){lcd.print(closing);}
    else if(((rollup.closing() == false)&&(rollup.closingCycle() == true))||((rollup.opening() == false)&&(rollup.openingCycle() == true))){
      if(*counter == 0){
        lcd.print(side); lcd.print(rollup.incrementCounter());lcd.print("%");
        *counter += 1;
      }
      else if(*counter == 1){
        lcd.write("          ");
        *counter = 0;
      }
    }
    else{lcd.print(side); lcd.print(rollup1.incrementCounter());lcd.print("%");}
}
void lcdPrintTemp(byte _row){
    
    lcd.setCursor(0,_row); lcd.print(F("         "));
    lcd.setCursor(0,_row); 
    if(sensorFailure == false){lcd.print(greenhouseTemperature.value()); lcd.print(F("C"));}
    else if(sensorFailure == true){lcd.print("!!!");}
    lcd.setCursor(9,_row); lcd.print("|HR:");lcd.print((short)(greenhouseHumidity.value())); lcd.print(F("%"));


}

void lcdPrintTempCible(){
}

void lcdPrintTime(byte _row){
    lcd.setCursor(0,_row); lcdPrintDigits(rightNow[HOUR].value()); lcd.print(F(":")); lcdPrintDigits(rightNow[MINUT].value()); lcd.print(F("(")), lcd.print(greenhouse._timepoint); lcd.print(F(")")), 
    lcd.setCursor(9,_row); lcd.print("|TT:");lcd.print((short)(greenhouse._heatingTemp)); lcd.print(F("-"));lcd.print((short)(greenhouse._coolingTemp)); lcd.print(F("C"));
}

void homeDisplay(){
  
  if(firstPrint == true){
    lcd.clear();
    lcdPrintTemp(0); 
    lcdPrintTime(1);
    lcdPrintRollups("E: ","OPENING", "CLOSING", 0, 2, rollup1, &flashCount1);
    lcdPrintRollups("|W: ","|OPENING", "|CLOSING", 9, 2, rollup2, &flashCount2);
    lcdPrintOutput("F1: ", 0, 3, FAN_PIN);
    lcdPrintOutput("|H1: ", 9, 3, HEATER_PIN);
    firstPrint = false;
  }
  
  if(greenhouseTemperature.valueHasChanged()){
    lcdPrintTemp(0); 
    greenhouseTemperature.updateLastValue();
  }
  for(int x = 1; x < 6; x++){
    if (rightNow[x].valueHasChanged()){
      lcdPrintTime(1);
      rightNow[x].updateLastValue();
    }
  }
  
  lcdPrintRollups("E: ","OPENING", "CLOSING", 0, 2, rollup1, &flashCount1);
  lcdPrintRollups("|W: ","|OPENING", "|CLOSING", 9, 2, rollup2, &flashCount2);
  lcdPrintOutput("F1: ", 0, 3, FAN_PIN);
  lcdPrintOutput("|H1: ", 9, 3, HEATER_PIN);
}
void printHeader(String header, short maxLineOfMenu){
  
  //clear + print header
  if(firstPrint){
    lcd.clear();
    lcd.setCursor(0,0);lcd.print(header);
    firstPrint = false;
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

  printHeader("ROLLUP - PARAMETERS",4);
  adjustLine();

  //print content
  for(int x = 1; x < 4; x++){
    int writeLine = x+line;
    if (writeLine > maxLine){
      writeLine = maxLine;
    }
    switch(writeLine){
      case 1: lcd.setCursor(0,x);lcd.print("UP: "); lcd.print(rollup.rotationUp());lcd.print("s"); lcd.setCursor(9,x);lcd.print("|DOWN: "); lcd.print(rollup.rotationDown());lcd.print("s");break;
      case 2: lcd.setCursor(0,x);lcd.print("Pause: "); lcd.print(rollup.pause());lcd.print("s");break;
      case 3:  lcd.setCursor(0,x);lcd.print("Hyst:  ");  lcd.print(rollup.hyst());lcd.print("C");break;
      case 4:  lcd.setCursor(0,x);lcd.print("Something else");break;
    }
  }
}

void stageDisplay(Rollup rollup){
  
  float stageTemp1 = greenhouse._coolingTemp + rollup.stageMod(1);
  float stageTemp2 = greenhouse._coolingTemp + rollup.stageMod(2);
  float stageTemp3 = greenhouse._coolingTemp + rollup.stageMod(3);
  float stageTemp4 = greenhouse._coolingTemp + rollup.stageMod(4);

  printHeader("ROLLUP - STAGES",4);
  adjustLine();

  //print content
  for(int x = 1; x < 4; x++){
    int writeLine = x+line;
    if (writeLine > maxLine){
      writeLine = maxLine;
    }
    switch(writeLine){
      case 1:  lcd.setCursor(0,x);lcd.print("S1: "); lcd.print(stageTemp1);lcd.print("-");lcd.print(stageTemp2);lcd.print("-"); lcd.print(rollup.stageTarget(1)); lcd.print("%");break;
      case 2:  lcd.setCursor(0,x);lcd.print("S2: "); lcd.print(stageTemp2);lcd.print("-");lcd.print(stageTemp3);lcd.print("-"); lcd.print(rollup.stageTarget(2)); lcd.print("%");break;
      case 3:  lcd.setCursor(0,x);lcd.print("S3: "); lcd.print(stageTemp3);lcd.print("-");lcd.print(stageTemp4);lcd.print("-"); lcd.print(rollup.stageTarget(3)); lcd.print("%");break;
      case 4:  lcd.setCursor(0,x);lcd.print("S4: "); lcd.print(stageTemp4);lcd.print("- ... ");lcd.print("-"); lcd.print(rollup.stageTarget(4)); lcd.print("%");break;
    }
  }
}

void fanDisplay(Fan fan){
  
  float onTemp = greenhouse._coolingTemp + fan.tempParameter();
  float offTemp = greenhouse._coolingTemp + fan.tempParameter() - fan.hyst();
  
  printHeader("FAN - PARAMETERS",3);
  adjustLine();

  //print content
  for(int x = 1; x < 4; x++){
    int writeLine = x+line;
    if (writeLine > maxLine){
      writeLine = maxLine;
    }
    switch(writeLine){
      case 1: lcd.setCursor(0,1);lcd.print("Activ. temp: "); lcd.print(onTemp);lcd.print("C");break;
      case 2: lcd.setCursor(0,2);lcd.print("Shut. temp: "); lcd.print(offTemp);lcd.print("C");break;
      case 3: lcd.setCursor(0,3);lcd.print("MOD:"); if(fan.tempParameter() >= 0){lcd.print(" ");}lcd.print(fan.tempParameter()); lcd.setCursor(9,3);lcd.print("|HYST: "); lcd.print(fan.hyst());break;
    }
  }
}

void heaterDisplay(Heater heater){
  
  float onTemp = greenhouse._heatingTemp + heater.tempParameter();
  float offTemp = greenhouse._heatingTemp + heater.tempParameter() + heater.hyst();

  printHeader("HEATER - PARAMETERS",3);
  adjustLine();

  //print content
  for(int x = 1; x < 4; x++){
    int writeLine = x+line;
    if (writeLine > maxLine){
      writeLine = maxLine;
    }
    switch(writeLine){
      case 1: lcd.setCursor(0,1);lcd.print("Activ. temp: "); lcd.print(onTemp);lcd.print("C");break;
      case 2: lcd.setCursor(0,2);lcd.print("Shut. temp: "); lcd.print(offTemp);lcd.print("C");break;
      case 3: lcd.setCursor(0,3);lcd.print("MOD:"); if(heater.tempParameter() >= 0){lcd.print(" ");}lcd.print(heater.tempParameter()); lcd.setCursor(9,3);lcd.print("|HYST: "); lcd.print(heater.hyst());break;
    }
  }
}

void timepointsDisplay(){
  printHeader("TIMEPOINTS - TIME",4);
  adjustLine();

  //print content
  for(int x = 1; x < 4; x++){
    int writeLine = x+line;
    if (writeLine > maxLine){
      writeLine = maxLine;
    }
    switch(writeLine){
      case 1: lcd.setCursor(0,x);lcd.print("TP1: "); lcdPrintDigits(timepoint1.hr());lcd.print(":");lcdPrintDigits(timepoint1.mn());lcd.print(" - "); lcdPrintDigits(timepoint2.hr());lcd.print(":");lcdPrintDigits(timepoint2.mn());break;
      case 2: lcd.setCursor(0,x);lcd.print("TP2: "); lcdPrintDigits(timepoint2.hr());lcd.print(":");lcdPrintDigits(timepoint2.mn());lcd.print(" - "); lcdPrintDigits(timepoint3.hr());lcd.print(":");lcdPrintDigits(timepoint3.mn());break;
      case 3: lcd.setCursor(0,x);lcd.print("TP3: "); lcdPrintDigits(timepoint3.hr());lcd.print(":");lcdPrintDigits(timepoint3.mn());lcd.print(" - "); lcdPrintDigits(timepoint4.hr());lcd.print(":");lcdPrintDigits(timepoint4.mn());break;
      case 4: lcd.setCursor(0,x);lcd.print("TP4: "); lcdPrintDigits(timepoint4.hr());lcd.print(":");lcdPrintDigits(timepoint4.mn());lcd.print(" - "); lcdPrintDigits(timepoint1.hr());lcd.print(":");lcdPrintDigits(timepoint1.mn());break;
   }
  }
}

void temperaturesDisplay(){
  printHeader("TIMEPOINTS - TEMP",4);
  adjustLine();

  //print content
  for(int x = 1; x < 4; x++){
    int writeLine = x+line;
    if (writeLine > maxLine){
      writeLine = maxLine;
    }
    switch(writeLine){
      case 1: lcd.setCursor(0,x);lcd.print("TP1: "); lcd.print(timepoint1.heatingTemp());lcd.print("C - "); lcd.print(timepoint1.coolingTemp());lcd.print("C");break; 
      case 2: lcd.setCursor(0,x);lcd.print("TP2: "); lcd.print(timepoint2.heatingTemp());lcd.print("C - "); lcd.print(timepoint2.coolingTemp());lcd.print("C");break; 
      case 3: lcd.setCursor(0,x);lcd.print("TP3: "); lcd.print(timepoint3.heatingTemp());lcd.print("C - "); lcd.print(timepoint3.coolingTemp());lcd.print("C");break;
      case 4: lcd.setCursor(0,x);lcd.print("TP4: "); lcd.print(timepoint4.heatingTemp());lcd.print("C - "); lcd.print(timepoint4.coolingTemp());lcd.print("C");break; }
  }
}

void geoDisplay(){
  
}

void sensorsDisplay(){
  
}

void lcdDisplay(){

  #ifdef KEYPAD_DISPLAY
    char keyPressed = keypad.getKey();
    
    switch(keyPressed){
      case '*' : line--;break;
      case '0' : break;
      case '#' : line++;break;
    }
    if (keyPressed != NO_KEY && keyPressed != '*' && keyPressed != '#'){key = keyPressed;}
    if (keyPressed != NO_KEY){firstPrint = true;}
    
    switch (key){
      case '1' : homeDisplay(); break;
      case '2' : rollupDisplay(rollup1);break;
      case '3' : stageDisplay(rollup1);break;
      case '4' : stageDisplay(rollup2);break;
      case '5' : fanDisplay(fan1);break;
      case '6' : heaterDisplay(heater1);break;
      case '7' : timepointsDisplay();break;
      case '8' : temperaturesDisplay();break;
      case '9' : geoDisplay();break;
    }
  #endif

  #ifndef KEYPAD_DISPLAY
    homeDisplay();
  #endif
}

