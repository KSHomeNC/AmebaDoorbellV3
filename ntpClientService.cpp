#include "ntpClientService.h"
WiFiUDP ntpUDP;

ntpServices::ntpServices(){         
              
}
    
void ntpServices::begin(const char* server){
      sprintf(timeServer,"%s", server);
      sockId = ntpUDP.begin(localPort);
}

void ntpServices::begin(){         
      Serial.println("Socket begin");
      sockId = ntpUDP.begin(localPort);        
}
  

void ntpServices::handleNTPResponse()
{
  
  if (ntpUDP.parsePacket()) {
    Serial.println(" ntp response");
    // We've received a packet, read the data from it
    ntpUDP.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    utcEpoc = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(utcEpoc);


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((utcEpoc  % 86400L) / 3600); // print the hour (86400 equals secs per day)    
    Serial.print(':');
    if (((utcEpoc % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((utcEpoc  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((utcEpoc % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(utcEpoc % 60); // print the second

    // print the hour, minute and second:

    etcEpoc = utcEpoc - (5*60*60); // 5hr in seconds
    Serial.print("The EST time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((etcEpoc  % 86400L) / 3600); // print the hour (86400 equals secs per day)    
    Serial.print(':');
    if (((etcEpoc % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((etcEpoc  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((etcEpoc % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(etcEpoc % 60); // print the second   
  }
  else
  {
    Serial.println(" failed to receive response");
  }

  
}

void ntpServices::readNTPServer()
{
  // set all bytes in the buffer to 0
  Serial.print("Socket id = ");
  Serial.println(sockId);
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp: 
  if(sockId){
    if(ntpUDP.beginPacket(timeServer, sererPort)){ // NTP requests are to port 123    
      if(ntpUDP.write(packetBuffer, NTP_PACKET_SIZE))  {
        ntpUDP.endPacket();
        Serial.println(" ntp request send");
      }
    }  
  }//sockId
  
  
 // give a second to reach packet to the destination
  delay(1000);
}


long long ntpServices::getEpoc()
{
  readNTPServer();
  delay(1000);
  handleNTPResponse();
  return etcEpoc;
}
