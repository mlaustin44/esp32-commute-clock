# esp32-commute-clock
Commute travel time clock with Google Maps API data, using ESP32 and 2.8" TFT

Project has only been tested on an ESP32 - uncertain if other platforms will work due to new requirment for Google API for an SSL connection.

Required libraries:
-Espresif ESP32-Arduino
-Mini Grafx by Daniel Eichhorn (available in Arduino IDE Library Manager)
-Time by Michael Margolis      (available in Arduino IDE Library Manager)

Main file (esp32-commute-clock.ino) contains all the parameter declarations on lines 28-46.  A Google API key is required.  Note that google requires a key connected to a paid account, and each API request has a cost, but Google (as of October 2018) gives every account a $200 credit each month, which will cover many more requests than this clock could make.

The pins used for SPI are for SPI (on the Node-32S) are:
CS - Pin 17
DC - Pin 16
MOSI - Pin 23
SCK - Pin 18
RES - Pin 5
LED - Pin 13

SCK and MOSI are the hardware (VSPI) pins.  Changing the pin definition for CS and DC caused issues.

Google's root certificate to make the HTTPS connection is included as a multiline string.

NTP.ino contains the NTP request functionality and time manipulation functions.  The NTP request function (getNTP()) is not called directly, but is configured as the time source for the Time Library.  dstHour() is a quick daylight savings time and time zone adjustment function that has an array of daylight savings dates through 2024 pre-programmed in.  getDay() uses a formula to determine day of week from date based on https://cs.uwaterloo.ca/~alopez-o/math-faq/node73.html.

googleMapsAPI.ino is the Google Map API interface.  It builds a url-based JSON request String from the inputs, makes a connection to the google server, and returns a JSON object (see https://developers.google.com/maps/documentation/distance-matrix/intro).  The JSON object is then parsed using the Arduino JSON library to return the travel time in seconds.

graphics.ino contains 3 functions to generate time, date, and travel time strings and print them to the display.  generateTime() makes the correction from 24 hour to 12 hour time and then generates a char array for the time and date.  generateTraffic() calls the google api function to get the travel time in seconds, then generates a string displaying travel time and sets the text color.

Both generateTime() and generateTraffic() take pointers to char arrays as inputs.  Since the array is passed by reference, the function works on the original char array in memory instead of a copy, which is why both functions have void returns (since they do thier work through side effects instead of returning a value).  

updateScreen() uses the various intervals to determine which portions of the screen need to be changed, updates those by calling the respective functions, then builds and draws the new screen.
