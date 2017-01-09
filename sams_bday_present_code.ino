// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEO_PIXEL_RING_PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, NEO_PIXEL_RING_PIN, NEO_GRB + NEO_KHZ800);


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
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

RTC_DS3231 rtc;
const int latchPin = 4; //Goes to pin 12 on chip
const int clockPin = 3; // Goes to pin 11 on chip
const int dataPin = 5;
 
byte numitronA = 0;
byte numitronB = 0;

//TODO
  // Button to turn off display when book is closed
  // Button to set time
  // Add the two circular leds to show minutes/seconds
  // Make numitrons show hours
  // Maybe add a date disp, from one of the spare 7-segment arrays prewired to a max7221 I have laying around. (Probably not, will use too much power/space).
  // Also replace resistor voltage divider with the linear regulator.  (5-3.3 = 1.7*.3 < 2 W, so no need for a heatsink.)


void setup () {

  //Neo pixel stuff
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code


  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  //End neo pixel stuff
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);

  
  if (! rtc.begin()) {
    //Insert some error message on the leds here about no rtc!
    while (1);
  }

  /*
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  */
  // Turn both numitrons off.
  numitronA = 0;
  numitronB = 0;
  updateShiftRegisters();
  
}

void updateShiftRegisters()
{
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, MSBFIRST, numitronA);
   shiftOut(dataPin, clockPin, MSBFIRST, numitronB);
   digitalWrite(latchPin, HIGH);
}

/*
 * Returns  a byte that when shifted out to one of the numitrons, will display the given char.
 * Will make work with 0-9 for now, might add more later.
 */
byte getCharToDisplay(char c) {
  byte output = 0;
  switch(c) {
    case '0' :
      output = B11011111;
      break;
    case '1' :
      output = B00001010;
      break;
    case '2' :
      output = B11101101;
      break;
    case '3' :
      output = B01101111;
      break;
    case '4' :
      output = B00111011;
      break;
    case '5' :
      output = B01110111;
      break;
    case '6' :
      output = B11110111;
      break;
    case '7' :
      output = B00001110;
      break;
    case '8' :
      output = B11111111;
      break;
    case '9' :
      output = B01111111;
      break;
    default :
      //do nothing, 0 is fine, turn all segments off.
      break;
  }
  output >> 1; // Small mistake I made when writing out output, 
  // pin 0 on both shift regs is unused, 
  // display segments are 1 indexed, 
  // shift 1 over to correct.
  return output;
}

void loop () {
  //Neo pixel section
    // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
//colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
  // Send a theater pixel chase in...
  theaterChase(strip.Color(127, 127, 127), 50); // White
  theaterChase(strip.Color(127, 0, 0), 50); // Red
  theaterChase(strip.Color(0, 0, 127), 50); // Blue

  rainbow(20);
  rainbowCycle(20);
  theaterChaseRainbow(50); //These are really quite pretty!
  //End neopixel section
  
  // Check example sketch for more datetime object methods.
  DateTime now = rtc.now();
  //Date time has year,month, day, hour, minute, second methods.

  int seconds = now.second();
  numitronB = getCharToDisplay('0' + (seconds%10));
  numitronA = getCharToDisplay('0' + (seconds/10));
  updateShiftRegisters();
  delay(500);

  /*
  for ( int i = 0; i < 10; i++ ) {
    numitronA = getCharToDisplay('0'+i); 
    numitronB = getCharToDisplay('0'+i); 
    updateShiftRegisters();
    delay(500);
  }  //Example numitro ncode
  */ 

}

