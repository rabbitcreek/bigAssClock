#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <Preferences.h>
Preferences preferences; 
const int  buttonPin1 = 0; 
const int  buttonPin2 = 35; 
int buttonPushCounter1 = 0;   // counter for the number of button presses
int buttonState1 = 1;         // current state of the button
int lastButtonState1 = 0;  
int buttonPushCounter2 = 0;   // counter for the number of button presses
int buttonState2 = 1;         // current state of the button
int lastButtonState2 = 0;
TFT_eSPI tft = TFT_eSPI();  // Invoke library

#include <ESP32Servo.h>
bool mainHiLow = 1; 
 int hourDifference = 0;
#include <SPI.h>
Servo myservo;
#include <Wire.h> 

#include <SPI.h> // Required for RTClib to compile properly
#include <RTClib.h> // From https://github.com/millerlp/RTClib
// Real Time Clock setup
RTC_DS3231 RTC; // Uncomment when using this chip
int dS = 0;
DateTime futureHigh;
DateTime futureLow;
DateTime future;
int correction = 0;
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
  myservo.attach(13,600,2400);
  //correction = 60;
  // Draw clock face
 //RTC.adjust(DateTime(F(__DATE__), F(__TIME__))); 
  //RTC.adjust(DateTime(2019,10,1,7,30,0)); 
  // Start up the oled display
   //ESP32PWM::allocateTimer(0);
  myservo.setPeriodHertz(50);
  // put your setup code here, to run once:
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.fillScreen(TFT_RED);
  preferences.begin("correction", false);
  unsigned int counter = preferences.getUInt("counter", 0);
  if(!digitalRead(buttonPin2)){
  while(buttonPushCounter2 < 10)wtRead();
  counter = buttonPushCounter1;
  preferences.putUInt("counter", counter);
  preferences.end();
  }
  correction = counter;
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextFont(4);
Serial.print("counter: ");
Serial.println(counter);
}


void loop(){
  delay(10000);
  
Serial.print("im here");
  dS = 0;
  now = RTC.now();
zip++;
  if((now.month()>3&&now.month()<11)||(now.month()==3&&now.day()>11)||(now.month()==11&&now.day()<6))dS=1;
   now = (now.unixtime() - dS*3600 + (zip* 86400));
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
  
  
  servoWrite=(hourFuture*10)+((nextTime.minute()*10)/60);
  servoWrite = servoWrite + correction;
  if(servoWrite > 120)servoWrite = servoWrite - 120;
  servoWrite = constrain(servoWrite, 0, 120);
  Serial.print(servoWrite);
  Serial.print("servoWriteOne");
  
  servoWrite=map(servoWrite,0,120,1170,1400);//each servo is different--these are the micorseconds limits for mine
  //servoWrite=constrain(servoWrite,1170,1400);
  Serial.print("servoWriteTwo:");
  Serial.print(servoWrite);
  myservo.writeMicroseconds(servoWrite);
  delay(1000);
  //servoMove();
  
}
void SerialScreen(DateTime now, DateTime futureHigh, DateTime futureLow,int dS){
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
void graphTide(DateTime now, DateTime futureHigh, DateTime futureLow,int dS){
  
  
  
  
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
void wtRead(){
  buttonState1 = digitalRead(buttonPin1);
  if (buttonState1 != lastButtonState1) {
    // if the state has changed, increment the counter
    if (buttonState1 == LOW) {
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter1++;
      //Serial.println("on");
      //Serial.print("number of button pushes: ");
      //Serial.println(buttonPushCounter1);
     
    } else {
      // if the current state is LOW then the button went from on to off:
     // Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  lastButtonState1 = buttonState1;
  buttonState2 = digitalRead(buttonPin2);
  if (buttonState2 != lastButtonState2) {
    // if the state has changed, increment the counter
    if (buttonState2 == LOW) {
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter2++;
      
      //buttonPushCounter1  = 0;
      
      if(buttonPushCounter2 == 3){
        //Serial.print("total wt = ");
        //Serial.print(wtTotal);
      }
    } else {
      // if the current state is LOW then the button went from on to off:
     // Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  lastButtonState2 = buttonState2;
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  if(buttonPushCounter2 == 0){
  
  tft.drawNumber(buttonPushCounter1, 40,40,7);
  }
}
   