#include <Esp8266.h>
#include <SoftwareSerial.h>

// If using a 3.3V Pro you will likely encounter problems uploading at 115.2K. To remedy, I recommend running optiboot at 57.6K

// Keep in mind that Serial is directional and should be wired as follows:
// TX (device 1) -> RX (device 2)
// RX (device 1) <- TX (device 2)

// receives ESP TX pin
#define ESP_RX 3
// transmits to ESP RX pin
#define ESP_TX 4

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

//  if (esp8266.configure("ssid","password") != SUCCESS) {
//    Serial.println("Configure failed");
//    for (;;) {}
//  }

  while (!esp8266.configureServer(serverPort)) {
      Serial.println("Failed to configure server");
      delay(3000);
  }
}

void loop() {
  if (esp8266.readSerial()) {
    if (esp8266.isData()) {  
      /*
      Serial.print("Got data, len ");
      Serial.print(esp8266.getDataLength());
      Serial.print(" on channel ");
      Serial.println(esp8266.getChannel());
      */
      
      // see what we got
      for (int i = 0; i < esp8266.getDataLength(); i++) {
        Serial.print(data[i]);  
      }
      
      // output to debug
      //esp8266.debug(data, 32);
      
      if (esp8266.send(esp8266.getChannel(), "ok") == SUCCESS) {
       // success 
      } else {
       // failed to send 
      }
      
      // or send byte array
//      uint8_t buf[1];
//      buf[0] = 0;
//      esp8266.send(esp8266.getChannel(), buf, 1);
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
      esp8266.restartEsp8266();
      // need to apply server config which is lost on restart
      esp8266.configureServer(serverPort);
    }
  } else {
   // no data 
  }
}
