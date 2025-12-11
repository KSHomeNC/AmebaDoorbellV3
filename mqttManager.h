#ifndef __MQTT_MANAGER__
#define __MQTT_MANAGER__

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

typedef enum{
  PUB_TOPIC_BELL,
  PUB_TOPIC_PERSON_UNKNOWN,
  PUB_TOPIC_PERSON_KNOWN,
}PUB_TOPIC_TYPE;

#define SUB_TOPIC_PLAY_RTSP "doorbell/rtsp"
#define SUB_TOPIC_PLAY_RTSP_START "doorbell/start"
#define WILL_TOPIC "doorbell/cnx/status"
#define WILL_MSG "offline"
#define INIT_PUB_WILL_MSG ""
class MQTTManager {
  private:
    WiFiClient espClient;
    PubSubClient client;
    const char* mqtt_server = "192.168.12.198";
    const char mqtt_topic[3][128] = {"doorbell/live","doorbell/unknown","doorbell/known"};
  
  public:
    MQTTManager() : client(espClient) {}

    void begin();

    bool reconnect();

    void disconnect();

    bool publishMessage(PUB_TOPIC_TYPE topic, const char* message, uint8_t qos);
    
    void mqttCB(char* topic, byte* payload, unsigned int length);
    
    void loop();
};

#endif //__MQTT_MANAGER__