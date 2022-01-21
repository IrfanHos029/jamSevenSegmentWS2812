
#include <Adafruit_NeoPixel.h>
#include "RTClib.h"
#include <Wire.h>

#define PinLed D5
#define LEDS_PER_SEG 5
#define LEDS_PER_DOT 4
#define LEDS_PER_DIGIT  LEDS_PER_SEG *7
#define LED   LEDS_PER_DIGIT * 4 + 2 * LEDS_PER_DOT    //jumblah semua led strip


unsigned long tmrsave=0;
unsigned long tmrsaveHue=0;
int delayWarning(200);
int delayHue(2);
int Delay(500);
int brightnes = 200;
bool dotsOn = false;
int hl;
int hr;
int ml;
int mr;

static int hue;
int pixelColor;


Adafruit_NeoPixel strip(LED,PinLed,NEO_GRB + NEO_KHZ800);

RTC_DS3231 rtc;


long numbers[] = { 
//  7654321
  0b0111111,  // [0] 0
  0b0100001,  // [1] 1
  0b1110110,  // [2] 2
  0b1110011,  // [3] 3
  0b1101001,  // [4] 4
  0b1011011,  // [5] 5
  0b1011111,  // [6] 6
  0b0110001,  // [7] 7
  0b1111111,  // [8] 8
  0b1111011,  // [9] 9
  0b0000000,  // [10] off
  0b1111000,  // [11] degrees symbol
  0b0011110,  // [12] C(elsius)
};



void setup() {

     rtc.begin();
     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
     strip.begin();
     strip.setBrightness(brightnes);
    
     updateClock();
     strip.show();
}

void loop() {
  strip.setBrightness(brightnes);
  ShowClock(Wheel(hue+pixelColor) & 255);
  displayDots(strip.Color(255, 0, 0));

  strip.show();
   
}

void DisplayShow(byte number, byte segment, uint32_t color) {
  // segment from left to right: 3, 2, 1, 0
  byte startindex = 0;
  switch (segment) {
    case 0:
      startindex = 0;
      break;
    case 1:
      startindex = LEDS_PER_DIGIT;
      break;
    case 2:
      startindex = LEDS_PER_DIGIT * 2 + LEDS_PER_DOT * 2;
      break;
    case 3:
      startindex = LEDS_PER_DIGIT * 3 + LEDS_PER_DOT * 2;
      break;    
  }

   for (byte i=0; i<7; i++){                // 7 segments
    for (byte j=0; j<LEDS_PER_SEG; j++) {             // LEDs per segment
      strip.setPixelColor(i * LEDS_PER_SEG + j + startindex , (numbers[number] & 1 << i) == 1 << i ? color : strip.Color(0,0,0));
      //Led[i * LEDS_PER_SEG + j + startindex] = ((numbers[number] & 1 << i) == 1 << i) ? color : color(0,0,0);
      strip.show();
    }
  } 
  
  //yield();
}


void updateClock(){
  DateTime now = rtc.now();

   hl = now.hour() / 10;
   hr = now.hour() % 10;
   ml = now.minute() / 10;
   mr = now.minute() % 10;
}

void ShowClock(uint32_t color){
  
  DisplayShow(hl,3,color);
  DisplayShow(hr,2,color);
  DisplayShow(ml,1,color);
  DisplayShow(mr,0,color);
}

void displayDots(uint32_t color) {
  unsigned long tmr = millis();
  if(tmr - tmrsave > Delay){
    tmrsave = tmr;
  if (dotsOn) {
    for(int i = 70; i <= 77; i++){
      strip.setPixelColor(i , color);
    }

  } else {
    for(int i = 70; i <= 77; i++){
      strip.setPixelColor(i , strip.Color(0,0,0));
    }
  }
  dotsOn = !dotsOn;  
}
strip.show();
}

void timerHue(){
  unsigned long tmr = millis();
  if(tmr - tmrsaveHue > delayHue){
    tmrsaveHue = tmr;
  if(pixelColor <256){
    pixelColor++;
    if(pixelColor==255){
      pixelColor=0;
    }
  }
}

  for(int hue=0; hue<strip.numPixels(); hue++) {
    hue++;
      
    }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
