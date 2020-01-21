## Dependencies
Ardunio Libraries:
 * PubSubClient (2.7.0) - https://github.com/knolleary/pubsubclient

## EspMQTT_SingleRelay
### Hardware
Buy on AliExpress: https://www.aliexpress.com/item/32969807508.html  
Price: ~ 2,17 €

[TODO: insert image]

### Required modifications
The linked **relay module** is the latest version (**v4**) and in its stock configuration it is **active-low** and can be triggered via **GPIO0** pulled low. **Problem**: Although this pin is active-high on the ESP8266 as documented in the datasheet **the on-board bootloader pulls the pin low for a short period of time** as part of its boot sequence. This results in the relay closing for a short time every time the module is powered up. Obviously this not acceptable for most applications.

There are various hacks to avoid this but the only proper way is to re-route the relay pin to a GPIO which stays high until explicitly pulled down via application code.
For the ESP-01 we are going to **repurpose the RX pin** as its state is not modified by the bootloader. Please check the code for instructions how to change the function of the **RX pin to GPIO3**.

The following documents the required hardware changes:
 * The **connection of GPIO0** of the ESP-01 board **to the relay** on the relay board **needs to be separated**.
  NOTE: To program the ESP-01 GPIO0 still needs to be pulled to GND so its not a good idea to just cut off the pin. The better solution is to melt the solder of the pin and push the pin upwards until its end is on the same level as the plastic. Now its still possible to connect GND to the pin from the opposite site. 
  [TODO: insert image]
 * **On the relay board** the **GPIO0 and RX** (right next to GPIO0) **have to be connected**. This will ensure that when RX is pulled low it will close the relay. As RX itself is not connected to any other part on the relay module this is the only modification required.
  [TODO: insert image]

### Configuration
See [config.h](EspMQTT_SingleRelay/config.h).

### Test
```
#define APP_ENABLE_TRIGGER
#define APP_TRIGGER_TIME 2000
mosquitto_pub -h localhost -p 1883 -t "espmqtt/client_4c11ae0e3acb/action" -m "trigger"

#define APP_ENABLE_ONOFF
mosquitto_pub -h localhost -p 1883 -t "espmqtt/client_4c11ae0e3acb/action" -m "on"
mosquitto_pub -h localhost -p 1883 -t "espmqtt/client_4c11ae0e3acb/action" -m "off"
```

