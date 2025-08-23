/*
 Example guide:
 https://ameba-doc-arduino-sdk.readthedocs-hosted.com/en/latest/ameba_pro2/amb82-mini/Example_Guides/Multimedia/RTSP%20Streaming.html
*/

#include "WiFi.h"
#include "StreamIO.h"
#include "VideoStream.h"
#include "AudioStream.h"
#include "AudioEncoder.h"
#include "RTSP.h"
#include "WDT.h"

#define AON_WDT_Enable (0)
#if AON_WDT_Enable == 0
#define RUN_CALLBACK_IF_WATCHDOG_BARKS (0)
#endif

#include "ledManager.h"
#include "buttonManager.h"
#include "mqttManager.h"
#include "setDateNTime.h"
#include "fileManager.h"
LEDManager ledManager;
ButtonManager buttonManager(BUTTON_PIN);
MQTTManager mqttManager;

String FW_VERSION = "3.1.001";
WDT wdt(AON_WDT_Enable);
#define CHANNEL 0
#define HOST_NAME "Ameba_doorbellTest"
// Default preset configurations for each video channel:
// Channel 0 : 1920 x 1080 30FPS H264
// Channel 1 : 1280 x 720  30FPS H264
// Channel 2 : 1280 x 720  30FPS MJPEG

// Default audio preset configurations:
// 0 :  8kHz Mono Analog Mic
// 1 : 16kHz Mono Analog Mic
// 2 :  8kHz Mono Digital PDM Mic
// 3 : 16kHz Mono Digital PDM Mic
#define CHANNELJPEG 1    // Channel for taking snapshots
VideoSetting configV(CHANNEL);
VideoSetting configJPEG(VIDEO_FHD, CAM_FPS, VIDEO_JPEG, 1);
AudioSetting configA(0);
Audio audio;
AAC aac;
RTSP rtsp;
StreamIO audioStreamer(1, 1);    // 1 Input Audio -> 1 Output AAC
StreamIO avMixStreamer(2, 1);    // 2 Input Video + Audio -> 1 Output RTSP
kshomeDnT datenTime;
String rstpLink ="";
char ssid[] = "subhSpec";    // your network SSID (name)
char pass[] = "pawan@158";        // your network password
int status = WL_IDLE_STATUS;
void getRstpLink();


#define isTimeOut(st,to)  (((millis()-st)>to)?true:false)



void wdtStart( unsigned long time_ms){
     // setup 5s watchdog
    wdt.init(time_ms);

#if AON_WDT_Enable
    // when WDT always on IRQ is not supported
#else
#if RUN_CALLBACK_IF_WATCHDOG_BARKS
    wdt.init_irq((wdt_irq_handler)my_watchdog_irq_handler, 0);
#else
    // system would restart in default when watchdog barks
#endif
#endif

    // enable watchdog timer
    wdt.start();
}

void system_recover(){
    while(1); // let watchdog take the action on timer expiration.
}

void cameraReStart() {
    int cnt = 0;
    // 1) Configure video, then init video module
    Camera.configVideoChannel(CHANNEL, configV);
    Camera.configVideoChannel(CHANNELJPEG, configJPEG);
    Camera.videoInit();            // or Camera.videoInit(CHANNEL);

    // 2) Audio path
    audio.configAudio(configA);
    audio.begin();
    aac.configAudio(configA);
    aac.begin();

    // 3) RTSP + StreamIO (get consumers ready before frames start)
    rtsp.configVideo(configV);
    rtsp.configAudio(configA, CODEC_AAC);

    audioStreamer.registerInput(audio);
    audioStreamer.registerOutput(aac);
    
    do{
        if(audioStreamer.begin() != 0){
            delay(100);
            cnt++;
            Serial.println("audioStreamer begin failed");
        }
        else {
            break;
        }
    }while(  cnt< 5);
    //if (audioStreamer.begin() != 0) Serial.println("audioStreamer begin failed");
    cnt =0;

    avMixStreamer.registerInput1(Camera.getStream(CHANNEL));
    avMixStreamer.registerInput2(aac);
    avMixStreamer.registerOutput(rtsp);
    
   /* do{
        if(avMixStreamer.begin() != 0){
            delay(100);
            cnt++;
            Serial.println("avMixStreamer begin failed");
            avMixStreamer.end();
        }
        else {
            break;
        }
    }while(  cnt< 5);  

    cnt=0;
*/
    avMixStreamer.resume();

    // Start sink first, then producer
    rtsp.begin();
    Camera.channelBegin(CHANNELJPEG);
    Camera.channelBegin(CHANNEL);

    delay(300);
    printInfo();
}

