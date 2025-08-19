#ifndef __FILE_MANAGER__
#define __FILE_MANAGER__
#include "Arduino.h"

void fsBegin();
int fileRead(String fname, unsigned char* buff, int size);
int fileWrite( String fname, unsigned char* buff, int len);
bool fileRemove( String fname);
String getRootPath();
int getDirList(String path, char *rxList, int size);
void textToSpeech(String Mp3FName, String txtMsg);
void playMP3(String filename);
void textToSpeechLoop();
#endif //__FILE_MANAGER__