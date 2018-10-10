void generateTime (char *tstring, char *dstring) {  //returns formatted, 12 hour time and date strings (global variables passed by reference).  using global variables easier than malloc.

  char amPm[3];

  int dtHour = 0;
  if (timeHour > 12) {      //figure out AM/PM and adjust hour to 12 hour time
    dtHour = timeHour - 12;
    sprintf(amPm, "PM");
  }
  else if (timeHour == 0){
    dtHour = 12;
    sprintf(amPm, "AM");
  }
  else {
    dtHour = timeHour;
    sprintf(amPm, "AM");
  }
  
  int dtMinute = minute();
  int dtSecond = second();

  

  sprintf(tstring, "%2d:%02d:%02d %s", dtHour, dtMinute, dtSecond, amPm); //make time string and print to global variable (passed by reference, not copy, so works on actual variable
  sprintf(dstring, "%s, %2d/%02d/%04d", dtDay[dayOfWeek], month(), day(), year());              //make date string

}

void generateTraffic(char *ttstring){
 
 int trafficHours;
 
 int trafficDuration = getTrafficDuration(workAdd, homeAdd, leaveTime, trafficMode); //get drive duration with traffic.  return is in seconds
    
 if (trafficDuration >= 3600) {              //if time is >1 hour, interger divide 3600seconds to return hours (since int division throws awaya remainder), otherwise hours - 0
   trafficHours = trafficDuration / 3600;
 }
 else {
   trafficHours = 0;
 }
 int trafficMinutes = ( trafficDuration % 3600 ) / 60;    //modulo durations by 3600 to get non-hour portion, then interger divide by 60 to get minutes (int div throws away remainder)
 int trafficSeconds = ( trafficDuration % 60 );           //modulo duration by 60 to get non-minute, non-hour portion.  since units of duration is seconds, any leftover is seconds

 //Serial.println(trafficHours);        //debug
 //Serial.println(trafficMinutes);
 //Serial.println(trafficSeconds);

  if (trafficDuration > redLim) { //arduino always passes global variables by reference if you dont define them inside a function
    trafficColor = 13; //13 = red
  }
  else if (trafficDuration > yellowLim) {
    trafficColor = 15; //15 = yellow
  }
  else {
    trafficColor = 11; //11 = green
  }
  

 sprintf(ttstring,"%d:%02d:%02d", trafficHours, trafficMinutes, trafficSeconds);  //print traffic duration to string in h:mm:ss format.  if you expect >10 hours in traffic, need to enlarge the array
 //sprintf(ttstring,"%d", trafficDuration); //debug

} 

void updateScreen() {  
  if (millis() - lastUpdate > reqInterval) {
    generateTraffic(travTime);  //the traffic string is a global variable so that it persists and can be modified by reference through a side effect of the generateTraffic function.
                                //could also be done with malloc to allocate the memory and then not free it, but this is simplier.
    lastUpdate = millis();
  }
  else if (firstUpdate == 0) {  //one-time update on reset so that screen isn't blank for reqInterval
    generateTraffic(travTime);
    lastUpdate = millis();
    dayOfWeek = getDay();
    firstUpdate = 1;            
  }

  if (hour() == 0) {
    dayOfWeek = getDay();
  }

  generateTime(time_string, date_string);

  if (dispSec != second()) {  //only update screen if seconds have changed
    gfx.fillBuffer(0);
    
    gfx.setTextAlignment(TEXT_ALIGN_CENTER);
    gfx.setFont(ArialRoundedMTBold_36);
    gfx.setColor(10);
    gfx.drawString(160, 20, time_string);
    
    gfx.setFont(ArialRoundedMTBold_36);
    gfx.drawString(160, 60, date_string);
    
    gfx.setTextAlignment(TEXT_ALIGN_CENTER);
    gfx.setFont(ArialRoundedMTBold_36);
    gfx.setColor(trafficColor);
    gfx.drawString(160, 120, travText);  
    gfx.drawString(160, 160, travTime);
  
    gfx.commit();

    dispSec = second();
  }
}
