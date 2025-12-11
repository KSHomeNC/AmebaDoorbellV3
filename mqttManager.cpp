#include "mqttManager.h"
extern void cameraReStart();
extern void triggeredWD( unsigned long time_ms);
extern String getRstpLink();
MQTTManager *mqttMgr;
void mqttCallback(char* topic, byte* payload, unsigned int length);

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
   mqttMgr->mqttCB(topic, payload, length); 
}
void MQTTManager :: mqttCB(char* topic, byte* payload, unsigned int length)
{
    char buff[64];
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)(payload[i]));
    }
    Serial.println();
    String link = getRstpLink();
    link.toCharArray(buff, 64);

    client.publish( SUB_TOPIC_PLAY_RTSP_START, (const char *) buff, 1);
    cameraReStart();
    triggeredWD(60000); // 1 minute reboot time
}
void MQTTManager::begin() {
  espClient.setNonBlockingMode();
  client.setServer(mqtt_server, 1883);
  client.setSocketTimeout(10); // 10 sec
  client.setKeepAlive(10);    // 10 sec
  client.setCallback(mqttCallback);
  client.setPublishQos(1);
  Serial.print("mqtt QoS level");
  Serial.println(client.setPublishQos(1));
  reconnect();
  mqttMgr = this;
}


bool MQTTManager::reconnect() {
  const int maxRetries = 2;
  int retryCount = 0;

  while (!client.connected() && retryCount < maxRetries) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("Advance doorbell",WILL_TOPIC,1,true,WILL_MSG)) {
      Serial.println("connected");
      client.subscribe(SUB_TOPIC_PLAY_RTSP);
      //client.setPublishQos(1);
      client.publish(WILL_TOPIC,INIT_PUB_WILL_MSG,false);
      return true;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying...");
      retryCount++;
      delay(500);
    }
  }

  Serial.println("MQTT connection failed: Max retries reached");
  return false;
}

bool MQTTManager::publishMessage(PUB_TOPIC_TYPE topic, const char* message, uint8_t qos) {
  if (!client.connected()) {
    if (!reconnect()) {
      Serial.println("Failed to publish message: MQTT not connected");
      return false;
    }
  }
  Serial.print("mqtt QoS level");
  Serial.println(client.setPublishQos(qos));
  if (!client.publish(mqtt_topic[topic], message,false)) {
    Serial.println("Failed to publish message: Publish error");
    return false;
  }

  return true;
}

void MQTTManager::disconnect(){
  client.disconnect();
}

void MQTTManager::loop() {
  if (client.connected()) {
    client.loop();
  } else {
    reconnect();
  }
}
