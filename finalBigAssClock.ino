#include <FastLED.h>
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip

#define NUM_LEDS 106
#define PIN 15 
CRGB leds[NUM_LEDS];

TFT_eSPI tft = TFT_eSPI();  // Invoke library for screen

#include <ESP32Servo.h>  //Servo library
bool mainHiLow = 1; 
 int hourDifference = 0;
#include <SPI.h>
Servo myservo;//starts servo
#include <Wire.h> 

#include <SPI.h> // Required for RTClib to compile properly
#include <RTClib.h> // From https://github.com/millerlp/RTClib
// Real Time Clock setup
RTC_DS3231 RTC; // sets up RTC
int dS = 0;//correction for daylight savings...you set the RTC for current time!!!
DateTime futureHigh;
DateTime futureLow;
DateTime future;

int slope;
int i = 0;
int zag = 0;
bool gate = 1;
int servoWrite = 0;
int correction = 103;
float tidalDifference=0;
 float pastResult;
 bool bing = 1;
 bool futureLowGate = 0;
 bool futureHighGate = 0;
// Tide calculation library setup.
// Change the library name here to predict for a different site.
#include "TidelibValdezPrinceWilliamSoundAlaska.h"//this will run out of data in 5 years!!
// Other sites available at http://github.com/millerlp/Tide_calculator
TideCalc myTideCalc; // Create TideCalc object 


long oldmillis; // keep track of time
float results; // tide height
DateTime now; // define variable to hold date and time
bool hiLow;
DateTime intoTime;

int zip = 0;


void setup() {
  // put your setuWire.begin(); 
  
  
  RTC.begin();
  Serial.begin(115200);
  delay(1000);
  FastLED.addLeds<WS2811, PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  myservo.attach(12,600,2400);  //this is  the total range for the servo
  //correction = 60;
  // Draw clock face
 //RTC.adjust(DateTime(F(__DATE__), F(__TIME__))); 
  //RTC.adjust(DateTime(2019,10,1,7,30,0)); 
  myservo.setPeriodHertz(50);//this is the speed that most servos run
  // put your setup code here, to run once:
  
  tft.init();//starts screen
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.fillScreen(TFT_RED);
  
  
  
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
 

}


