/*H***************************************************************************************************************
 * Project: esp32-commute-clock
 * Author: M Austin
 * Date: October, 2018
 * Description: Clock which shows current time and date, as well as current travel time (using Google Maps API)
 *              between two locations (labeled here as work and home).  
 *              
 * Notes: Need to verify and fill in info from line 20 to line 46 specific to application (WiFi crednetials,
 *        locations, etc).  Note that a configured Google API account with a key is required to get traffic 
 *        data.
 *        
 *Version       Date          Who      Detail
 *1.0           10/10/2018    MLA      Initial Commit
 *
 *H*/

#include <TimeLib.h> 
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUDP.h>
#include <SPI.h>
#include "MiniGrafx.h" // General graphic library
#include "ILI9341_SPI.h" // Hardware-specific library
#include "ArialRounded.h" //font file - need to include a font with larger text definitions to avoid pixellation

//Define TFT pins and parameters - below are for ESP32S HSPI
#define TFT_DC 16   //DC (data control) pin from TFT
#define TFT_CS 17   //CS (SS) pin from TFT
#define TFT_LED 13  //LED pin from TFT
int SCREEN_WIDTH = 240;
int SCREEN_HEIGHT = 320;
int BITS_PER_PIXEL = 4; // 2^4 = 16 colors

//wifi network information
const char* ssid = "SSID";
const char* password = "PASSWORD";

//Google maps req'd info - see https://developers.google.com/maps/documentation/distance-matrix/intro
#define API_KEY "PUT_API_KEY_HERE"   //need valid API tied to account
String homeAdd = "PUT_HOME_ADDRESS_HERE";  //home location ("destination" in google api) 
String workAdd = "PUT_WORK_ADDRESS_HERE";  //work location ("origin" in google api)
String leaveTime = "now";                 //leave time required to generate traffic.  "now" gives current traffic
String trafficMode = "best_guess";        //see google api docs for details
const char* server = "https://maps.googleapis.com";

const int timeZone = -5; //hours from UTC (standard time, DST adjust is done automatically!)
const int yellowLim = 4200; //traffic time (in seconds) to change display from green to yellow
const int redLim = 4500;    //traffic time (in seconds) to change display from yellow to red

//traffic api request config
int stopHour = 19; //hour to stop updating travel time to save google API requests (in 24 hour format), and turn off LED
int startHour = 7; //hour to start updating travel time at low frequency (15 min intervals), and turn on LED
int startFreqHour = 15; //hour to start updating travel time at high frequency (1 minute intervals)


//NTP time - initialize constants and buffer.  should not need to change these
const char timeserver[] = "time.nist.gov";
const int NTP_packetSize = 48;
byte packetBuffer[NTP_packetSize];
int timeOffset;
int timeHour;

// TFT color definitions for mini grafx
uint16_t palette[] = {ILI9341_BLACK, // 0
                      ILI9341_WHITE, // 1
                      ILI9341_NAVY, // 2
                      ILI9341_DARKCYAN, // 3
                      ILI9341_DARKGREEN, // 4
                      ILI9341_MAROON, // 5
                      ILI9341_PURPLE, // 6
                      ILI9341_OLIVE, // 7
                      ILI9341_LIGHTGREY, // 8
                      ILI9341_DARKGREY, // 9
                      ILI9341_BLUE, // 10
                      ILI9341_GREEN, // 11
                      ILI9341_CYAN, // 12
                      ILI9341_RED, // 13
                      ILI9341_MAGENTA, // 14
                      ILI9341_YELLOW}; // 15

//global variable declerations:
int durationInTraffic; 

int reqInterval = 30000; //baseline request interval - don't change here, change in main loop
long lastUpdate = 0;
int firstUpdate = 0;

//define strings as global variables
char travTime[8];
char time_string[12];
char date_string[16];

char dtDay[7][4] = {
{ "Sun" },
{ "Mon" },
{ "Tue" },
{ "Wed" },
{ "Thr" },
{ "Fri" },
{ "Sat" } };

#define travText "Time to Home:" //display tring
int dispSec;
int trafficColor = 11;
int dayOfWeek;

// Initialize the driver
ILI9341_SPI tft = ILI9341_SPI(TFT_CS, TFT_DC);
MiniGrafx gfx = MiniGrafx(&tft, BITS_PER_PIXEL, palette);

//initialize client objects - clientsecure to allow https request to google api, udp for NTP time sync
WiFiClientSecure client;
WiFiUDP udp;

//google's root certificate for https
const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n" \
"A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n" \
"Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n" \
"MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n" \
"A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n" \
"hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n" \
"v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n" \
"eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n" \
"tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n" \
"C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n" \
"zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n" \
"mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n" \
"V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n" \
"bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n" \
"3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n" \
"J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n" \
"291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n" \
"ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n" \
"AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n" \
"TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n" \
"-----END CERTIFICATE-----\n";

void setup() {
  Serial.begin(115200);
  delay(10);
  
  WiFi.begin(ssid, password); //open wifi connection

  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setCACert(root_ca); //set google SSL root cert for WiFiClientSecure

  setSyncProvider(getNTP); //time library function, set time source provider (needs to be unix time, sec since 1/1/1970)
  setSyncInterval(600); //time library function, set ntp sync interval in seconds

  pinMode(TFT_LED, OUTPUT); //configure tft LED pin as an output so we can turn it on and off

  //initialize graphics driver and clear screen
  gfx.init();
  gfx.setRotation(3);
  gfx.fillBuffer(0);
  gfx.commit();

}

void loop() {
  timeHour = dstHour(); //see NTP tab for definition, gets time-zone and DST adjusted hour

  if (timeHour < startHour && timeHour > stopHour) { //at night, stop API requests and turn off screen
    reqInterval = 0;
    digitalWrite(TFT_LED, LOW);
  }
  else if (timeHour >= startHour && timeHour < startFreqHour) { //during normal day, set request interval to 900 seconds (15 minutes) and leave screen on
    reqInterval = 900000;
    digitalWrite(TFT_LED, HIGH);
  }
  else if (timeHour >= startFreqHour && timeHour <= stopHour) { //during afternoon (close to commute time!), set request interval to 60 seconds and leave screen on
    reqInterval = 60000;
    digitalWrite(TFT_LED, HIGH);
  }

  //Serial.println(reqInterval);
  
  updateScreen(); //definition in graphics tab, updates time and traffic and prints to display
  delay(100);
}
