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

class MQTTManager {
  private:
    WiFiClient espClient;
    PubSubClient client;
    const char* mqtt_server = "192.168.12.198";
    const char mqtt_topic[3][128] = {"doorbell/liveTest","doorbell/unknown","doorbell/known"};
  
  public:
    MQTTManager() : client(espClient) {}

    void begin();

    bool reconnect();

    bool publishMessage(PUB_TOPIC_TYPE topic, const char* message);

    void loop();
};

#endif //__MQTT_MANAGER__