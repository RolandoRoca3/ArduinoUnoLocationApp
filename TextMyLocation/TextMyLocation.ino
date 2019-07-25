/***
 * This is an experimental app to reply to sms messages with a URL that offers directions to the location of an Arduino Uno kit. Developed with 
 * the help of Adafruit FONA libraries and the module: Adafruit FONA 808 Shield - Mini Cellular GSM + GPS for Arduino
PRODUCT ID: 2636
Authors: Monte Windsor and Rolando Roca
 */

/***************************************************
  This is a library for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Adafruit_FONA.h>

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

// this is a large buffer for replies
char replybuffer[255];

//this is the buffer for the URL with location information
char locationurl[141];

// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines 
// and uncomment the HardwareSerial line
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// Use this for FONA 800 and 808s
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);


uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

uint8_t type;




void setup() {
  delay(10000);
    // put your setup code here, to run once:
  while (!Serial);


  Serial.begin(115200);
  Serial.println(F("Reply to txt with your location"));
  Serial.println(F("Initializing....(May take 3 seconds)"));

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case FONA800L:
      Serial.println(F("FONA 800L")); break;
    case FONA800H:
      Serial.println(F("FONA 800H")); break;
    case FONA808_V1:
      Serial.println(F("FONA 808 (v1)")); break;
    case FONA808_V2:
      Serial.println(F("FONA 808 (v2)")); break;
    case FONA3G_A:
      Serial.println(F("FONA 3G (American)")); break;
    case FONA3G_E:
      Serial.println(F("FONA 3G (European)")); break;
    default: 
      Serial.println(F("???")); break;
  }
  
  // Print module IMEI number.
  char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }
  delay(5000);
  // turn GPS on
  if (!fona.enableGPS(true))
    Serial.println(F("Failed to turn on"));
  delay(60000);  
}

void loop() {
  // put your main code here, to run repeatedly:

 // read all SMS (taken from FONA and edited)
        int8_t smsnum = fona.getNumSMS();
        uint16_t smslen;
        int8_t smsn;
        
        char sendto[21];

        if ( (type == FONA3G_A) || (type == FONA3G_E) ) {
          smsn = 0; // zero indexed
          smsnum--;
        } else {
          smsn = 1;  // 1 indexed
        }

        for ( ; smsn <= smsnum; smsn++) {
          Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);
          if (!fona.readSMS(smsn, replybuffer, 250, &smslen)) {  // pass in buffer and max len!
            Serial.println(F("Failed!"));
            break;
          }
          // if the length is zero, its a special case where the index number is higher
          // so increase the max we'll look at!
          if (smslen == 0) {
            Serial.println(F("[empty slot]"));
            smsnum++;
            continue;
          }

          Serial.print(F("***** SMS #")); Serial.print(smsn);
          Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
          Serial.println(replybuffer);
          // possible conditional for checking if the text is asking for location: 
          // (replybuffer.substring(1) == "where")||(replybuffer.substring(1) == " are")||(replybuffer.substring(1) == " r")
          if (true){

              // check for GPS location
              char gpsdata[120];
              fona.getGPS(0, gpsdata, 120);
              if (type == FONA808_V1)
                Serial.println(F("Reply in format: mode,longitude,latitude,altitude,utctime(yyyymmddHHMMSS),ttff,satellites,speed,course"));
              else 
                Serial.println(F("Reply in format: mode,fixstatus,utctime(yyyymmddHHMMSS),latitude,longitude,altitude,speed,course,fixmode,reserved1,HDOP,PDOP,VDOP,reserved2,view_satellites,used_satellites,reserved3,C/N0max,HPA,VPA"));
              Serial.println(gpsdata);
              
              // now need to parse Lat,Long to concatenate with locationurl
              //gpsdata

                String MessageInfo("");
                MessageInfo = String(MessageInfo + gpsdata);
                Serial.println("MessageInfo");
                Serial.println(MessageInfo);
              
                Serial.println("GPS State");
                Serial.println(getValue(MessageInfo,',',1));
                Serial.println("Latitude");
                Serial.println(getValue(MessageInfo,',',3));
                Serial.println("Longitude");
                Serial.println(getValue(MessageInfo,',',4));
              
                String UrlToSend = String("https://www.google.com/maps/search/?api=1&query=" + getValue(MessageInfo,',',3));
                UrlToSend = String(UrlToSend + ",");
                UrlToSend = String(UrlToSend+ getValue(MessageInfo,',',4));
              
                Serial.println("Formatted URL");
                Serial.println(UrlToSend);
                
                Serial.println("Desired URL Format");
                Serial.println("https://www.google.com/maps/search/?api=1&query=47.5951518,-122.3316393");
              
              // Length (with one extra character for the null terminator)
              int UrlLength = UrlToSend.length() + 1; 
               
              // Prepare the character array (the buffer) 
              char locationurl[UrlLength];
               
              // Copy it over 
              UrlToSend.toCharArray(locationurl, UrlLength);
              
                Serial.println("Formatted URL - Char");
                Serial.println(locationurl);
                            
              //debug print of message to be sent over sms 
              Serial.println(F("The following message will be sent over sms: ")); Serial.println(F("Test Response - in place of locationurl"));
        
              // Retrieve SMS sender address/phone number.
              if (! fona.getSMSSender(smsn, replybuffer, 250)) {
                Serial.println("Failed!");
                break;
              }
              memcpy(sendto,replybuffer,21);           
              Serial.print(F("FROM: ")); Serial.println(sendto); 
        
              // send an SMS with the location URL!
              flushSerial();
              Serial.print(F("Sending sms to #: "));Serial.print(sendto);


              
              // Original print for the message to be sent to 'sendto' number: Serial.println(message);
              if (!fona.sendSMS(sendto, locationurl)) {
                Serial.println(F("Failed"));
              } else {
                Serial.println(F("Sent!"));
             

              Serial.print(F("\n\rDeleting SMS #")); Serial.println(smsn);
              if (fona.deleteSMS(smsn)) {
                Serial.println(F("OK!"));
              } else {
                Serial.println(F("Couldn't delete"));
              }
            }
          Serial.println(F("*****"));
        }
        }
delay(10000);
}

// This code is from: https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void flushSerial() {
  while (Serial.available())
    Serial.read();
}
