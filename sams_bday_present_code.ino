#include <Button_Debounce.h>

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
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, NEO_PIXEL_RING_PIN, NEO_GRB + NEO_KHZ800);


// Fill the dots one after the other with a color
void colorWipe(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
}





RTC_DS3231 rtc;

// Shift register pins
const int latchPin = 4; //Goes to pin 12 on chip
const int clockPin = 3; // Goes to pin 11 on chip
const int dataPin = 5;
// End shift register pins

// Numitron state
byte numitronA = 0;
byte numitronB = 0;
// End numitron section

//TODO
  // Place into the book
  // Add the two circular leds to show hours/seconds
  // Make numitrons show minutes.
  // Maybe add a date disp, from one of the spare 7-segment arrays prewired to a max7221 I have laying around. (Probably not, will use too much power/space).
  // Also wire up shift registers master enable to a pin to properly turn off when book closed, might solve the occosional issue I've been hainv.g

// Button managers
const int close_button_pin = 12;
const int set_button_pin = 13;
BasicDebounce book_closed = BasicDebounce(close_button_pin, 50);
BasicDebounce set_button = BasicDebounce(set_button_pin, 50);

void checkButtons() {
  book_closed.update();
  set_button.update();
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

  //Button pins
  pinMode(close_button_pin, INPUT_PULLUP);
  pinMode(set_button_pin, INPUT_PULLUP);

  
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

void loop () {
   checkButtons();
  //Neo pixel section
    // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(30, 0, 0)); // Red //30 seems like a reasonable brightness.
  //colorWipe(strip.Color(0, 255, 0)); // Green
  //colorWipe(strip.Color(0, 0, 255)); // Blue
  //colorWipe(strip.Color(0,0,0)); //Turn it off
  //End neopixel section
  
  
  // Check example sketch for more datetime object methods.
  DateTime now = rtc.now();
  //Date time has year,month, day, hour, minute, second methods.

  // Set the numitron tubes to display the current minute
  int minutes = now.minute();
  numitronB = getCharToDisplay('0' + (minutes%10));
  numitronA = getCharToDisplay('0' + (minutes/10));

  // Set the led rings to display the hours, and seconds

  // Handle clock setting logic.
  

  // Handle turning off the leds when the book is closed. 
  if ( book_closed.query() ) {
    numitronA = 0;
    numitronB = 0; 
    colorWipe(strip.Color(0,0,0));
    //If the book is closed, turn off both numitrons, and the led rings
  }


  // Refresh the numitrons and the led strip to show their content!
  updateShiftRegisters();
  strip.show(); 
  delay(500); //Wait half a second.

}

