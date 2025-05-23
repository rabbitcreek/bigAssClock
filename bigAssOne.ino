




#include "FastLED.h"
#include <ESP32Servo.h>
Servo myservo;
#include <Wire.h> 
#define NUM_LEDS 35
CRGB leds[NUM_LEDS];
#define PIN 15 
#include <SPI.h> // Required for RTClib to compile properly
#include <RTClib.h> // From https://github.com/millerlp/RTClib
// Real Time Clock setup
RTC_DS3231 RTC; // Uncomment when using this chip
int dS = 0;
DateTime futureHigh;
DateTime futureLow;
DateTime future;
int slope;
int i = 0;
int zag = 0;
bool gate = 1;
int servoWrite = 0;
float tidalDifference=0;
 float pastResult;
 bool bing = 1;
 bool futureLowGate = 0;
 bool futureHighGate = 0;
// Tide calculation library setup.
// Change the library name here to predict for a different site.
#include "TidelibValdezPrinceWilliamSoundAlaska.h"
// Other sites available at http://github.com/millerlp/Tide_calculator
TideCalc myTideCalc; // Create TideCalc object 

// 0X3C+SA0 - 0x3C or 0x3D for oled screen on I2C bus
//#define I2C_ADDRESS 0x3C

//SSD1306AsciiWire oled; // create oled dispaly object

long oldmillis; // keep track of time
float results; // tide height
DateTime now; // define variable to hold date and time
bool hiLow;
DateTime intoTime;
void servoMove(){
   //important if you want your servo to do a maximum 180 degrees--most have their querks
  delay(200);
  myservo.writeMicroseconds(servoWrite);
  delay(1000);
  //myservo.detach();
  
}

