#define WIFI_SSID  "<YourWifiSSID>"
#define WIFI_PSK  "<YourWifiPassword>"

#define MQTT_DEVICE_ID_PREFIX "client_"
#define MQTT_TOPIC_PREFIX "espmqtt/"
#define MQTT_HOST "test.mosquitto.org"
#define MQTT_PORT 8883
//#define MQTT_USER "user"
//#define MQTT_PASSWORD "password"
#define MQTT_SSL_ENABLED
#define MQTT_CA_SHA1_FINGERPRINT "E6:2D:6F:0D:95:7E:D2:0B:74:B1:D5:5D:40:4E:EF:99:2C:E4:82:F0"

//#define NTP_ENABLED
#define NTP_TIMEZONE 0 * 3600 // UTC

#define APP_ENABLE_ONOFF
#define APP_ENABLE_TRIGGER
#define APP_TRIGGER_TIME 2000