void loop(){
  EVERY_N_MINUTES(10){
  

  dS = 0;
  now = RTC.now();

  if((now.month()>3&&now.month()<11)||(now.month()==3&&now.day()>11)||(now.month()==11&&now.day()<6))dS=1;
   now = (now.unixtime() - dS*3600 );
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
    SerialScreen(now, futureHigh, futureLow,dS);
    graphTide( now, futureHigh, futureLow, dS);
    lightTime();
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
}
}
  void armMove(DateTime now, DateTime futureHigh, DateTime futureLow,int dS){  // This takes high and low tide and figures amount to move servo
   
   
   int hourFuture = 0;
   DateTime nextTime;
   if( int(futureHigh.unixtime() - futureLow.unixtime()) < 0) hiLow = 1;  //Determines if the next is high or low tide hiLow=1 next high hiLow=0 next low
   if( int(futureHigh.unixtime() - futureLow.unixtime()) > 0) hiLow = 0;
  if(hiLow)nextTime = futureHigh;
  else nextTime = futureLow;   //sets next hour movement for arm
  hourFuture = nextTime.hour();
  hourFuture = hourFuture+1;  //arbitrary increase to make things right...you need this but cant figure out why
  if(nextTime.hour()>11)hourFuture=hourFuture-12;  //clock arithmatic to make all the hours fit on 12 hour clock
  if(hiLow==0)hourFuture=hourFuture+6;//this uses only one side to pointer to set high and low tides adds 6 hours on clock to set the low arm side
  if(hourFuture>11)hourFuture=hourFuture-12;//clock arithmetic again...
  
  
  servoWrite=(hourFuture*10)+((nextTime.minute()*10)/60);// multiplies hours by ten to make the minutes calculation easier
  servoWrite = servoWrite + correction;//this is where your correction goes to make the arm swing to the 12 oclock position or start from anywhere it is arbitrarly sits
  if(servoWrite > 120)servoWrite = servoWrite - 120;//more clock arithmatic
  servoWrite = constrain(servoWrite, 0, 120);//limits to clock
  Serial.print(servoWrite);
  Serial.print("servoWriteOne");
  
  servoWrite=map(servoWrite,0,120,1810,940);//each servo is different--these are the micorseconds limits for mine
  
  Serial.print("servoWriteTwo:");
  Serial.print(servoWrite);
  myservo.writeMicroseconds(servoWrite);//this is where the servo is moved
  delay(5000);
  //servoMove();
  
}
void SerialScreen(DateTime now, DateTime futureHigh, DateTime futureLow,int dS){  //this just prints out high and low tide and time
   bool hiLow;
   tft.fillScreen(TFT_BLACK);
   tft.setTextColor(TFT_YELLOW, TFT_BLACK);
   tft.setTextFont(4);
   tft.setCursor(0, 0);
   if( int(futureHigh.unixtime() - futureLow.unixtime()) < 0) hiLow = 1;
   if( int(futureHigh.unixtime() - futureLow.unixtime()) > 0) hiLow = 0;
 
  if (hiLow) {
    tft.print("HI         ");
    tft.println("LOW");
    tft.print(futureHigh.hour() +1, DEC); 
  tft.print(":");
  if (futureHigh.minute() < 10) {
    tft.print("0");
    tft.print(futureHigh.minute());
   }
  else if (futureHigh.minute() >= 10) {
    tft.print(futureHigh.minute());
  }
  tft.print("       ");
  tft.print(futureLow.hour() +1, DEC); 
  tft.print(":");
  if (futureLow.minute() < 10) {
    tft.print("0");
    tft.print(futureLow.minute());
   }
  else if (futureLow.minute() >= 10) {
   tft.print(futureLow.minute());
  }
    
  }
  else {
    tft.print("LOW         ");
    tft.println("HI");
    tft.print(futureLow.hour() +1, DEC); 
  tft.print(":");
  if (futureLow.minute() < 10) {
   tft.print("0");
   tft.print(futureLow.minute());
   }
  else if (futureLow.minute() >= 10) {
    tft.print(futureLow.minute());
  }
  tft.print("       ");
   tft.print(futureHigh.hour() +1, DEC); 
   tft.print(":");
  if (futureHigh.minute() < 10) {
    tft.print("0");
    tft.print(futureHigh.minute());
   }
  else if (futureHigh.minute() >= 10) {
    tft.print(futureHigh.minute());
  }
  }
  tft.println();
  tft.println();
  tft.println();
  tft.println();
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawCentreString("Whittier Dock Tide", 125, 55, 4);
}
void graphTide(DateTime now, DateTime futureHigh, DateTime futureLow,int dS){  //this function just tells how much time before next tide and current time
  
  
  
  
   if(futureHigh.unixtime() > futureLow.unixtime() ){
      hourDifference =  ((futureLow.unixtime() - now.unixtime())/3600);
     mainHiLow = 0;
   }
   else {
     hourDifference =  ((futureHigh.unixtime() - now.unixtime())/3600);
    mainHiLow = 1;
   }
   
   tft.setCursor(10, 80);
   tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.print(hourDifference);
   tft.print(" HOURS TO");
   if(mainHiLow)tft.println("   HIGH"); 
   else tft.println("    LOW");
   
   
    Serial.println();
    hourDifference = map(hourDifference, 0,6,16,0);
    hourDifference = constrain(hourDifference,0,16);
    
    //tft.println(myTideCalc.currentTide(now));

   tft.setCursor(5, 105);   
  tft.print(now.year(), DEC);
  tft.print("/");
  tft.print(now.month(), DEC); 
  tft.print("/");
  tft.print(now.day(), DEC); 
  tft.print("  ");
  tft.print(now.hour() + 1, DEC); 
  tft.print(":");
  if (now.minute() < 10) {
    tft.print("0");
    tft.println(now.minute());
   }
  else if (now.minute() >= 10) {
    tft.println(now.minute());
  }
 tft.print(" ");

       
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
  
   int dS = 0;
   now = RTC.now();
    if (now.month() == 7 && now.day() == 4){
      for (int j = 0; j < 5; j++){
        
  FadeInOut(0xff, 0x00, 0x00); // red
  FadeInOut(0xff, 0xff, 0xff); // white 
  FadeInOut(0x00, 0x00, 0xff); // blue
      }
    }
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
void FadeInOut(byte red, byte green, byte blue){
  float r, g, b;
      
  for(int k = 0; k < 256; k=k+1) { 
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    setAll(r,g,b);
    FastLED.show();
  }
     
  for(int k = 255; k >= 0; k=k-2) {
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    setAll(r,g,b);
    FastLED.show();
  }
}
   