void cameraSetup() {
    int cnt = 0;
    // 1) Configure video, then init video module
    Camera.configVideoChannel(CHANNEL, configV);
    Camera.configVideoChannel(CHANNELJPEG, configJPEG);
    Camera.videoInit();            // or Camera.videoInit(CHANNEL);

    // 2) Audio path
    audio.configAudio(configA);
    audio.begin();
    aac.configAudio(configA);
    aac.begin();

    // 3) RTSP + StreamIO (get consumers ready before frames start)
    rtsp.configVideo(configV);
    rtsp.configAudio(configA, CODEC_AAC);

    audioStreamer.registerInput(audio);
    audioStreamer.registerOutput(aac);
    do{
        if(audioStreamer.begin() != 0){
            delay(100);
            cnt++;
            Serial.println("audioStreamer begin failed");
        }
        else {
            break;
        }
    }while(  cnt< 5);
    //if (audioStreamer.begin() != 0) Serial.println("audioStreamer begin failed");
    cnt =0;

    avMixStreamer.registerInput1(Camera.getStream(CHANNEL));
    avMixStreamer.registerInput2(aac);
    avMixStreamer.registerOutput(rtsp);
    do{
        if(avMixStreamer.begin() != 0){
            delay(100);
            cnt++;
            Serial.println("avMixStreamer begin failed");
            avMixStreamer.end();
        }
        else {
            break;
        }
    }while(  cnt< 5);  

    cnt=0;

    // Start sink first, then producer
    rtsp.begin();
    
    Camera.channelBegin(CHANNEL);
    Camera.channelBegin(CHANNELJPEG);

    delay(300);
    printInfo();
}

void cameraRelease() {
    // Stop sinks/consumers first
  rtsp.end();
  

  // Then streamers/mixers
  audioStreamer.end();  
  avMixStreamer.end();
  
  
  // Then the producer (camera)
  Camera.channelEnd(CHANNEL);

  // Tear down audio
  aac.end();
  
  audio.end();

  // Fully deinit video for this channel (or use the all-channels version)
  Camera.videoDeinit(CHANNEL);   // <- use this
  // Camera.videoDeinit();       // (all channels) alternative

  // Optional: wait until the driver reports the channel is down
  unsigned long t0 = millis();
  while (Camera.videostream_status(CHANNEL) != 0 && (millis() - t0) < 500) {
    delay(10);
  }

  delay(300);
}

void wifiInit()
{
    WiFi.setHostname(HOST_NAME);
    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network:
        status = WiFi.begin(ssid, pass);

        // wait 2 seconds for connection:
        delay(2000);
    }    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
void ntpClientBegin(){
   /* Synch RTC with NTP server*/
   //datenTime.begin();   --- problem in opening more than 1 udp port mqtt is breaking
   //datenTime.synchRTCtoNtpServer();   
   Serial.println(datenTime.getDateNTimeStr());
}
LEDinfo ledInfo;
void setup()
{
    wdtStart(20000);
    Serial.begin(115200);

    ledManager.begin();
    ledManager.setNextPatternNone(BLUE);
    ledManager.setNextPatternNone(GREEN);
    ledManager.setNextPatternNone(RED);

    Serial.println(FW_VERSION);
  
    ledInfo.defaultState=true;
    ledInfo.onDuration =1; // any values keeps ON since offDuration is 0
    ledInfo.offDuration =0;
    ledManager.setNextPatternSingle(BLUE, &ledInfo);
       
    wifiInit();    
   
    buttonManager.begin();
    //file system  
    fsBegin();
    mqttManager.begin();
    ntpClientBegin();
    Serial.println(" device configured and ready to use");
    wdt.stop();
    wdtStart(5000); // start watch dog for application 

    ledInfo.defaultState=false;
    ledInfo.onDuration =0; // any values keeps OFF since offDuration is 1
    ledInfo.offDuration =1;
    ledManager.setNextPatternSingle(BLUE, &ledInfo); 

    ledInfo.defaultState=true;
    ledInfo.onDuration =1000; // blinking
    ledInfo.offDuration =5000;
    ledManager.setNextPatternSingle(GREEN, &ledInfo); 
}
bool isVideoStarted = false;
unsigned long startTimeV =0;
unsigned long endTimeV=60000; // default 1 minutes

