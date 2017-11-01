//LIBRARIES
#include <SPI.h>
#include <SD.h> //SD card
#include <ADXL335.h> //analog accel
#include <Wire.h> //I2C
#include <ADXL345.h> //digital accel
#include <DHT.h> //temp and humidity sensor
#include <DHT_U.h> //temp and humidity sensor
#include <Adafruit_Sensor.h> //temp and humidity sensor
#include <Adafruit_BMP280.h> //barometer
#include <Adafruit_TSL2561_U.h> //light
#include <RTClib.h> //RTC

//DEFINITIONS AND INSTANTIATIONS

//RTC def
RTC_DS1307 RTC;

//barometer def
#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10

//light def
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 12345);

//baro def
Adafruit_BMP280 bme;

//temp and hum def
#define DHTPIN 5     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

//analog accelerometer def
ADXL335 accelerometer;
const int chipSelect = 4;

//digital accelerometer def
ADXL345 adxl;

//loudness def
int loudness;

//loudness indicator light def
int LLED = 2;

//SD indicator light def
int SDLED = 3;

//SETUP CODE

void setup()
{
Serial.begin(9600);

//loudness light setup
pinMode(LLED, OUTPUT);

//SD light setup
pinMode(SDLED, OUTPUT);

//RTC setup
Wire.begin();
RTC.begin();

//light setup
tsl.enableAutoRange(true);
tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
tsl.begin();

//baro setup
  bme.begin();

//temp hum setup
  dht.begin();
  
//analog accel setup
  accelerometer.begin();

//data shield setup
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    digitalWrite(SDLED, LOW);
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  digitalWrite(SDLED, HIGH);
  //end data shield setup code

//digital accel setup
  adxl.powerOn();
  
   //set activity/ inactivity thresholds (0-255)
  adxl.setActivityThreshold(75); //62.5mg per increment
  adxl.setInactivityThreshold(75); //62.5mg per increment
  adxl.setTimeInactivity(10); // how many seconds of no activity is inactive?
 
  //look of activity movement on this axes - 1 == on; 0 == off 
  adxl.setActivityX(1);
  adxl.setActivityY(1);
  adxl.setActivityZ(1);
 
  //look of inactivity movement on this axes - 1 == on; 0 == off
  adxl.setInactivityX(1);
  adxl.setInactivityY(1);
  adxl.setInactivityZ(1);
 
  //look of tap movement on this axes - 1 == on; 0 == off
  adxl.setTapDetectionOnX(0);
  adxl.setTapDetectionOnY(0);
  adxl.setTapDetectionOnZ(1);
 
  //set values for what is a tap, and what is a double tap (0-255)
  adxl.setTapThreshold(50); //62.5mg per increment
  adxl.setTapDuration(15); //625us per increment
  adxl.setDoubleTapLatency(80); //1.25ms per increment
  adxl.setDoubleTapWindow(200); //1.25ms per increment
 
  //set values for what is considered freefall (0-255)
  adxl.setFreeFallThreshold(7); //(5 - 9) recommended - 62.5mg per increment
  adxl.setFreeFallDuration(45); //(20 - 70) recommended - 5ms per increment
 
  //setting all interrupts to take place on int pin 1
  //I had issues with int pin 2, was unable to reset it
  adxl.setInterruptMapping( ADXL345_INT_SINGLE_TAP_BIT,   ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_DOUBLE_TAP_BIT,   ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_FREE_FALL_BIT,    ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_ACTIVITY_BIT,     ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_INACTIVITY_BIT,   ADXL345_INT1_PIN );
 
  //register interrupt actions - 1 == on; 0 == off  
  adxl.setInterrupt( ADXL345_INT_SINGLE_TAP_BIT, 1);
  adxl.setInterrupt( ADXL345_INT_DOUBLE_TAP_BIT, 1);
  adxl.setInterrupt( ADXL345_INT_FREE_FALL_BIT,  1);
  adxl.setInterrupt( ADXL345_INT_ACTIVITY_BIT,   1);
  adxl.setInterrupt( ADXL345_INT_INACTIVITY_BIT, 1);
  //end digital accelerometer setup code

}

//LOOP

void loop()
{

  //RTC
  DateTime now = RTC.now(); 
  
  //light
  sensors_event_t event;
  tsl.getEvent(&event);
  
  //temp and hum
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  //loudness
  loudness = analogRead(3);

    //loudness indicator light
    if(loudness >= 10) {
     digitalWrite(LLED, HIGH);
  } else {
     digitalWrite(LLED, LOW);
  }

  //analog accelerometer
  //int x,y,z;
  //accelerometer.getXYZ(&x,&y,&z);
  float ax,ay,az;
  accelerometer.getAcceleration(&ax,&ay,&az);
  //end analog accelerometer code

  //digital accelerometer
  //original variables were x, y, z - replaced with dx, dy, dz  
  int dx,dy,dz;  
  adxl.readXYZ(&dx, &dy, &dz); //read the accelerometer values and store them in variables  x,y,z
  
  double dxdydz[3];
  double dax,d_ay,daz;
  adxl.getAcceleration(dxdydz);
  dax = dxdydz[0];
  d_ay = dxdydz[1];
  daz = dxdydz[2];
  //end digital accelerometer code
  /*in File>Examples>Digital Accelerometer>Demo Code, 
  there is example code for other types of functionality for the 
  digital accelerometer if you want to add these later*/

  //data shield
  //make a string for assembling the data to log:
  String dataString = "";

  //data sheild reading RTC
  dataString += String(now.year());
  dataString += "/";
  dataString += String(now.month());
  dataString += "/";
  dataString += String(now.day());
  dataString += " ";
  dataString += String(now.hour());
  dataString += ":";
  dataString += String(now.minute());
  dataString += ":";
  dataString += String(now.second());
  dataString += ",";

  //data sheild reading light
  dataString += String(event.light);
  dataString += ",";

  //data shield reading temp and hum
  dataString += String(h);
  dataString += ",";
  dataString += String(t);
  dataString += ",";

  //data shield reading baro
  dataString += String(bme.readPressure());
  dataString += ",";

  //data shield reading analog accelerometer
  dataString += String(ax);
  dataString += ",";
  dataString += String(ay);
  dataString += ",";
  dataString += String(az);
  dataString += ",";
  
  //data sheild reading digital accelerometer
  dataString += String(dax);
  dataString += ",";
  dataString += String(d_ay);
  dataString += ",";
  dataString += String(daz);
  dataString += ",";

  //data shield reading loudness
  dataString += loudness;

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    //keeping this on purpose, so I can see the data without pulling out the SD card!
    Serial.println(dataString);
    }
    // if the file isn't open, pop up an error:
    else {
    Serial.println("error opening datalog.txt");
    }

    //1 second delay
    delay(1000);
}
