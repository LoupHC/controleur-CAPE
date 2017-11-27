
//*****************LIBRAIRIES************************
#include "Arduino.h"
#include "GreenhouseLib_actuators.h"
#include "GreenhouseLib_timing.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>
#include <TimeLord.h>

//********************PINOUT**************************
#define WEST_OPENING_PIN 4 //connect this pin to the opening relay (west motor)
#define WEST_CLOSING_PIN 5 //connect this pin to the closing relay (west motor)
#define EAST_OPENING_PIN 6 //connect this pin to the opening relay (east motor)
#define EAST_CLOSING_PIN 7 //connect this pin to the closing relay (east motor)
#define FAN_PIN          2 //Connect this pin to the fan relay
#define HEATER_PIN       3 //connect this pin to the heater relay
#define ONE_WIRE_BUS     A1 //connect this pin to the DS18B20 data line

//********************OBJECTS**************************
//Create DS18B20 object
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//Create a RTC object
DS3231  rtc(SDA, SCL);                // Init the DS3231 using the hardware interface
Time  t;

// Create lcd object using NewLiquidCrystal lib
#define I2C_ADDR    0x27              // Define I2C Address where the PCF8574A is
#define BACKLIGHT_PIN     3
LiquidCrystal_I2C  lcd(I2C_ADDR, 2, 1, 0, 4, 5, 6, 7);

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

//Create Timelord object
const int TIMEZONE = -5; //PST
const float LATITUDE = 45.50, LONGITUDE = -73.56; // set your position here
TimeLord myLord; // TimeLord Object, Global variable

//Create timezones objects
const int nbTimezones = 5;
Timezone timezone[nbTimezones];
Timezone &timezone1 = timezone[0];
Timezone &timezone2 = timezone[1];
Timezone &timezone3 = timezone[2];
Timezone &timezone4 = timezone[3];
Timezone &timezone5 = timezone[4];

//Declaring rollup object
Rollup rollup1; //East side
Rollup rollup2; //West side
Fan fan1;
Heater heater1;

//********************VARIABLES**************************

//Time
byte program;                         //Actual program
byte lastProgram;                     //Last program
byte sunTime[6] = {};                 //actual time(sec, min, hour, day. month, year)
byte bitCount1 = 0;
byte bitCount2 = 0;

//Temperature
boolean failedSensor;
float greenhouseTemperature;
float coolingTemp;      //Upper normal temperature for the greenhouse
float heatingTemp;      //Lower normal temperature for the greenhouse

//Debounce/delays/avoid repetitions in printingPause time
boolean control = true;     //tells if in mode control or menu

unsigned long ramping = 900000;
unsigned long lastCount1;
unsigned long lastCount2;


//***************************************************
//********************SETUP**************************
//***************************************************

void setup() {
  //Serial.begin(9600);
  initLCD(20,4);
  sensors.begin();  //start communication with temp probe
  rtc.begin(); //start communication with RTC
  // The following lines can be uncommented to set the date and time
  //rtc.setTime(18, 59, 45);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(1, 1, 2017);   // Set the date to January 1st, 2014
  
  solarCalculations();
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

//********************Timezone*******************
  timezone1.setParameters(SR, -30, 18, 20);
  timezone2.setParameters(SR, 0, 18, 22);
  timezone3.setParameters(CLOCK, 12, 30, 20, 24);
  timezone4.setParameters(SS, -60, 16, 18);
  timezone5.setParameters(SS, 0, 17,  19);
  

//********************Rollup 1 (east)*******************
  rollup1.initOutputs(MAN_TEMP, ACT_HIGH, EAST_OPENING_PIN, EAST_CLOSING_PIN);

  rollup1.setParameters(0, 1, 189, 150, 100, 30, true);

  rollup1.initStage(1,0,10);
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
  fan1.setParameters(3, 1, true);
  

//************************Heater************************    
  heater1.initOutput(VAR_TEMP, HEATER_PIN);
  heater1.setParameters(-1, 2, true);

//********************Init variables***********************
  rightNowParameters();  //actual time, timezone and targetTemp;
}