unsigned long buttonPressTime = 0; // Tracks the time when the button was pressed
bool isButtonPressed = false;      // Tracks if the button press is being handled
const int ledPatternOffTime = 10000; // 10 seconds
bool isWiFiAvailable = true;
unsigned long wifiDisconnectTime = 0;
unsigned long wifiDisconnectTimeOut_ms = 10000;// 10 sec

void checkWiFiStatus()
{
  switch (WiFi.status())
  {
    case WL_CONNECT_FAILED:
    case WL_CONNECTION_LOST:
    case WL_DISCONNECTED:
        
        isWiFiAvailable = false;
        wifiDisconnectTime = millis();
        break;
    default:
        isWiFiAvailable = true;
        break;
  }
  if (isTimeOut(wifiDisconnectTime, wifiDisconnectTimeOut_ms) && !isWiFiAvailable){
      system_recover(); //10mSec
  }
  
}
uint32_t img_addr = 0;
uint32_t img_len = 0;
void captureImageAndStore()
{
    String fName = "capture/" + String(datenTime.getFileName()) + ".jpg";    // Capture snapshot of stranger under name Stranger <No.>
    Camera.getImage(CHANNELJPEG, &img_addr, &img_len);
    fileWrite(fName,(uint8_t *)img_addr, img_len);   
    Serial.print(" picture saved of size: ") ;
    Serial.println(img_len) ;
}
bool skipWdtRefresh =false;
unsigned long doorbellLiveDuration = 60000; //default 1 minutes
void loop() {
  if (buttonManager.isButtonPressed()) {
    if (!isButtonPressed) { // Only handle the button press once
      isButtonPressed = true;
      buttonPressTime = millis(); // Record the time of the button press

      // Set the LED pattern
   
      ledManager.setNextPatternMultiple(doorbellLiveDuration); // for 30 sec
      cameraReStart();
      isVideoStarted = true;  
      startTimeV = millis();
      char tmpBuf[64];
      rstpLink.toCharArray(tmpBuf, (rstpLink.length()+1));
      // Publish MQTT message
      mqttManager.publishMessage( PUB_TOPIC_BELL,tmpBuf);
      textToSpeech("doorBell.mp3", " Welcome at 3580, please wait for some time");
      captureImageAndStore();
      Serial.println("Button pressed. Setting LED pattern for 10 seconds.");
      wdtStart(doorbellLiveDuration); // reboot after 1 minute
      skipWdtRefresh = true;
    }
  }
  // Check if the LEDs need to be turned off after the specified time
  if (isButtonPressed && (millis() - buttonPressTime >= ledPatternOffTime)) {   
    isButtonPressed = false;        // Reset the button press state
    Serial.println("LEDs turned off after 10 seconds.");
  }
/*
  if(isVideoStarted){
    if( (millis() - startTimeV) >= endTimeV){
        isVideoStarted = false;
        cameraRelease();
        Serial.println("Video streaming stopped");
    }
  }
  */
  ledManager.loop();
  checkWiFiStatus();
  textToSpeechLoop();
  if(!skipWdtRefresh){ // skip if set by botton pressed handler
    wdt.refresh(); // refresh the watchdog
  }
  
  //mqttManager.loop();
  //delay(10);
}

void getRstpLink()
{
    IPAddress ip = WiFi.localIP();
    
    rstpLink = "rtsp://" + String(ip.get_address()) + ":" +String(rtsp.getPort());
    Serial.println(rstpLink);
}
void printInfo(void)
{
    Serial.println("------------------------------");
    Serial.println("- Summary of Streaming -");
    Serial.println("------------------------------");
    Camera.printInfo();

    IPAddress ip = WiFi.localIP();

    
    Serial.println("- RTSP -");
    Serial.print("rtsp://");
    Serial.print(ip);
    Serial.print(":");
    rtsp.printInfo();
    rstpLink = "rtsp://" + String(ip.get_address()) + ":" +String(rtsp.getPort());
    Serial.println("- Audio -");
    audio.printInfo();

    Serial.print(" RSTP link = ");
    Serial.println(rstpLink);
}
