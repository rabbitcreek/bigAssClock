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
int correction = 0;
 

#include <TFT_eSPI.h> 
TFT_eSPI tft = TFT_eSPI();
void setup() {
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
}

void loop() {
  // put your main code here, to run repeatedly:
tft.fillScreen(TFT_BLACK);
tft.drawNumber( correction, 40,40,7);
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
