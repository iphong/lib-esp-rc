# EspRC Library for ESP8266

ESP-NOW protocol made simple for quick & easy implementation.

### Usages

IMPORTANT: esp-now protocol doesn't required wifi to be connected to a network,
but it still need wifi to be turned on in order to work. If you are not using
wifi, set the wifi mode to station and disconnect wifi. EspRC will work
as long as wifi connection is being used. And don't let wifi to go to sleep.

```
WiFi.mode(WIFI_STA);
WiFi.disconnect();
```

### Basic examples

Upload the following code onto 2 esp8266 devices, then open both serial 
terminals and start typing...

```c++

#include "MeshRC.h"

void setup() {
    	Serial.begin(921600);

    	MeshRC.begin(1);
		MeshRC.on("message", []() {
		String msg = MeshRC.getValue();
		Serial.printf("received: %s \n", msg.c_str());
	});
}
void loop() {
	if (Serial.available()) {
		MeshRC.send("message", Serial.readStringUntil('\n'));
	}
}
```

### Advanced examples

This project pushes the ESP8266 to its limit. check it out at https://github.com/iphong/esp-visual-led
