/*
  Created by Loup Hébert-Chartrand, December 5, 2019.
  Released into the public domain.
  
  This software is furnished "as is", without technical support, and with no 
  warranty, express or implied, as to its usefulness for any purpose.
*/

#include "Arduino.h"
#include "GreenhouseLib.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DS3231.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>


//********************PINOUT**************************
#define WEST_OPENING_PIN 4 //connect this pin to the opening relay (west motor)
#define WEST_CLOSING_PIN 5 //connect this pin to the closing relay (west motor)
#define EAST_OPENING_PIN 6 //connect this pin to the opening relay (east motor)
#define EAST_CLOSING_PIN 7 //connect this pin to the closing relay (east motor)
#define FAN_PIN          2 //Connect this pin to the fan relay
#define HEATER_PIN       3 //connect this pin to the heater relay
#define ONE_WIRE_BUS     A1 //connect this pin to the DS18B20 data line

//********************MAIN PARAMETERS**************************
const int TIMEZONE = -5; //PST
const float LATITUDE = 45.50, LONGITUDE = -73.56; // set your position here
const byte TIMEPOINTS = 5;
const byte ROLLUPS = 2;
const byte FANS = 1;
const byte HEATERS = 1;

//********************OBJECTS**************************// Create lcd object using NewLiquidCrystal lib
#define I2C_ADDR    0x27              // Define I2C Address where the PCF8574A is
#define BACKLIGHT_PIN     3
LiquidCrystal_I2C  lcd(I2C_ADDR, 2, 1, 0, 4, 5, 6, 7);

//Create DS18B20 object
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//Create a RTC object
DS3231  rtc(SDA, SCL);                // Init the DS3231 using the hardware interface
Time  t;

Greenhouse greenhouse(TIMEZONE, LATITUDE, LONGITUDE, TIMEPOINTS, ROLLUPS, FANS, HEATERS);

//********************POINTERS**************************

Timepoint &timepoint1 = greenhouse.timepoint[0];
Timepoint &timepoint2 = greenhouse.timepoint[1];
Timepoint &timepoint3 = greenhouse.timepoint[2];
Timepoint &timepoint4 = greenhouse.timepoint[3];
Timepoint &timepoint5 = greenhouse.timepoint[4];
Rollup &rollup1 = greenhouse.rollup[0];
Rollup &rollup2 = greenhouse.rollup[1];
Fan &fan1 = greenhouse.fan[0];
Heater &heater1 = greenhouse.heater[0];

//********************VARIABLES**************************

byte rightNow[6];
float greenhouseTemperature;

boolean control = true;     //tells if in mode control or menu

//***************************************************
//********************SETUP**************************
//***************************************************

