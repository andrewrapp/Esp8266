# Esp8266
Arduino library for esp8266 with AT firmware

Currently supports esp in server mode

There are many versions of AT firmware floating around for the esp device. This library works with AT firmware from expressif https://github.com/espressif/esp8266_at.git (commit 4d000)

I've written a guide for getting started with the esp8266 https://medium.com/@nowir3s/getting-started-with-esp8266-875fb54441d6

Once the esp8266 has the above firmware and is connected to a wifi network it's ready for use with this library

Caveats: Don't send more than around 40 bytes at a time, including "\r\n" or it will fail. I've tested with 32 bytes and no issues so far. It's recommended to send back an ACK before sending more data from the client or the serial buffer can overflow.

Performance: At 9600 baud I see around 8 transmissions of 32 bytes each, per second. This includes the roundtrip ACK.

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

// now connect and send data from a client

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