void armMove(DateTime now, DateTime futureHigh, DateTime futureLow,int dS){
   
   
   int hourFuture = 0;
   DateTime nextTime;
   if( int(futureHigh.unixtime() - futureLow.unixtime()) < 0) hiLow = 1;
   if( int(futureHigh.unixtime() - futureLow.unixtime()) > 0) hiLow = 0;
  if(hiLow)nextTime = futureHigh;
  else nextTime = futureLow;
  hourFuture = nextTime.hour();
  hourFuture = hourFuture+1;
  if(nextTime.hour()>11)hourFuture=hourFuture-12;
  if(hiLow==0)hourFuture=hourFuture+6;//this uses only one side to pointer to set high and low tides
  if(hourFuture>11)hourFuture=hourFuture-12;
  for(hourFuture = 0; hourFuture < 13; hourFuture++){
  
  servoWrite=(hourFuture*10)+((nextTime.minute()*10)/60);
  Serial.print(servoWrite);
  Serial.print("servoWriteOne");
  
  servoWrite=map(servoWrite,0,120,1175,1395);//each servo is different--these are the micorseconds limits for mine
  servoWrite=constrain(servoWrite,1175,1395);
  Serial.print("servoWriteTwo:");
  Serial.print(servoWrite);
  myservo.writeMicroseconds(servoWrite);
  delay(1000);
  //servoMove();
  }
}
void setup(){
  Wire.begin();  
  RTC.begin();
  Serial.begin(115200);
  delay(1000);
 //RTC.adjust(DateTime(F(__DATE__), F(__TIME__))); 
  //RTC.adjust(DateTime(2019,10,1,7,30,0)); 
  // Start up the oled display
   ESP32PWM::allocateTimer(0);
  myservo.setPeriodHertz(50);
   FastLED.addLeds<WS2811, PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
   myservo.attach(13,600,2400);
}
void loop(){
  delay(8000);
Serial.print("im here");
  dS = 0;
  now = RTC.now();
  if((now.month()>3&&now.month()<11)||(now.month()==3&&now.day()>11)||(now.month()==11&&now.day()<6))dS=1;
   now = (now.unixtime() - dS*3600);
   bing = 1;
   i = 0;
    pastResult=myTideCalc.currentTide(now);
    Serial.print("current tide:");
    Serial.println(pastResult);
  while(bing){ //This loop asks when the next high or low tide is by checking 5 min intervals from current time
    i++;
   
    DateTime future(now.unixtime() + (i*5*60L));
    results=myTideCalc.currentTide(future);
    tidalDifference=results-pastResult;
    if (gate){
      if(tidalDifference<0)slope=0;//if slope is positive--rising tide--slope neg falling tide
      else slope=1;
      gate=0;
   }
   if(tidalDifference>0&&slope==0){
      futureLow = future;
      gate=1;
      //bing = 0;
      futureLowGate = 1;
   }
    else if(tidalDifference<0&&slope==1){
    futureHigh = future;
    gate=1;
    //bing = 0;
    futureHighGate = 1;
  
   }
   if( futureHighGate && futureLowGate) {
  
    armMove(now, futureHigh, futureLow, dS);
    
    delay(4000);
    gate = 1;
    bing = 0;
    futureHighGate = 0;
    futureLowGate = 0;
   }
   pastResult=results;
  }
   if( int(futureHigh.unixtime() - futureLow.unixtime()) < 0) hiLow = 1;
   if( int(futureHigh.unixtime() - futureLow.unixtime()) > 0) hiLow = 0;
  if(hiLow)intoTime = futureHigh;
  else intoTime = futureLow;
  
  if(hiLow){
     Serial.println("I found HIGH tide");
  }
  else Serial.println("I found LOW tide");
 
  Serial.print(intoTime.year(), DEC);
  Serial.print("/");
  Serial.print(intoTime.month(), DEC); 
  Serial.print("/");
  Serial.print(intoTime.day(), DEC); 
  Serial.print("  ");
   Serial.print((intoTime.hour() +1), DEC); 
  Serial.print(":");
  if (intoTime.minute() < 10) {
    Serial.print("0");
    Serial.print(intoTime.minute());
   }
  else if (intoTime.minute() >= 10) {
    Serial.print(intoTime.minute());
  }
  Serial.print(":");
  if (intoTime.second() < 10) {
    Serial.print("0");
    Serial.println(intoTime.second());
  }
  else if (intoTime.second() >= 10) {
    Serial.println(intoTime.second());
  }
   
  while(intoTime.unixtime() >= now.unixtime() ){
    now = RTC.now();    
    //outPut(now, intoTime, dS);
    Serial.print("servoWriteThree: ");
    Serial.println(servoWrite);
    servoMove();
    now = (now.unixtime() - dS * 3600);
    delay(60000);
    lightTime();
  }
}
void lightTime(){
  setAll(0,0,0);
  setAll(250,214,165);
  timerLight();
}
void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue); 
  }
  FastLED.show();
}
void timerLight() {
  /*
   int dS = 0;
   now = RTC.now();
   if((now.month()<3||now.month()>11)||(now.month()==3&&now.day()<11)||(now.month()==11&&now.day()>6))dS=1;
   now = (now.unixtime() - dS*3600);
   int hourTime = now.hour();
   Serial.print("now.hour");
   Serial.print(hourTime);
  if(now.hour()>11)hourTime = hourTime - 12;
   hourTime = (hourTime * 9) +((now.minute() * 9) / 60);
   hourTime = hourTime + 35;
   Serial.print("hourTime");
   Serial.println(hourTime);
   if (hourTime >= 104) hourTime = hourTime - 104;
   if (hourTime == 0) hourTime = 1;
   */
   now = RTC.now();
   if((now.month()<3||now.month()>11)||(now.month()==3&&now.day()<11)||(now.month()==11&&now.day()>6))dS=1;
   now = (now.unixtime() - dS*3600);
   int hourTime = now.hour();
   Serial.print("now.hour");
   Serial.print(hourTime);
  if(now.hour()>11)hourTime = hourTime - 12;
   hourTime = (hourTime * 3)+ ((now.minute() * 3) / 60);
   
   Serial.print("hourTime");
   Serial.println(hourTime);
   
   leds[hourTime - 1] = CRGB::Red; 
   leds[hourTime] = CRGB::Red; 
   leds[hourTime + 1] = CRGB::Red; 
   FastLED.show();
}
void setPixel(int Pixel, byte red, byte green, byte blue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H 
   // FastLED
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
 #endif
}