void setup() {
  Serial.begin(9600);
  initLCD(20,4);
  sensors.begin();  //start communication with temp probe
  rtc.begin();      //start communication with temp probe
  
  // The following lines can be uncommented to set the date and time
  //rtc.setTime(18, 59, 45);     // Set the time to 12:00:00 (24hr format) MUST BE SET TO WINTER TIME
  //rtc.setDate(1, 1, 2017);   // Set the date to January 1st, 2014
  
  getDateAndTime();                             //get RTC values
  greenhouse.setNow(rightNow[6]);               //set time with greenhouse object
  greenhouse.solarCalculations();               //calculate hour saving, sunrise, sunset
  
//********************Parameters*******************
  //Timezones parameters :
    //Type : SR (starting time set according to sunrise time) (SS for sunset, CLOCK for manual)
    //Time adjustement : -30 (beginning of timezone is 30 min before sunrise)(-60 to +60)
    //FOR CLOCK MODE ONLY : 12(hour)25(minuts)
    //Target temperature : 20 (target temp for the duration of the timezone)

   //initOutputs parameters :
    //Mode : MAN_TEMP (routine operating on stages, targetting spec increment)
    //Relay type : ACT_HIGH (relay active when pin is high)
    //Opening pin : connect opening relay to this pin
    //Closing pin : connect closing relay to this pin

   //Rollup parameters :
    //Temperature mod : 0C (adjust to external target temperature)
    //hysteresis : 1C
    //Rotation time (Up): 100 sec (for full opening) (0 to 255 seconds)
    //Rotation time (Down): 100 sec (for full closing) (0 to 255 seconds)
    //Increments : 100
    //Pause between rotation : 5 (0 to 255 seconds)
    //Safety mode : ON (safety opening cycle every 30 min even if considered fully open)
   
   //Stages parameters:
    //Stage number : 1 (max. 4)
    //Stage temperature mod : 0C (adjust to external target temperature)(-10 to +10)
    //Target increment : 10 (0 to max. increment)
    
  //********************Timepoint*******************
  timepoint1.setParameters(SR, -30, 18, 20);        //Ex. 30 minuts before sunrise : heatingReference = 18, coolingReference = 20;
  timepoint2. setParameters(SR, 0, 18, 22);
  timepoint3.setParameters(CLOCK, 12, 30, 20, 24);  //Ex. At 12h30 : heatingReference = 20, coolingReference = 24;
  timepoint4.setParameters(SS, -60, 16, 18);
  timepoint5.setParameters(SS, 0, 17,  19);
  

//********************Rollup 1 (east)*******************
  rollup1.initOutputs(MAN_TEMP, ACT_HIGH, EAST_OPENING_PIN, EAST_CLOSING_PIN);

  rollup1.setParameters(0, 1, 189, 150, 100, 30, true); //Ex. coolstages begin at cooling temperature+0, hysteresis is 1C, opening time is 189 sec, closing time is 150 sec, theres 100 increments(%), 30 seconds of pause between moves

  rollup1.initStage(1,0,10);                            //Ex.At cooling temperature+0, rollup is set to 10%
  rollup1.initStage(2,1,30);
  rollup1.initStage(3,2,50);
  rollup1.initStage(4,3,100);
  
//********************Rollup 2 (west)*******************
  rollup2.initOutputs(MAN_TEMP, ACT_HIGH, WEST_OPENING_PIN, WEST_CLOSING_PIN);
  
  rollup2.setParameters(0, 1, 189, 150, 100, 30, true);

  rollup2.initStage(1,0,10);
  rollup2.initStage(2,1,20);
  rollup2.initStage(3,2,30);
  rollup2.initStage(4,3,50);

//*************************Fan**************************
  fan1.initOutput(VAR_TEMP, FAN_PIN);
  fan1.setParameters(3, 1, true);                       //At cooling temperature+3, fan starts, hysteresis is 1C   
  

//************************Heater************************    
  heater1.initOutput(VAR_TEMP, HEATER_PIN);
  heater1.setParameters(-1, 2, true);                  //At heating temperature -1, furnace starts, hysteresis is 2C

//********************Init variables***********************
  greenhouse.startingParameters();  //actual time, timepoint and targetTemp;
  
}


//***************************************************
//*********************LOOP**************************
//***************************************************

void loop() {
  getDateAndTime();                                             //actual time
  greenhouseTemperature = getGreenhouseTemp();                  //actual temperature
  lcdDisplay();                                                 //display infos on LCD screen
  greenhouse.fullRoutine(rightNow[6], greenhouseTemperature);   //timepoint ad target temperatures definitions, outputs activations
}


//***************************************************
//*********************MACROS**************************
//***************************************************


void getDateAndTime(){
  t = rtc.getTime();
  rightNow[5] = t.year-2000;
  rightNow[4] = t.mon;
  rightNow[3] = t.date;
  rightNow[HEURE] = t.hour;
  rightNow[MINUTE] = t.min;
  rightNow[0] = t.sec;
  
  #ifdef DEBUG_CLOCK
  for(int x = 0; x < sizeof(rightNow); x++){
    Serial.print(rightNow[x]);
    Serial.print(":");
  }
  Serial.println("");
  #endif
}


float getGreenhouseTemp(){
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);
    if((temp < -20.00)||(temp > 80)){
      temp = greenhouse._coolingTemp+10;
      return temp;
    }
    return temp;
}

byte onebit[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10000,
};
byte twobit[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10100,
};byte threebit[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10101,
};
byte bitCount1 = 0;
byte bitCount2 = 0;

void initLCD(byte length, byte width){
  lcd.begin(length, width);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.createChar(0, onebit);
  lcd.createChar(1, twobit);
  lcd.createChar(2, threebit);
  lcd.clear();
}

