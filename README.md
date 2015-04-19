# esp8266
Arduino library for esp8266 with AT firmware

Currently supports esp as a server

First configure the esp to connect to a wifi network and get it's ip address. See https://medium.com/@nowir3s/getting-started-with-esp8266-875fb54441d6

Example:

```
#include <Esp8266.h>
#include <SoftwareSerial.h>

// create an esp instance
Esp8266 esp8266 = Esp8266();
// use softserial to free the serial port for debug/other use
SoftwareSerial espSerial(ESP_RX, ESP_TX);
// connect at the speed esp is configured for. NOTE softserial is limited to 19200
espSerial.begin(9600);
// provide esp with the serial port
esp8266.setEspSerial(&espSerial);  
// use uart for debug
esp8266.setDebugSerial(&Serial);
// create an data array for incoming data
uint8_t data[32];
// provide a data array to the library
esp8266.setDataByteArray(data, 32);
// start listening on port 1066
esp8266.configureServer(1066);

// now connect and send data from a client (e.g. 

// check for data (do in loop)
if (esp8266.readSerial()) {
  if (esp8266.isData()) {  
    // got data.. do something with it!
    if (data[0] == ??) {
      //??
    }
    
    // send a reply to the client on the connection
    esp8266.send(esp8266.getChannel(), "thanks for the data!");
  }
}
```

Complete example https://github.com/andrewrapp/Esp8266/blob/master/examples/Esp8266Server/Esp8266Server.ino






