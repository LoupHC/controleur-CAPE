

#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <DS3231.h>


//Create DS18B20 object
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//Create DHT object
#define DHTTYPE           DHT11       // Uncomment the type of sensor in use:
DHT dht(DHT_PIN, DHTTYPE);

//Create a RTC object
DS3231  rtc(SDA, SCL);                // Init the DS3231 using the hardware interface
Time  t;

unsigned long counter = 1;

void getDateAndTime(){
  t = rtc.getTime();
  rightNow[5].setValue(t.year-2000);
  rightNow[4].setValue(t.mon);
  rightNow[3].setValue(t.date);
  rightNow[HOUR].setValue(t.hour);
  rightNow[MINUT].setValue(t.min);
  rightNow[0].setValue(t.sec);


  for(int x = 0; x < 6; x++){
    rightNow[x].setLimits(0,60);
    rightNowValue[x] = rightNow[x].value(); 
  }
  
  #ifdef DEBUG_CLOCK
  for(int x = 0; x < 6; x++){
    Serial.print(rightNow[x].value());
    Serial.print(":");
  }
  Serial.println("");
  #endif
}


void getGreenhouseTemp(){
    greenhouseTemperature.setLimits(-180, 100);
    if(counter == 1){
      sensors.requestTemperatures();
      float temp = sensors.getTempCByIndex(0);
      //Serial.println(temp);
      //Serial.println(millis());
      
      if((temp <= -127.00)||(temp >= 85.00)){
        temp = greenhouse._coolingTemp+10;
        greenhouseTemperature.setValue(temp);
        sensorFailure = true;
      }
      else{
        greenhouseTemperature.setValue(temp);
        sensorFailure = false;
      }
      counter = 0;
    }
    counter++;
}

void getGreenhouseHum(){
    greenhouseHumidity.setLimits(0, 100);
    greenhouseHumidity.setValue((float)dht.readHumidity());
}