//***************************************************
//*********************LOOP**************************
//***************************************************
void loop() {
  greenhouseTemperature = getGreenhouseTemp();
  //Update l'eure/la date
  getDateAndTime();
  //Définition de la température cible
  selectActualProgram();
  //Ajustement progressif
  startRamping();
  //Affichage
  lcdDisplay();
  //Rollups routine
  Serial.println("------EAST-------");
  rollup1.manualRoutine(coolingTemp, greenhouseTemperature);
  Serial.println("------WEST-------");
  rollup2.manualRoutine(coolingTemp, greenhouseTemperature);
  Serial.println("------FAN--------");
  fan1.routine(coolingTemp, greenhouseTemperature);
  Serial.println("------HEATER-----");
  heater1.routine(heatingTemp, greenhouseTemperature);
}

//***************************************************
//********************MACROS**************************
//***************************************************

void initLCD(byte length, byte width){
  lcd.begin(length, width);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.createChar(0, onebit);
  lcd.createChar(1, twobit);
  lcd.createChar(2, threebit);
  lcd.clear();
}

void solarCalculations(){
  initTimeLord();
  //Première lecture d'horloge pour définir le lever et coucher du soleil
  getDateAndTime();
  //Définition du lever et du coucher du soleil
  setSunrise();
  setSunset();
}

void initTimeLord(){
  myLord.TimeZone(TIMEZONE * 60);
  myLord.Position(LATITUDE, LONGITUDE);
  myLord.DstRules(3,2,11,1,60); // DST Rules for USA
}


void getDateAndTime(){
  t = rtc.getTime();
  sunTime[5] = t.year-2000;
  sunTime[4] = t.mon;
  sunTime[3] = t.date;
  sunTime[HEURE] = t.hour;
  sunTime[MINUTE] = t.min;
  sunTime[0] = t.sec;
  myLord.DST(sunTime);
  
  #ifdef DEBUG_CLOCK
  for(int x = 0; x < sizeof(sunTime); x++){
    Serial.print(sunTime[x]);
    Serial.print(":");
  }
  Serial.println("");
  #endif
}


void setSunrise(){
  //Définit l'heure du lever et coucher du soleil
  myLord.SunRise(sunTime); ///On détermine l'heure du lever du soleil
  myLord.DST(sunTime);//ajuster l'heure du lever en fonction du changement d'heure
  Timezone::sunRise[HEURE] = (short)sunTime[HEURE];
  Timezone::sunRise[MINUTE] = (short)sunTime[MINUTE];

  #ifdef DEBUG_SOLARCALC
    Serial.print("lever du soleil :");Serial.print(Timezone::sunRise[HEURE]);  Serial.print(":");  Serial.println(Timezone::sunRise[MINUTE]);
  #endif
}

void setSunset(){
  // Sunset: 
  myLord.SunSet(sunTime); // Computes Sun Set. Prints:
  myLord.DST(sunTime);
  Timezone::sunSet[HEURE] = (short)sunTime[HEURE];
  Timezone::sunSet[MINUTE] = (short)sunTime[MINUTE];
  #ifdef DEBUG_SOLARCALC
    Serial.print("coucher du soleil :");  Serial.print(Timezone::sunSet[HEURE]);  Serial.print(":");  Serial.println(Timezone::sunSet[MINUTE]);
  #endif
}
void rightNowParameters(){
  //Exécution du programme
  getDateAndTime();
  selectActualProgram();
  setTempCible();
}

