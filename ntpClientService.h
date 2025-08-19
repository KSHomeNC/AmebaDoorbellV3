#ifndef __NTP_CLI_SER__
#define __NTP_CLI_SER__

#include "Arduino.h"
#include "WiFi.h"
#include "WiFiUdp.h"

#define NTP_PACKET_SIZE  48 // NTP time stamp is in the first 48 bytes of the message

class ntpServices
{
  protected:
    
    unsigned int localPort = 2390; 
    char timeServer[128] = "us.pool.ntp.org" ;    
    byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
    unsigned int sererPort = 123;
    long long etcEpoc;
    long long utcEpoc;

    void readNTPServer();
    void handleNTPResponse();
    int sockId = 0; 

  public:

    ntpServices();
    void begin(const char* server);
    void begin();    
    long long getEpoc(); 
};


#endif //__NTP_CLI_SER__