void lcdDisplay() {
  if(control == false){
    lcd.clear();
  }
  lcdPrintTemp();
  lcdPrintTempCible();
  lcdPrintRollups();
  lcdPrintOutputs();
  lcdPrintTime();
  //lcdPrintOutputsStatus();
}

void lcdPrintOutputs(){
    //Fan
    lcd.setCursor(0, 3); lcd.print(F("          "));
    lcd.setCursor(0, 3); lcd.print(F("FAN: "));
    if(digitalRead(FAN_PIN) == HIGH){
      lcd.print("ON");
    }
    else{
      lcd.print("OFF");
    }
    //Heater
    lcd.setCursor(9, 3); lcd.print(F("          "));
    lcd.setCursor(9, 3); lcd.print(F("|HEAT: "));
    if(digitalRead(HEATER_PIN) == HIGH){
      lcd.print("ON");
    }
    else{
      lcd.print("OFF");
    }
}
void lcdPrintRollups(){
    //East rollup
    lcd.setCursor(0, 2); lcd.print(F("          "));
    lcd.setCursor(0, 2); lcd.print(F("EAST:"));
    if(rollup1.incrementCounter() == OFF_VAL){  
      lcd.setCursor(0, 2);lcd.print(F(""));
    }
    else if (rollup1.opening() == true){
      lcd.setCursor(0, 2); lcd.print("OPENING  ");
    }
    else if (rollup1.closing() == true){
      lcd.setCursor(0, 2); lcd.print("CLOSING  ");
    }
    else{
      lcd.print(rollup1.incrementCounter());lcd.print("%");
    }
    if(((rollup1.closing() == false)&&(rollup1.closingCycle() == true))||((rollup1.opening() == false)&&(rollup1.openingCycle() == true))){
      if(bitCount1 == 0){
        lcd.write(byte(0));
        bitCount1 ++;
      }
      else if(bitCount1 == 1){
        lcd.write(byte(1));
        bitCount1 ++;
      }
      else if(bitCount1 == 2){
        lcd.write(byte(2));
        bitCount1 = 0;
      }
    }
    //West rollup
    lcd.setCursor(9, 2); lcd.print(F("          "));
      lcd.setCursor(9, 2); lcd.print(F("|WEST:"));
    if(rollup2.incrementCounter() == OFF_VAL){  
      lcd.print(F(""));
    }
    else if (rollup2.opening() == true){
      lcd.setCursor(9, 2); lcd.print("|OPENING   ");
    }
    else if (rollup2.closing() == true){
      lcd.setCursor(9, 2); lcd.print("|CLOSING   ");
    }
    else{
      lcd.print(rollup2.incrementCounter());lcd.print("%");
    }
    if(((rollup2.closing() == false)&&(rollup2.closingCycle() == true))||((rollup2.opening() == false)&&(rollup2.openingCycle() == true))){
      if(bitCount2 == 0){
        lcd.write(byte(0));
        bitCount2 ++;
      }
      else if(bitCount2 == 1){
        lcd.write(byte(1));
        bitCount2 ++;
      }
      else if(bitCount2 == 2){
        lcd.write(byte(2));
        bitCount2 = 0;
      }
    }
}
void lcdPrintTemp(){
    float temperature = greenhouseTemperature;
    lcd.setCursor(0,0); lcd.print(F("         "));
    lcd.setCursor(0,0); lcd.print(temperature); lcd.print(F("C"));
}

void lcdPrintTempCible(){
      lcd.setCursor(9,0);lcd.print("|TT:");lcd.print((short)(greenhouse._heatingTemp + rollup1.tempParameter())); lcd.print(F("-"));lcd.print((short)(greenhouse._coolingTemp + rollup1.tempParameter())); lcd.print(F("C"));
}
void lcdPrintTime(){
    lcd.setCursor(0,1); lcdPrintDigits(rightNow[HEURE]); lcd.print(F(":")); lcdPrintDigits(rightNow[MINUTE]);
    lcd.setCursor(9,1); lcd.print(F("|Timepoint ")), lcd.print(greenhouse._timepoint);
}

void lcdPrintDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
  lcd.print("0");
  lcd.print(digits);
}

void serialPrintDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
  Serial.print("0");
}

