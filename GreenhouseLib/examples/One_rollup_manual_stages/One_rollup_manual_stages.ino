
//Libraries
#include "Arduino.h"
#include "GreenhouseLib_rollups.h"
#include <OneWire.h>
#include <DallasTemperature.h>

//Pinout
#define OPENING_PIN 6 //connect this pin to the opening relay
#define CLOSING_PIN 7 //connect this pin to the closing relay
#define ONE_WIRE_BUS A1 //connect this pin to the DS18B20 data line

//Create DS18B20 object
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float greenhouseTemperature;  //store temperature value
float targetTemperature = 20; //target temperature

//Declaring rollup object
Rollup rollup1;

//Declaring target temperature

void setup() {
  Serial.begin(9600);
  sensors.begin();  //start communication with temp probe
  rollup1.initRollup(4);
  rollup1.initOutputs(MAN_TEMP, ACT_HIGH, OPENING_PIN, CLOSING_PIN);

  rollup1.setParameters(0, 1, 100, 100, 100, 5, true);

    //Temperature mod : 0C (adjust to external target temperature)
    //hysteresis : 1C
    //Rotation time (Up): 100 sec (for full opening) (0 to 255 seconds)
    //Rotation time (Down): 100 sec (for full closing) (0 to 255 seconds)
    //Increments : 100
    //Pause between rotation : 5 (0 to 255 seconds)
    //Safety mode : ON (safety opening cycle every 30 min even if considered fully open)

    rollup1.setStageMod(1,0);
    rollup1.setStageTarget(1,10);
      //Stage number : 1 (max. 4)
      //Stage temperature mod : 0C (adjust to external target temperature)(-10 to +10)
      //Target increment : 10 (0 to max. increment)
    rollup1.setStageMod(2,1);
    rollup1.setStageTarget(2,30);
    rollup1.setStageMod(3,2);
    rollup1.setStageTarget(3,50);
    rollup1.setStageMod(4,3);
    rollup1.setStageTarget(4,100);

}

void loop() {

  sensors.requestTemperatures();  //ask for temperature
  greenhouseTemperature = sensors.getTempCByIndex(0); //store value
  rollup1.manualRoutine(targetTemperature, greenhouseTemperature);
}
