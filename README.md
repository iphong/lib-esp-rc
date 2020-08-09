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

### Examples

```c++

#include "EspRC.h"

void setup() {

  EspRC.begin(1);
  
  EspRC.send("foo");
  EspRC.on("foo", [](){
    // Do somesimthing
  });

  EspRC.send("bar", "hello");
  EspRC.on("bar", [](String data){
    data.equals("hello");
  });
}

```
