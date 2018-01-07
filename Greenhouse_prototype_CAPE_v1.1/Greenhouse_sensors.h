
//Select your sensors

//Temperature sensor
#define TEMP_DS18B20

//Humidity sensor
//#define HUM_DHT
//#define DHTTYPE   DHT11

//Real time clock
#define CLOCK_DS3231



#ifdef TEMP_DS18B20
  #include <OneWire.h>
  #include <DallasTemperature.h>
  //Create DS18B20 object
  OneWire oneWire(TEMP_SENSOR);
  DallasTemperature sensors(&oneWire);
#endif

#ifdef HUM_DHT
  #include <DHT.h>
  //Create DHT object
  DHT dht(HUMIDITY_SENSOR, DHTTYPE);
#endif

#ifdef CLOCK_DS3231
  #include <DS3231.h>
  //Create a RTC object
  DS3231  rtc(SDA, SCL);                // Init the DS3231 using the hardware interface
  Time  t;
#endif

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
    rightNow[HOUR].setValue(t.hour);
    rightNow[MINUT].setValue(t.min);
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
}

void getGreenhouseHum(){
  #ifdef HUMIDIDTY_DHT
    greenhouseHumidity.setLimits(0, 100);
    greenhouseHumidity.setValue((float)dht.readHumidity());
  #endif
}

