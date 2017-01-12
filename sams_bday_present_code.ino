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
const int numitron_dp_pin = 10;
// End numitron section

//TODO
  // Place into the book
  // Add the two circular leds to show hours/seconds
  // Make numitrons show minutes.
  // Maybe add a date disp, from one of the spare 7-segment arrays prewired to a max7221 I have laying around. (Probably not, will use too much power/space).
  // Also wire up shift registers master enable to a pin to properly turn off when book closed, might solve the occosional issue I've been hainv.g

// Button managers
const int close_button_pin = 12;
const int set_button_pin = 9;
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

  //Numitron pins
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  pinMode(numitron_dp_pin,OUTPUT);
  digitalWrite(numitron_dp_pin,LOW);

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

// begin/end is inclusive
void colorize_ring_hours(byte begin, byte end, bool am) {
  for ( byte i = begin; i != end+1; ++i ) {
                       // Pixel number, r, g, b
    if ( am ) {
      strip.setPixelColor(i, i,i,31-i);
    } else {
      strip.setPixelColor(i,31-i,i,i);
    }
  }
}

// begin/end is inclusive
void colorize_ring_seconds(byte begin, byte end, byte seconds, byte hours, int milliseconds) {
  float mult = .1
  + ((abs(milliseconds-500))/500.0)*5;
  for ( byte i = begin; i != end+1; ++i ) {
                       // Pixel number, r, g, b
     // float normalizer = day*day + hour*hour + minute*minute;
     // strip.setPixelColor(i, 30*((day*day)/normalizer),30*((hour*hour)/normalizer),30*((minute*minute)/normalizer));

     int color_transition = ((i-begin)*60)/(end-begin);
     if ( end == begin ) { color_transition = 60; } //If one one led is lit, then make it fully transitioned, so it appears to transitions aroind and around!!
     byte g = (60-color_transition/2)/3;
     byte b = (hours*10)/12;
     byte r = color_transition/6;
      if ( i != end ) {
       g*=mult;
       r*=mult;
       b*=mult;
     } else {
       int sum = r+g+b;
       g = sum/2;
       r = sum/2;
       b = sum/2;
     }
     strip.setPixelColor(i, r,g,b);
 
    
  }
}

void update_rings(byte curr_hour, byte curr_second, bool am, int milliseconds) {
  // Ring A should be leds 0-11, and ring B should then be 12-23. I will use ring A for hours, and ring B for seconds.

  // Handle hours
  // 
  colorize_ring_hours(0,curr_hour, am);
  // Handle second
  //hour translates directly, but for seconds, we want to enable up to 12, so divide by 5.
  colorize_ring_seconds(12,12+(curr_second/5),curr_second, curr_hour, milliseconds);


}

void blank_displays() {
   numitronA = 0;
   numitronB = 0;
  digitalWrite(numitron_dp_pin,LOW);
   colorWipe(strip.Color(0,0,0));
}

void render_displays() {
  updateShiftRegisters();
  strip.show(); 
}
int millis_offset = -1;

enum SetMode { off, hour, minute };
SetMode set_mode = off;
long set_button_false = 0;
int long_press_dur = 2000;
byte set_hour = 0;
byte set_minute = 0;

void handle_book_close_during_set_mode_btn_press() {
  DateTime now = rtc.now();
  if ( set_mode == hour ) {
        set_mode = minute;
  } else if ( set_mode == minute ) {
        set_mode = off;
        //Set rtc time here!
        // January 21, 2014 at 3am you would call:
        rtc.adjust(DateTime(now.year(),now.month(),now.day(), set_hour, set_minute, 0));
        set_button.set_button_pressed_callback(0);
        book_closed.set_button_pressed_callback(0);
  }
}

void handle_set_btn_during_set_mode_btn_press() {
     if ( set_mode == hour ) {
        set_hour += 1;
     }
     else if ( set_mode == minute ) {
        set_minute += 1;
      }

}


void loop () {
   checkButtons();
  //Clock setting logic
  // Setting mode
  if ( set_mode != off) {
    // Do clock setting stuff, then return
    byte set_helper = 05;
    set_hour = set_hour%24;
    if ( set_mode == hour ) {
      set_helper= set_hour;
    }
    set_minute = set_minute%60;
    if ( set_mode == minute ) {
      set_helper= set_minute;
    }
    blank_displays();
    numitronB = getCharToDisplay('0' + (set_helper%10));
    numitronA = getCharToDisplay('0' + (set_helper/10));
    render_displays();
    return;
  }

  // Logic to enter setting mode
  if ( set_button.query()) {
      if ( ( millis() - set_button_false ) > long_press_dur ) {
        set_mode = hour;
        set_hour = 0;
        set_minute = 0;
        bool prev_book_closed_btn = false;
        bool prev_set_btn = true;
        blank_displays();
        set_button.set_button_pressed_callback(&handle_set_btn_during_set_mode_btn_press);
        book_closed.set_button_pressed_callback(&handle_book_close_during_set_mode_btn_press);
      }
  } else {
    set_button_false = millis();
  }

  
  //Neo pixel section
    // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(0, 0, 0)); // Background color, for leds not overwritten, ie, if it's 6, then 7,8,9,10,11 will be this color. I like it off, but ask alyssa what she things though.
  //End neopixel section
  
  
  // Check example sketch for more datetime object methods.
  DateTime now = rtc.now();
  //Date time has year,month, day, hour, minute, second methods.

  // Set the numitron tubes to display the current minute
  int minutes = now.minute();
  numitronB = getCharToDisplay('0' + (minutes%10));
  numitronA = getCharToDisplay('0' + (minutes/10));

  // Set the led rings to display the hours, and seconds
  if ( millis_offset == -1 ) {
      int second = now.second();
      long millis_count = millis();
      while ( second == now.second() ) {
        delay(5);
        now = rtc.now();
      }
      millis_offset = millis() - millis_count;
  }
  // Millis offset should insure milliseconds stay roughly in sync with the clock.
  update_rings(now.hour()%12,now.second(),now.hour() <= 11,(millis()+millis_offset)%1000);
  // Set the numitron dp
  if ( now.hour() > 11 ) {
     digitalWrite(numitron_dp_pin,HIGH);
  } else {
     digitalWrite(numitron_dp_pin,LOW);
  }
  // Handle clock setting logic.
  

  // Handle turning off the leds when the book is closed. 
  if ( book_closed.query() ) {
    //If the book is closed, turn off both numitrons, and the led rings
    blank_displays();
  }


  // Refresh the numitrons and the led strip to show their content!
  render_displays();
  delay(10); 
}

