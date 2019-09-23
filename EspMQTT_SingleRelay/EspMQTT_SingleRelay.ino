#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include "config.h"

// We use RX (PIN 3) as GPIO to control the relais!
// The relais module design is intended to be used with GPIO0
// but although GPIO0 is pulled high initally on boot there
// is also a short low pulse on boot which results in the relais
// initally to be activated without a way to surpress it...
// Who every thought thats a good idea when designing that...
// To make GPIO3 work you have to modify the board, see README.
#define GPIO_RELAIS 3

#ifdef MQTT_SSL_ENABLED
WiFiClientSecure wifi;
#else
WiFiClient wifi;
#endif
PubSubClient pubsub(wifi);

static char mqtt_device_id[80];
static char mqtt_topic_action[120];
static long timer_start;
static long timer_last_active;

void app_init() {
  Serial.begin(74880, SERIAL_8N1, SERIAL_TX_ONLY);

  pinMode(GPIO_RELAIS, FUNCTION_3);
  pinMode(GPIO_RELAIS, OUTPUT);
  digitalWrite(GPIO_RELAIS, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void app_setup() {

}

void app_handle_action_on() {
  digitalWrite(GPIO_RELAIS, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

void app_handle_action_off() {
  digitalWrite(GPIO_RELAIS, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
}

void app_handle_action_trigger() {
  app_handle_action_on();
  timer_start = millis();
}

void app_loop() {
  if (timer_start != -1 && millis() > APP_TRIGGER_TIME + timer_start) {
    app_handle_action_off();
    timer_start = -1;
  }

  // client.publish(temperature_topic, String(temp).c_str(), true);
}

void wifi_setup() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.println("MAC address: ");
  Serial.print(mac[0],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.println(mac[5],HEX);

#ifdef NTP_ENABLED
  wifi_set_clock();
#endif
}

void wifi_set_clock() {
  configTime(NTP_TIMEZONE, 0, "0.pool.ntp.org", "1.pool.ntp.org");

  Serial.println("Sync time via NTP");
  time_t t = time(nullptr);
  while (t < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    t = time(nullptr);
  }
  Serial.println("");

  struct tm timeinfo;
  gmtime_r(&t, &timeinfo);
  Serial.print("Time: ");
  Serial.print(asctime(&timeinfo));
}

void pubsub_setup() {
#ifdef MQTT_SSL_ENABLED
#ifdef MQTT_CA_SHA1_FINGERPRINT
  wifi.setFingerprint(MQTT_CA_SHA1_FINGERPRINT);
#else
  wifi.setInsecure();
#endif
#endif

  pubsub.setServer(MQTT_HOST, MQTT_PORT);
  pubsub.setCallback(pubsub_callback);

  // Format unique mqtt device id
  byte mac[6];
  WiFi.macAddress(mac);
  sprintf(mqtt_device_id, "%s%02x%02x%02x%02x%02x%02x", MQTT_DEVICE_ID_PREFIX, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Format mqtt subscription topic names
  sprintf(mqtt_topic_action, "%s%s%s", MQTT_TOPIC_PREFIX, mqtt_device_id, "/action");
}

void pubsub_callback(char* topic, byte* payload, unsigned int length) {
    String topicStr;
    String payloadStr;

    for (int i = 0; topic[i]; i++)
        topicStr += topic[i];
    for (int i = 0; i < length; i++)
        payloadStr += (char)payload[i];

    Serial.print("Message arrived - [");
    Serial.print(topicStr);
    Serial.print("] ");
    Serial.println(payloadStr);

    // Handle mqtt messages
    if (topicStr == mqtt_topic_action) {
#ifdef APP_ENABLE_ONOFF
      if (payloadStr == "on") {
        app_handle_action_on();
      }
      if (payloadStr == "off") {
        app_handle_action_off();
      }
#endif
#ifdef APP_ENABLE_TRIGGER
      if (payloadStr == "trigger") {
        app_handle_action_trigger();
      }
#endif
    }
}

void pubsub_connect() {
  while (!pubsub.connected()) {
    bool state;
#ifdef MQTT_USER && MQTT_PASSWORD
    Serial.println("Attempting MQTT connection with credentials");
    state = pubsub.connect(mqtt_device_id, MQTT_USER, MQTT_PASSWORD);
#else
    Serial.println("Attempting MQTT connection");
    state = pubsub.connect(mqtt_device_id);
#endif

    if (state) {
      Serial.print("Connected as ");
      Serial.println(mqtt_device_id);

      pubsub.subscribe(mqtt_topic_action);
      Serial.print("Subcribed to ");
      Serial.println(mqtt_topic_action);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(pubsub.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  app_init();

  app_setup();
  wifi_setup();
  pubsub_setup();
}

void loop() {
  if (!pubsub.connected()) {
    pubsub_connect();
  }
  pubsub.loop();
  app_loop();
}