void selectActualProgram(){
  //Sélectionne le programme en cour
    #ifdef DEBUG_PROGRAM
      Serial.println("----");
      Serial.print ("Heure actuelle ");Serial.print(" : ");Serial.print(sunTime[HEURE] );Serial.print(" : ");Serial.println(sunTime[MINUTE]);
    #endif
    for (byte y = 0; y < (nbTimezones-1); y++){
      
    #ifdef DEBUG_PROGRAM
      Serial.print ("Programme "); Serial.print(y+1);Serial.print(" : ");Serial.print(P[y][HEURE]);Serial.print(" : ");Serial.println(P[y][MINUTE]);
    #endif
      if (((sunTime[HEURE] == timezone[y].hr())  && (sunTime[MINUTE] >= timezone[y].mn()))||((sunTime[HEURE] > timezone[y].hr()) && (sunTime[HEURE] < timezone[y+1].hr()))||((sunTime[HEURE] == timezone[y+1].hr())  && (sunTime[MINUTE] <timezone[y+1].mn()))){
          program = y+1;
        }
    }
    
    #ifdef DEBUG_PROGRAM
      Serial.print ("Programme ");Serial.print(nbTimezones);Serial.print(" : ");Serial.print(P[nbTimezones-1][HEURE]);Serial.print(" : ");Serial.println(P[nbTimezones-1][MINUTE]);
    #endif
    
    if (((sunTime[HEURE] == timezone[nbTimezones-1].hr())  && (sunTime[MINUTE] >= timezone[nbTimezones-1].mn()))||(sunTime[HEURE] > timezone[nbTimezones-1].hr())||(sunTime[HEURE] < timezone[0].hr())||((sunTime[HEURE] == timezone[0].hr())  && (sunTime[MINUTE] < timezone[0].mn()))){
      program = nbTimezones;
    }
    #ifdef DEBUG_PROGRAM
      Serial.print ("Program is : ");
      Serial.println(program);
    #endif
}

void setTempCible(){
  coolingTemp = timezone[program-1].coolingTemp();
  heatingTemp = timezone[program-1].heatingTemp();
}

float getGreenhouseTemp(){
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);
    if((temp < -20.00)||(temp > 80)){
      temp = coolingTemp+10;
      failedSensor = true;
      return temp;
    }
    else{
      failedSensor = false;
      return temp;
    }
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
      lcd.setCursor(9,0);lcd.print("|TT:");lcd.print((short)(heatingTemp + rollup1.tempParameter())); lcd.print(F("-"));lcd.print((short)(coolingTemp + rollup1.tempParameter())); lcd.print(F("C"));
}
void lcdPrintTime(){
    lcd.setCursor(0,1); lcdPrintDigits(sunTime[HEURE]); lcd.print(F(":")); lcdPrintDigits(sunTime[MINUTE]);
    lcd.setCursor(9,1); lcd.print(F("|Timezone ")), lcd.print(program);
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

void startRamping(){
  //Définition des variables locales
  float newHeatingTemp;
  float newCoolingTemp;

  newHeatingTemp = timezone[program-1].heatingTemp();
  newCoolingTemp = timezone[program-1].coolingTemp();

  if (newCoolingTemp > coolingTemp){
    unsigned long rampingCounter = millis();
    //Serial.println(rampingCounter);
    //Serial.println(rampingCounter - lastCount);
    if(rampingCounter - lastCount1 > ramping) {
      lastCount1 = rampingCounter;
      coolingTemp += 0.5;
    }
  }
  else if (newCoolingTemp < coolingTemp){
    unsigned long rampingCounter = millis();
    if(rampingCounter - lastCount1 > ramping) {
      lastCount1 = rampingCounter;
      coolingTemp -= 0.5;
    }
  }
  if (newHeatingTemp > heatingTemp){
    unsigned long rampingCounter = millis();
    //Serial.println(rampingCounter);
    //Serial.println(rampingCounter - lastCount);
    if(rampingCounter - lastCount2 > ramping) {
      lastCount2 = rampingCounter;
      heatingTemp += 0.5;
    }
  }
  else if (newHeatingTemp < heatingTemp){
    unsigned long rampingCounter = millis();
    if(rampingCounter - lastCount1 > ramping) {
      lastCount2 = rampingCounter;
      heatingTemp -= 0.5;
    }
  }
}
