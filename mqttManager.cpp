#include "mqttManager.h"

void MQTTManager::begin() {
  client.setServer(mqtt_server, 1883);
  client.setSocketTimeout(200);
  client.setKeepAlive(60000);
  reconnect();
}

bool MQTTManager::reconnect() {
  const int maxRetries = 2;
  int retryCount = 0;

  while (!client.connected() && retryCount < maxRetries) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ArduinoClient")) {
      Serial.println("connected");
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

bool MQTTManager::publishMessage(PUB_TOPIC_TYPE topic, const char* message) {
  if (!client.connected()) {
    if (!reconnect()) {
      Serial.println("Failed to publish message: MQTT not connected");
      return false;
    }
  }

  if (!client.publish(mqtt_topic[topic], message)) {
    Serial.println("Failed to publish message: Publish error");
    return false;
  }

  return true;
}

void MQTTManager::loop() {
  if (client.connected()) {
    client.loop();
  } else {
    reconnect();
  }
}
