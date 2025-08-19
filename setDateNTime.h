#ifndef __DATENTIME_H__
#define __DATENTIME_H__

#include <stdio.h>
#include <time.h>
#include "rtc.h"
#include "ntpClientService.h"

/* Change these values to set the current initial date */
#define YEAR 2020
#define MONTH 1
#define DAY 1
/* Change these values to set the current initial time */
#define HOUR 13
#define MIN 14
#define SEC 15

class kshomeDnT
{
  private:
    char datenTimeStr[128];
    ntpServices* ntpClient;
    long long rtcEpoc;


    void setRTCEpoc(long long epocTime)
    {
      rtc.Write(epocTime);
    }  
    long long getRTCEpocTime()
    {
      return rtc.Read();
    }
    
  public:
    kshomeDnT()
    {
      ntpClient = new ntpServices();
      long long rtcEpoc =rtc.SetEpoch(YEAR, MONTH, DAY, HOUR, MIN, SEC);
      rtc.Init();
      rtc.Write(rtcEpoc);
    }
    void begin()
    {
      ntpClient->begin();
    }
    void synchRTCtoNtpServer()
    {
      rtcEpoc = ntpClient->getEpoc();
      setRTCEpoc(rtcEpoc);
    }
  /*
    tm* getDateNTime()
    {
      struct tm *timeinfo = NULL;
      long long seconds = getEpocTime();
      timeinfo = localtime(&seconds);
      timeinfo->tm_year = timeinfo->tm_year + 1900;
      timeinfo->tm_mon = timeinfo->tm_mon + 1;
      return timeinfo;
    }
    */
    char* getDateNTimeStr()
    {
      struct tm * timeinfo = NULL;
      long long seconds = getRTCEpocTime();
      timeinfo = localtime(&seconds);
      timeinfo->tm_year = timeinfo->tm_year + 1900;
      timeinfo->tm_mon = timeinfo->tm_mon + 1; 
      
      sprintf(datenTimeStr, "%02d:%02d:%04d  %02d:%02d:%02d", timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_year,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
      return datenTimeStr;
      
    }
    char* getFileName()
    {
      struct tm * timeinfo = NULL;
      long long seconds = getRTCEpocTime();
      timeinfo = localtime(&seconds);
      timeinfo->tm_year = timeinfo->tm_year + 1900;
      timeinfo->tm_mon = timeinfo->tm_mon + 1; 
      
      sprintf(datenTimeStr, "%02d%02d%04d_%02d%02d%02d", timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_year,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
      return datenTimeStr;
      
    }
    void printEpochTime(void) {
      Serial.print("Epoch Time(in s) since January, 1, 1970:");
      Serial.print(getRTCEpocTime());
      Serial.print("s");
    }
    void printStringTime(struct tm* tInfo) {    
      Serial.print("Time as a custom formatted string: ");
      Serial.print(tInfo->tm_year);
      Serial.print("-");
      Serial.print(tInfo->tm_mon);
      Serial.print("-");
      Serial.print(tInfo->tm_mday);
      Serial.print(" ");
      Serial.print(tInfo->tm_hour);
      Serial.print(":");
      Serial.print(tInfo->tm_min);
      Serial.print(":");
      Serial.println(tInfo->tm_sec);
    }
    void printBasicString(void) {
      Serial.println();
      Serial.print("Time as a basic string:           ");
      //Serial.print(ctime(getRTCEpocTime()));
    }
  
};




#endif //__DATENTIME_H__
