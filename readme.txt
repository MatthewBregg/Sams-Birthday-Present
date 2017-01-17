Fairly simple code for the clock I made for sams graduation/birthday/christmas.

Set up to display the minutes on two numitron tubes, hour on a led ring, and seconds on a second led ring.
Each of the rings have 12 leds, so treated like a clock face.
Used a separate rtc with a battery to keep track of time.
Does not have any notification for dead battery, time will just start resetting on each unplug.

To set, hold the red button down.
After holding it down for a bit, the hour will be displayed on the numitrons.
Hit the button to increment form 0-23.
Hit the other switch, the one that turns off the lights when the book is closed to set the numitrons to display minutes.
Hit the set button to increment minutes.
Then hit the book close switch to start the clock again, seconds start from 0.
The rtc can track date, but I don't.

Libraries needed
- Adafruit RTC library
- Adafruit neopixel library
- My button debounce library.
- Wire library
