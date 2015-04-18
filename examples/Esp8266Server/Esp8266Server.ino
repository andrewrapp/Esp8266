#include <Esp8266.h>
#include <SoftwareSerial.h>

// receives ESP TX pin
#define ESP_RX 3
// transmits to ESP RX pin
#define ESP_TX 4

// Keep in mind that Serial is directional and should be wired as follows:
// TX (device 1) -> RX (device 2)
// RX (device 1) <- TX (device 2)
// So, connect the
//    ESP8266 TX to the Arduino RX pin number ESP_RX
// and the 
//    ESP8266 RX to the Arduino TX pin number ESP_TX

SoftwareSerial espSerial(ESP_RX, ESP_TX);
Esp8266 esp8266 = Esp8266();

int serverPort = 1111;

uint8_t data[32];

void setup() {
  // delay is necessary if esp8266 is booting, since it will spew the boot garbage for about 3 seconds. any setup commands would fail
  delay(3000);  
  espSerial.begin(9600);
  esp8266.setEspSerial(&espSerial);  
  Serial.begin(9600); while(!Serial); // UART serial debug
  esp8266.setDebugSerial(&Serial);
  // make sure this is big enough to read your packet or you'll get unexpected results
  esp8266.setDataByteArray(data, 32);
  // listen on port 80
  while (!esp8266.configureServer(serverPort)) {
      Serial.println("Failed to configure server");
      delay(3000);
  }
}

void loop() {
  if (esp8266.readSerial()) {
    if (esp8266.isData()) {  
      Serial.print("Got data, len ");
      Serial.print(esp8266.getDataLength());
      Serial.print(" on channel ");
      Serial.println(esp8266.getChannel());
      
      esp8266.send(esp8266.getChannel(), "ok");
      
      // or send byte array
//      uint8_t buf[3];
//      buf[0] = 0;
//      buf[1] = 1;
//      buf[2] = 2;
//      esp8266.send(esp8266.getChannel(), buf, 3);
    } else if (esp8266.isConnect()) {
      Serial.print("Connected on channel ");
      Serial.println(esp8266.getChannel());     
    } else if (esp8266.isDisconnect()) {
      Serial.print("Disconnected on channel ");
      Serial.println(esp8266.getChannel());     
    } 
  } else if (esp8266.isError()) {
    Serial.print("Failed on command ");
    Serial.print(esp8266.getLastCommand());
    Serial.print(" with error ");
    Serial.println(esp8266.getLastResult());    
    
    if (esp8266.getLastCommand() == UNKNOWN_COMMAND) {
      Serial.print("Resetting");
      
      // assume the worst and reset
      esp8266.resetEsp8266();
      // need to apply server config which is lost on restart
      esp8266.configureServer(serverPort);
    }
  } else {
   // no data 
  }
}
