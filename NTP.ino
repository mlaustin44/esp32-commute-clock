time_t getNTP () {          //NTP time retriever function, sends request packet to NTP server then parses the return to get unix time (seconds since 1970)
  memset(packetBuffer, 0, NTP_packetSize);

  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  udp.beginPacket(timeserver, 123);
  udp.write(packetBuffer, NTP_packetSize);
  udp.endPacket();

  delay(1000);

  if (udp.parsePacket()) {
    udp.read(packetBuffer, NTP_packetSize);
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);  //a word is two bytes
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secs1900 = highWord << 16 | lowWord; //pack high and low words (2 bytes/16bits ea) into an unsigned long (32 bits)

    time_t timeEpoch = secs1900 - 2208988800UL; //ntp time is seconds since 1900, subtract 70 years to get unix time (secs since 1/1/1970)
    return timeEpoch;
  }
}

int dstHour () {  //returns daylight savings time adjusted hour from NTP
  int dstStartDays[7] = {11,10,8,14,13,12,10};  //array of dst start dates in March, from 2018-2024
  int dstEndDays[7] = {4,3,1,7,6,5,3};          //array of dst end dates in November, from 2018-2024

  int yearIndex = year() - 2018; //generate array position for dstStart/dstEnd for current year

  int curMonth = month();
  int curDay = day();

  //Serial.println(month());
  //Serial.println(day());
  //Serial.println(dstStartDays[yearIndex]);

  if ( curMonth > 3 && curMonth < 11) {
    timeOffset = timeZone;
  }
  else if (curMonth == 3) {
    if (curDay >= dstStartDays[yearIndex]) {
      timeOffset = timeZone;
    }
    else {
      timeOffset = timeZone - 1;
    }
  }
  else if (curMonth == 11) {
    if (curDay <= dstEndDays[yearIndex]) {
      timeOffset = timeZone;
    }
    else {
      timeOffset = timeZone - 1;
    }
  }
  else {
    timeOffset = timeZone - 1;
  }


  timeHour = hour() + timeOffset;

  if (timeHour < 0) {  //need to correct for cases where GMT offset creates <0 or >24 hours (i.e., 0200 GMT with a -5 offset returning -3)
    timeHour += 24;
  }
  else if (timeHour > 23) {
    timeHour -= 24;
  }
  
  return timeHour;
}

int getDay() {
  int tMonth;
  int tYear;
  if (month() <= 2) {
    tMonth = month() + 10;
    tYear = year() - 2001;
  } 
  else {
    tMonth = month() - 2;
    tYear = year() - 2000;
  }
  
  int w = int(day() + floor(2.6*tMonth - 0.2) - 40 + tYear + floor(tYear / 4) + 5) % 7;
  Serial.println(w);
  return w;
}

