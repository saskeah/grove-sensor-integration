
//LIBRARIES ETC

#include <SPI.h>
#include <SD.h>
#include <ADXL335.h>
#include <Wire.h>
#include <ADXL345.h>

//accelerometer(s)?
ADXL335 accelerometer;
const int chipSelect = 4;

ADXL345 adxl; //variable adxl is an instance of the ADXL345 library

//heart rate monitor
unsigned char counter;
unsigned long temp[21];
unsigned long sub;
bool data_effect=true;
unsigned int heart_rate;//the measurement result of heart rate
const int max_heartpluse_duty = 2000;
//you can change it follow your system's request.
//2000 meams 2 seconds. System return error
//if the duty overtrip 2 second.

//loudness
int val;

//SETUP CODE

void setup()
{
Serial.begin(9600);
  
//analog accel setup
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
    // from analog accelerometer code
    accelerometer.begin();
  }

//data shield setup
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
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

//heart rate setup
    arrayInit();
    attachInterrupt(0, interrupt, RISING);//set interrupt 0,digital port 2
    //end heart rate setup
}


//LOOP

void loop()
{

  //loudness
  analogRead(0);
  delay(10);
  val = analogRead(0);
  delay(500);

  //analog accelerometer
  int x,y,z;
  accelerometer.getXYZ(&x,&y,&z);
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
  //in File>Examples>Digital Accelerometer>Demo Code, 
  //there is example code for other types of functionality for the 
  //digital accelerometer if you want to add these later

  //data shield
  // make a string for assembling the data to log:
  String dataString = "";

  // data shield reading analog accelerometer
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

  //data sheild reading heart rate
  dataString += heart_rate;
  dataString += ",";

  //data shield reading loudness
  dataString += val;
  dataString += ",";

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
    }
    // if the file isn't open, pop up an error:
    else {
    Serial.println("error opening datalog.txt");
    }
    delay(1000);
}


//FUNCTIONS

//heart rate
/*Function: calculate the heart rate*/
void sum()
{
 if(data_effect)
 {
 heart_rate=1200000/(temp[20]-temp[0]);//60*20*1000/20_total_time
 //Serial.print("Heart_rate_is:\t");
 //Serial.println(heart_rate);
 }
 data_effect=1;//sign bit
}
/*Function: Interrupt service routine.Get the sigal from the external
interrupt*/
void interrupt()
{
 temp[counter]=millis();
 //Serial.println(counter,DEC);
 //Serial.println(temp[counter]);
 switch(counter)
 {
 case 0:
 sub=temp[counter]-temp[20];
 //Serial.println(sub);
 break;
 default:
 sub=temp[counter]-temp[counter-1];
 //Serial.println(sub);
 break;
 }
 if(sub>max_heartpluse_duty)//set 2 seconds as max heart pluse duty
 {
 data_effect=0;//sign bit
 counter=0;
 //Serial.println("Heart rate measure error,test will restart!" );
 arrayInit();
 }
 if (counter==20&&data_effect)
 {
 counter=0;
 sum();
 }
 else if(counter!=20&&data_effect)
 counter++;
 else
 {
 counter=0;
 data_effect=1;
 }

}
/*Function: Initialization for the array(temp)*/
void arrayInit()
{
 for(unsigned char i=0;i < 20;i ++)
 {
 temp[i]=0;
 }
 temp[20]=millis();
}
//end heart rate functions








