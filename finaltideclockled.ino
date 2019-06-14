

#include "FastLED.h"
#include <TimeLord.h>
#include <Wire.h> 
#define NUM_LEDS 105
CRGB leds[NUM_LEDS];
#define PIN 9 
int dateArray [4][2] = {{7,4}, {12,25}, {1,1}, {8,4}};
int ledArray [11][3] = {{245,68,0},{254,127,156},{238,165,130},{255,250,244},{255,255,255},{255,255,251},{255,255,255},{250,214,165},{38,83,141},{0,85,165},{25,25,112}};
float const LONGITUDE = -149.903;
float const LATITUDE = 61.21;
#include <SPI.h> // Required for RTClib to compile properly
#include <RTClib.h> // From https://github.com/millerlp/RTClib
// Real Time Clock setup
RTC_DS3231 RTC; // Uncomment when using this chip
DateTime now; // define variable to hold date and time
int currentDay = 0;
int hoursDay = 0;
int timeOn = 0;
int timeOff = 0;
int timeNotch = 0;
double timeOut = 0.0;
double timerOne = 0.0;
TimeLord tardis; 
void setup()
{
   
  Wire.begin();  
  RTC.begin();
  Serial.begin(57600);
  //RTC.adjust(DateTime(F(__DATE__), F(__TIME__))); 
   //RTC.adjust(DateTime(2019,5,27,9,20,0)); 
  tardis.TimeZone(-8 * 60); // tell TimeLord what timezone your RTC is synchronized to. You can ignore DST
  // as long as the RTC never changes back and forth between DST and non-DST
  tardis.Position(LATITUDE, LONGITUDE); // tel
  
  byte today[] = {  0, 0, 12, 27, 10, 2012    }; // store today's date (at noon) in an array for TimeLord to use

  FastLED.addLeds<WS2811, PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
}

// *** REPLACE FROM HERE ***
void loop() {
  //setAll(0,0,0); 
  //if ((millis() - timeOut) > 10000)
  now = RTC.now();
  timeOut = millis();
  for ( int i = 0; i < 4; i++){
    if (now.month() == dateArray[i][0] && now.day() == dateArray[i][1]){
      for (int j = 0; j < 5; j++){
  FadeInOut(0xff, 0x00, 0x00); // red
  FadeInOut(0xff, 0xff, 0xff); // white 
  FadeInOut(0x00, 0x00, 0xff); // blue
      }
    }
  }
  if (currentDay != now.day() ) {
    byte today[] = { 0,0,12, now.day(), now.month(), now.year()};
    if (tardis.SunSet(today)){
      timeOff = today[tl_hour];
      if (timeOff <= 4) timeOff = 24;
    }
    if (tardis.SunRise(today)){
       timeOn = today[tl_hour];
      hoursDay = timeOff - timeOn;
    }
    Serial.print("timeOff");
    Serial.println(timeOff);
    Serial.print("timeOn");
    Serial.println(timeOn);
    Serial.print("hoursDay");
    Serial.println(hoursDay);
    currentDay = now.day();
  }
  if (now.hour() <= timeOn ){
     timerOne = millis();
    while(millis() - timerOne < 10000){
      int q=random(20,60);
      int b = random( 64,150);
      int p = random(5,10);
      int r = random(100, 2000);
      meteorRain(0x1C,0x69,0x24,p, b, true, q);
      delay(r);
    }
    setAll(0,0,0);
    setAll(ledArray[10][0],ledArray[10][1],ledArray[10][2]);
    delay(60000);
    Serial.print("now.hour < time on");
    Serial.println(now.hour());
  }
   else if (now.hour() >= timeOff){
     timerOne = millis();
    while(millis() - timerOne < 10000){
      int q=random(20,60);
      int b = random( 64,150);
      int p = random(5,10);
      int r = random(100, 2000);
      meteorRain(0x1C,0x69,0x24,p, b, true, q);
      delay(r);
    }
    setAll(0,0,0);
    setAll(ledArray[10][0],ledArray[10][1],ledArray[10][2]);
    delay(60000);
    Serial.print("now.hour > timeOff");
    Serial.print(now.hour());
  }
  else if(now.hour() >= timeOn) {
    
    int hourClock = now.hour() - timeOn;
    timeNotch = constrain(hoursDay,4,20);
    timeNotch = map(hourClock,0,hoursDay,0,9);
    timeNotch = constrain(timeNotch,0,9);
    Serial.print("timeNotch");
    Serial.println(timeNotch);
    
    
    setAll(ledArray[timeNotch][0],ledArray[timeNotch][1],ledArray[timeNotch][2]);
    
    delay(60000);
    //setAll(0,0,0);
    }
  }
  
  // ---> here we call the effect function <---
 //setAll(ledArray[10][0],ledArray[10][1],ledArray[10][2]); 

void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  setAll(0,0,0);
  setAll(30,0,0);
  for(int i = NUM_LEDS; i > 0; i--) {
    
    
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
    
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        setPixel(i-j, red, green, blue);
      } 
    }
   
    showStrip();
    delay(SpeedDelay);
  }
}

void fadeToBlack(int ledNo, byte fadeValue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
    // NeoPixel
    uint32_t oldColor;
    uint8_t r, g, b;
    int value;
    
    oldColor = strip.getPixelColor(ledNo);
    r = (oldColor & 0x00ff0000UL) >> 16;
    g = (oldColor & 0x0000ff00UL) >> 8;
    b = (oldColor & 0x000000ffUL);

    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
    
    strip.setPixelColor(ledNo, r,g,b);
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   leds[ledNo].fadeToBlackBy( fadeValue );
 #endif  
}
 
void FadeInOut(byte red, byte green, byte blue){
  float r, g, b;
      
  for(int k = 0; k < 256; k=k+1) { 
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    setAll(r,g,b);
    showStrip();
  }
     
  for(int k = 255; k >= 0; k=k-2) {
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    setAll(r,g,b);
    showStrip();
  }
}
// ---> here we define the effect function <---
// *** REPLACE TO HERE ***

void showStrip() {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.show();
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   FastLED.show();
 #endif
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

void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue); 
  }
  showStrip();
}
