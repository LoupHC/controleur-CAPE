#include "Keypad.h"
#include <LiquidCrystal_I2C.h>


// Create lcd object using NewLiquidCrystal lib
#define I2C_ADDR    0x27
#define BACKLIGHT_PIN     3
LiquidCrystal_I2C  lcd(I2C_ADDR, 2, 1, 0, 4, 5, 6, 7);


byte bitCount1 = 0;
byte bitCount2 = 0;

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
    if(digitalRead(pin) == HIGH){
      lcd.print("ON");
    }
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
    if(sensorFailure == false){lcd.print(greenhouseTemperature); lcd.print(F("C"));}
    else if(sensorFailure == true){lcd.print("!!!");}
    lcd.setCursor(9,_row); lcd.print("|TT:");lcd.print((short)(greenhouse._heatingTemp)); lcd.print(F("-"));lcd.print((short)(greenhouse._coolingTemp)); lcd.print(F("C"));

}

void lcdPrintTempCible(){
}

void lcdPrintTime(byte _row){
    lcd.setCursor(0,_row); lcdPrintDigits(rightNow[HOUR]); lcd.print(F(":")); lcdPrintDigits(rightNow[MINUT]);
    lcd.setCursor(9,_row); lcd.print(F("|TP: ")), lcd.print(greenhouse._timepoint);
}



void lcdDisplay(){
  lcdPrintTemp(0);
  lcdPrintTime(1);
  lcdPrintRollups("E: ","OPENING", "CLOSING", 0, 2, rollup1, &bitCount1);
  lcdPrintRollups("|W: ","|OPENING", "|CLOSING", 9, 2, rollup2, &bitCount2);
  lcdPrintOutput("F1: ", 0, 3, FAN_PIN);
  lcdPrintOutput("|H1: ", 9, 3, HEATER_PIN);
}
