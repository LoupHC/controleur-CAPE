
//Select your sensors

//Temperature sensor
#define TEMP_DS18B20
//#define TEMP_DHT22
//#define TEMP_DHT11
//#define TEMP_DHT12

//Humidity sensor
//#define HUM_DHT22
//#define HUM_DHT11
#define HUM_DHT12

//Real time clock
#define CLOCK_DS3231

//Temerature pin
#define TEMP_SENSOR          7 //connect this pin to the DS18B20 data line

#ifdef TEMP_DS18B20
  #include <OneWire.h>
  #include <DallasTemperature.h>
  //Create DS18B20 object
  OneWire oneWire(TEMP_SENSOR);
  DallasTemperature sensors(&oneWire);
#endif

#ifdef HUM_DHT11
  #include <DHT.h>
  //Create DHT object
  DHT dht(HUMIDITY_SENSOR, DHT11);
#endif

#ifdef HUM_DHT22
  #include <DHT.h>
  //Create DHT object
  DHT dht(HUMIDITY_SENSOR, DHT22);
#endif

#ifdef CLOCK_DS3231
  #include <DS3231.h>
  //Create a RTC object
  DS3231  rtc(SDA, SCL);                // Init the DS3231 using the hardware interface
  Time  t;
#endif

#include <DHT12.h>
  //Create DHT object
DHT12 DHT;


unsigned long counter = 1;

void getDateAndTime(){

  for(int x = 0; x < 6; x++){
    rightNow[x].setLimits(0,60);
  }

  #ifdef CLOCK_DS3231
    t = rtc.getTime();
    rightNow[5].setValue(t.year-2000);
    rightNow[4].setValue(t.mon);
    rightNow[3].setValue(t.date);
    rightNow[2].setValue(t.hour);
    rightNow[1].setValue(t.min);
    rightNow[0].setValue(t.sec);
  #endif


  for(int x = 0; x < 6; x++){
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

  #ifdef TEMP_DS18B20
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);

    if((temp <= -127.00)||(temp >= 85.00)){
      temp = greenhouse._coolingTemp+10;
       greenhouseTemperature.setValue(temp);
      sensorFailure = true;
    }
    else{
      greenhouseTemperature.setValue(temp);
      sensorFailure = false;
    }
  #endif

  #ifdef TEMP_DHT12
    DHT.read();
    greenhouseTemperature.setValue(DHT.temperature);
  #endif

}

void getGreenhouseHum(){
  greenhouseHumidity.setLimits(0, 100);

  #ifdef HUMIDIDTY_DHT
    greenhouseHumidity.setValue((float)dht.readHumidity());
  #endif
  #ifdef HUM_DHT12
    DHT.read();
    greenhouseHumidity.setValue(DHT.humidity);
  #endif
}
