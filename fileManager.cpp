#include "fileManager.h"
#include "Arduino.h"


#include"AmebaFatFS.h"
#include "GenAI.h"

AmebaFatFS fs;
GenAI tts;
String rootPath="";

String Mp3FName = "";
String txtMsg = "";

void fsBegin(){
  fs.begin();
  delay(1000); // give some time to initialized
  rootPath = String(fs.getRootPath());
 }
bool fileRemove( String fname){
  if (fs.status()){    
   return fs.remove(rootPath + fname);
  }
    
  
}
int fileRead(String fname, unsigned char* buff, int size){
  int len =0;
  if (fs.status()){    
    File file = fs.open(rootPath + fname);
    if( file != NULL){
      len = file.read((void*)buff, size); 
      file.close(); 
    }
  }
  return len;
}

int fileWrite( String fname, unsigned char* buff, int size){
  int len =0;

  if (fs.status()){   
    File file = fs.open(rootPath + fname);
    if( file != NULL){
      len = file.write(buff, size);  
      file.close();
    }
  }
  return len;
}

String getRootPath(){  
  return String(fs.getRootPath());
}

// return the read buffer size
int getDirList(String path, char *rxList, int size){
  char *p =0;
  char *q =0;
  String dirPath = String(fs.getRootPath())+path;

  /* list root directory and put results in buf */
  memset(rxList, 0, size);
  fs.readDir((char*)dirPath.c_str(), rxList, size);

  Serial.print("Files under");
  Serial.print( dirPath);
  Serial.println(":\r\n");  
  /* the filenames are separated with '\0', so we scan one by one */
  
  p = rxList;
  q=p;
  int strLen = strlen(p);
  while ( strLen> 0) {
    Serial.println(p);
    p += strlen(p) + 1;
    strLen = strlen(p);
  }
  Serial.print("buffer len of dir list =  ");
  Serial.println(p-q);    
  return p-q; // return the length of the read buffer
}
bool isFileAvailable(String filename)
{
  bool isAvailable = false;

  String filepath = rootPath + filename;
  File file = fs.open(filepath);
  if(file){
    if(file.size()>0)
    {
      isAvailable = true;
    }  
    file.close(); 
  }
  return isAvailable;
}
bool isAudioBusy = false;
void textToSpeech(String FName, String tMsg)
{
  if(!isAudioBusy){
    Mp3FName = FName;
    txtMsg = tMsg;
    isAudioBusy = true;
  }
}

void textToSpeechLoop()
{
  if (isAudioBusy){
    if(!isFileAvailable("voice/" + Mp3FName))
    {
      tts.googletts("voice/" +Mp3FName,txtMsg , "en-US");
      Serial.println("File Not available");
      delay(500); //wait for .5 sec 
    }
    else{
      Serial.println("File available");
    }
    
    playMP3(Mp3FName); 
    isAudioBusy=false;
  }
}


void playMP3(String filename)
{    
  String filepath = String(fs.getRootPath()) + "voice/" + filename;
  Serial.println(filepath);
  File file = fs.open(filepath, MP3);
  if(file){
    file.setMp3DigitalVol(0xA0);
    file.playMp3();
    file.close();    
  }

}
