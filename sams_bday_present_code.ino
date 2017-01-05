// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;
const int latchPin = 4; //Goes to pin 12 on chip
const int clockPin = 3; // Goes to pin 11 on chip
const int dataPin = 5;
 
byte numitronA = 0;
byte numitronB = 0;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup () {
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

