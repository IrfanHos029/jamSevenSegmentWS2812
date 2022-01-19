//#define BLYNK_PRINT Serial

#include <Adafruit_NeoPixel.h>
#include <Blynk.h>
#include <NTPClient.h>
#include "RTClib.h"
#include <Wire.h>
#include <WiFiUdp.h>
#include <BlynkSimpleEsp8266.h>
//#include <EEPROM.h>

#define PinLed D5
#define LEDS_PER_SEG 5
#define LEDS_PER_DOT 4
#define LEDS_PER_DIGIT  LEDS_PER_SEG *7
#define LED   LEDS_PER_DIGIT * 4 + 2 * LEDS_PER_DOT    //jumblah semua led strip


unsigned long tmrsave=0;
unsigned long tmrsaveHue=0;
unsigned long tmrWarning=0;
int delayWarning(200);
int delayHue(2);
int Delay(500);
int r,g,b,Dots;
int rD,gD,bD;
int brightnes = 0;
bool dotsOn = false;
bool warningWIFI = false;
bool valueModeColor=false;
int hl;
int hr;
int ml;
int mr;
int tl;
int tr;
int Vtemp;
int HReset,MReset,SReset;
byte ModeRestart=0;
int ValueDisplayD =0;
char auth[] = "MDiv0_KOlQFRLMGkbZ5S7sWJyYlxdEog";
char ssid[] = "IrfanRetmi";
char pass[] = "00002222";
//int ValuePASS;
static int hue;
int pixelColor;
int lastConnectionAttempt = millis();
int connectionDelay = 5000; // try to reconnect every 5 seconds

Adafruit_NeoPixel strip(LED,PinLed,NEO_GRB + NEO_KHZ800);

const long utcOffsetInSeconds = 25200;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", utcOffsetInSeconds);

RTC_DS3231 rtc;
BlynkTimer timer;

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

    
    pinMode(D4,OUTPUT);
    digitalWrite(D4,HIGH);
      Serial.begin(9600);   
      Blynk.begin(auth,ssid,pass);
      delay(50);
       Serial.println("READY");
     timeClient.begin();
     rtc.begin();
     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
     strip.begin();
     strip.setBrightness(150);
     timer.setInterval(500, SendTemp);
     updateClock();
     strip.show();
}

void loop() {
Blynk.run();
StateWIFI();
updateClock();
timerHue();
//TimerReset();
  if(ValueDisplayD==0){
     ShowClock(Wheel((hue+pixelColor) & 255));
     displayDots(strip.Color(255, 0, 0));
  }
 
 if(ValueDisplayD==1){
  strip.setBrightness(brightnes);
  ShowClock((valueModeColor == 1)? Wheel((hue+pixelColor) & 255) : strip.Color( r, g, b));
  displayDots(strip.Color(rD, gD, bD));
}

 if(ValueDisplayD==2){
  strip.setBrightness(brightnes);
  ShowTemp((valueModeColor == 1)? Wheel((hue+pixelColor) & 255) : strip.Color( r, g, b));
  displayDots(strip.Color(0,0,0));
}
Restart();
    strip.show();
    timer.run();
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
   HReset = now.hour();
   MReset = now.minute();
   SReset = now.second();
   float Temperature = rtc.getTemperature();
   tl = int(Temperature) / 10;
   tr = int(Temperature) % 10; 
   Vtemp = int(Temperature);
}

void ShowClock(uint32_t color){
  
  DisplayShow(hl,3,color);
  DisplayShow(hr,2,color);
  DisplayShow(ml,1,color);
  DisplayShow(mr,0,color);
}

void ShowTemp(uint32_t color){
  
  DisplayShow(tl,3,color);
  DisplayShow(tr,2,color);
  DisplayShow(11,1,color);
  DisplayShow(12,0,color);
  displayDots(strip.Color(0,0,0));
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

void StateWIFI(){
  unsigned long tmr = millis();
  if(WiFi.status() != WL_CONNECTED) {
      if(tmr - tmrWarning > delayWarning){
        tmrWarning = tmr;
        if(warningWIFI){
      Serial.println("DISCONNECTED");
      digitalWrite(D4,HIGH);
        }
        else{ digitalWrite(D4,LOW);}
        warningWIFI = !warningWIFI;
        }
        valueModeColor == 1;
       gD=255;
       ValueDisplayD=0;
       strip.show();     
     }
     else{
      digitalWrite(D4,LOW);
      warningWIFI=false;   
      //Blynk.run();
      }
}     

void SendTemp(){
  updateClock();
  Blynk.virtualWrite(V5,Vtemp);
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
      //strip.setPixelColor(hue,Wheel((i+pixelColor) & 255));
    }
}

//Serial.println();

void setTimer(int Hour,int Minute, int Sec){
  rtc.adjust(DateTime(2022, 1, 21, Hour, Minute, Sec)); 
}

void TimerReset(){
  updateClock();
  if( HReset == 0 && MReset == 0 && SReset == 0){
    ESP.restart();
    delay(1500);
  }
}

void Restart(){
  if(ModeRestart == 1){
    ESP.restart();
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


BLYNK_WRITE(V0){
  r = param.asInt();
}

BLYNK_WRITE(V1){
  g = param.asInt();
  }

BLYNK_WRITE(V2){
  b = param.asInt();
}

BLYNK_WRITE(V3){
  valueModeColor = param.asInt();
}

BLYNK_WRITE(V4){
  brightnes = param.asInt();
}

BLYNK_WRITE(V5){
  rD = param[0].asInt();
  gD = param[1].asInt();
  bD = param[2].asInt();
}

BLYNK_WRITE(V6){
   int Value = param.asInt();
   timeClient.update();
  if(Value == 1){
       
       int HourNTP  = timeClient.getHours();      
       int MinuteNTP = timeClient.getMinutes();
       int SecondNTP = timeClient.getSeconds();
       setTimer(HourNTP,MinuteNTP,SecondNTP);
       
  }
  else{ 
          updateClock();
          Value=0;
  }

}

BLYNK_WRITE(V7){
   ValueDisplayD = param.asInt();
  
}

BLYNK_WRITE(V8){
  ModeRestart = param.asInt();
  //ESP.restart();
}
