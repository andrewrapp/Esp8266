/**
 * Copyright (c) 2015 Andrew Rapp. All rights reserved.
 *
 * This file is part of Esp8266
 *
 * Esp8266 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Esp8266 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Esp8266.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Esp8266.h>

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "HardwareSerial.h"

Esp8266::Esp8266() {
  // defaults//zero state
  // uint8_t resetPin = 9;
}

Stream* Esp8266::getEspSerial() {
  return esp;
}

Stream* Esp8266::getDebugSerial() {
  return debug;
}

void Esp8266::setDebugSerial(Stream* debugSerial) {
  debug = debugSerial;
}

// entry point.. lib can't function w/o serial port
void Esp8266::setEspSerial(Stream* espSerial) {
  esp = espSerial;
  // TODO make parameter
  esp->setTimeout(5000);
  startup = millis();
}

void Esp8266::setDataByteArray(uint8_t *data, uint8_t size) {
  dataByteArray = data;
  byteArraySize = size;
}

void Esp8266::setDataCharacterArray(char *data, uint8_t size) {
  dataCharacterArray = data;
  characterArraySize = size;
}

uint8_t Esp8266::getDataLength() {
  return dataLength;
}

uint8_t Esp8266::getChannel() {
  return channel;
}

bool Esp8266::isData() {
  return lastCommand == IPD_COMMAND;
}  

bool Esp8266::isConnect() {
  return lastCommand == CONNECTED_COMMAND;
}

bool Esp8266::isSuccess() {
  return lastResult == SUCCESS;
}

bool Esp8266::isError() {
  return !isSuccess();
}

bool Esp8266::isDisconnect() {
  return lastCommand == DISCONNECTED_COMMAND;
}

int Esp8266::getLastCommand() {
  return lastCommand;
}

int Esp8266::getLastResult() {
  return lastResult;
}

void Esp8266::closeAllConnections() {
  for (int i = 0; i < 10; i++) {
    if (connections[i]) {
      closeConnection(i);  
    }
  }
}

void Esp8266::resetEsp8266() {
  closeAllConnections();
  // TODO only if running as server
  stopServer();
  sendRestart();
}

int Esp8266::printDebug(char* text) {
  #ifdef DEBUG
    return getDebugSerial()->print(text);
  #endif
  
  return ERROR;
}

void Esp8266::configureEsp8266() {
/*
<---
AT(13)(13)(10)(13)(10)OK(13)(10)<--[11c],[14ms],[1w]
<---
AT+RST(13)(13)(10)(13)(10)OK(13)(10).|(209)N;(255)P:L(179)(10)BD;G8\(21)E5(205)N^f(5)(161)O'}5"B(201)(168)HHWV.V(175)H(248)<--[58c],[1926ms],[4w]
<---
AT+CWMODE=3(13)(13)(10)(13)(10)OK(13)(10)<--[20c],[25ms],[1w]
<---
AT+CIFSR(13)(13)(10)+CIFSR:APIP,"192.168.4.1"(13)(10)+CIFSR:APMAC,"1a:fe:34:9b:a7:4c"(13)(10)+CIRTP0.0(10)CRSM,8e4ba4(13)(10)(13)<--[96c],[164ms],[1w]
<---
AT+CIPMUX=1(13)(13)(10)(13)(10)OK(13)(10)<--[20c],[25ms],[1w]
<---
AT+CPSRV(164)(245)(197)b(138)(138)(138)(138)(254)(13)(13)(10)(13)(10)OK(13)(10)<--[26c],[18ms],[0w]
<---
AT+CIPMUX=1(13)(13)(10)(13)(10)OK(13)(10)<--[20c],[24ms],[1w]
<---
AT+C(160)M(21)IY(21)I(138)b(138)(138)(254)1(13)(13)(10)no(32)change(13)(10)<--[31c],[21ms],[0w]
*/

  sendAt();
  sendRestart();
  sendCwmode();
  //joinNetwork();
  sendCifsr();
  // configureServer();
  // enableMultiConnections();
  // startServer(80);
    
  resetCbuf(cbuf, BUFFER_SIZE);
  lastReset = millis();
}

// config to apply after reset or power cycle. everthing else should be retained
// TODO arguments
boolean Esp8266::configureServer(int listenPort) {
  return enableMultiConnections() == SUCCESS && startServer(listenPort) == SUCCESS;
}

// TODO static ip: AT+CIPSTA
// TODO rename atAt
int Esp8266::sendAt() {
  getEspSerial()->print("AT\r\n");
  
  readFor(100);  
  
  if (strstr(cbuf, "OK") == NULL) {
      return ERROR;
  }
  
  return SUCCESS;
}  

// TODO rename atRestart
int Esp8266::sendRestart() {
  getEspSerial()->print("AT+RST\r\n");
  
  readFor(2500);  
  
  if (strstr(cbuf, "OK") == NULL) {
      #ifdef DEBUG
        printDebug("RST fail");        
      #endif
      
      return ERROR;
  }

  lastReset = millis();  
  
  return SUCCESS;
}  

int Esp8266::sendCwmode() {
  getEspSerial()->print("AT+CWMODE=3\r\n");
  
  readFor(100);
  
  if (strstr(cbuf, "OK") == NULL) {
      #ifdef DEBUG
        printDebug("fail");        
      #endif    
      
      return ERROR;
  }
  
  return SUCCESS;
}

// TODO pass arguments
int Esp8266::joinNetwork(char* network, char* password) {  
  getEspSerial()->print("AT+CWJAP=\""); 
  getEspSerial()->print(network); 
  getEspSerial()->print("\",\"");
  getEspSerial()->print(password);
  getEspSerial()->print("\"\r\n");
  
  readFor(10000);
  
  if (strstr(cbuf, "OK") == NULL) {
      #ifdef DEBUG
        getDebugSerial()->println("Join fail");        
      #endif    
      
      return ERROR;
  }
  
  return SUCCESS;
}

// TODO parse ip address
int Esp8266::sendCifsr() {
  // on start we could publish our ip address to a known entity
  // or, get from router, or use static ip, 
  getEspSerial()->print("AT+CIFSR\r\n");
  readFor(200);
  
  // TODO parse ip address
  if (strstr(cbuf, "AT+CIFSR") == NULL) {
      #ifdef DEBUG
        getDebugSerial()->println("CIFSR fail");        
      #endif    
      
      return ERROR;
  }
  
  return SUCCESS;
}

// required for server mode
// set to 0 for single connection
int Esp8266::enableMultiConnections() {
  getEspSerial()->print("AT+CIPMUX=1\r\n"); 
  // if still connected: AT+CIPMUX=1(13)(13)(10)link(32)is(32)builded(13)(10)
  readFor(200);    

  if (!(strstr(cbuf, "OK") != NULL || strstr(cbuf, "builded"))) {
      #ifdef DEBUG
        getDebugSerial()->println("CIPMUX fail");        
      #endif    
      
      return ERROR;
  }
  
  return SUCCESS;
}

int Esp8266::closeConnection(int id) {
  getEspSerial()->print("AT+CIPCLOSE=");
  getEspSerial()->print(id);
  getEspSerial()->print("\r\n"); 
  
  readFor(200);    

  if (strstr(cbuf, "OK") == NULL) {
      #ifdef DEBUG
        getDebugSerial()->println("Close conn fail");        
      #endif    
      
      return ERROR;
  }
  
  return SUCCESS;  
}

// must enable multi connections prior to calling
int Esp8266::startServer(int listenPort) {
  return startStopServer(true, listenPort);  
}

// stop server and close connections. must call reset after
int Esp8266::stopServer() {
  return startStopServer(false, 0);
}

// TODO check result if stop when not running
int Esp8266::startStopServer(bool start, int listenPort) {
  getEspSerial()->print("AT+CIPSERVER="); 
  if (start) {
    getEspSerial()->print(1);
  } else {
    getEspSerial()->print(0);
  }
  
  if (start) {
    getEspSerial()->print(",");
    getEspSerial()->print(listenPort);    
  }

  getEspSerial()->print("\r\n");  
  
  readFor(500);  
  
  if (!(strstr(cbuf, "OK") != NULL || strstr(cbuf, "no change") || strstr(cbuf, "restart"))) {
      #ifdef DEBUG
        getDebugSerial()->println("CIPSERVER fail");        
      #endif    
      
      return ERROR;
  }  
  
  return SUCCESS;
}

void Esp8266::checkReset() {
  if (millis() - lastReset > resetEvery) {
    #ifdef DEBUG
      getDebugSerial()->println("Resetting");
    #endif
    
    resetEsp8266();
//    configureEsp8266();
  } 
}

void Esp8266::resetCbuf(char cbuf[], int size) {
 for (int i = 0; i < size; i++) {
   cbuf[i] = 0; 
 } 
}

void Esp8266::printCbuf(char cbuf[], int len) {
  #ifdef DEBUG  
    long start = millis();
  
    for (int i = 0; i < len; i++) {
        if (cbuf[i] <= 32 || cbuf[i] >= 127) {
          // not printable. print the char value
          getDebugSerial()->print("(");
          getDebugSerial()->print((uint8_t)cbuf[i]);
          getDebugSerial()->print(")");
        } else {
          getDebugSerial()->write(cbuf[i]);
        }
    }
   
    long end = millis();
    getDebugSerial()->print(" in ");
    getDebugSerial()->print(end - start);
    getDebugSerial()->println("ms");
  #endif
}

void Esp8266::printByteArray(uint8_t buf[], int len) {
  #ifdef DEBUG    
    for (int i = 0; i < len; i++) {
      getDebugSerial()->print("(");
      getDebugSerial()->print(buf[i]);
      getDebugSerial()->print(")");
    }
  #endif
}

int Esp8266::readBytes(uint8_t buf[], int startAt, int len, int timeout) {  
  int pos = startAt;
  long start = millis();

  while (millis() - start < timeout) {          
    if (getEspSerial()->available() > 0) {
      uint8_t in = getEspSerial()->read();      
  
      #ifdef DEBUG    
        getDebugSerial()->print("(");
        getDebugSerial()->print(in);
        getDebugSerial()->print(")");
      #endif

      buf[pos++] = in;
      
      if (pos == len) {
        return len;
      }    
    }
  }
  
  if (millis() - start >= timeout) {
    // timeout
      #ifdef DEBUG    
        getDebugSerial()->print("pos ");
        getDebugSerial()->print(pos);
      #endif

    return ERROR;
  }

  return pos;  
}

bool Esp8266::find(char pattern[], int timeout) {  
  long start = millis();
  int matchAt = 0;

  while (millis() - start < timeout) {          
    if (getEspSerial()->available() > 0) {
      uint8_t in = getEspSerial()->read();
      
      if (pattern[matchAt] == (char) in) {
        // match
        if (matchAt == strlen(pattern) - 1) {
          return true;
        } else {
          // match on next char
          matchAt++;
        }
      } else {
        if (matchAt > 0) {
          matchAt = 0;

          if (pattern[matchAt] == (char) in) {
            matchAt++;
          }          
        }
      }
    }
  }

  // not found
  return false;
}

int Esp8266::readChars(char cbuf[], int startAt, int len, int timeout) {  
  int pos = startAt;
  long start = millis();

  while (millis() - start < timeout) {          
    if (getEspSerial()->available() > 0) {
      uint8_t in = getEspSerial()->read();
      
      if (in <= 32 || in >= 127 || in == 0) {
        // TODO debug print warning 
      }
      
      cbuf[pos++] = in;
      
      if (pos == len) {
        // null terminate
        cbuf[pos] = 0;
        return len;
      }    
    }
  }
  
  // timeout
  return ERROR;
}

int Esp8266::readBytesUntil(char cbuf[], uint8_t match, int maxReadLen, int timeout) {  
  int pos = 0;
  long start = millis();

  while (millis() - start < timeout) {          
    if (getEspSerial()->available() > 0) {
      uint8_t in = getEspSerial()->read();
      
      // don't include the match byte
      if (match == in) {        
        return pos;  
      }
      
      cbuf[pos++] = in;
      
      if (pos == maxReadLen) {
        return 0;
      }    
    }
  }

  return -1;  
}

// debugging aid
int Esp8266::readFor(int timeout) {
  long start = millis();
  long lastReadAt = 0;
  bool waiting = false;
  long waits = 0;
  int pos = 0;
  
  #ifdef DEBUG
    getDebugSerial()->println("<--");
  #endif
  
  resetCbuf(cbuf, BUFFER_SIZE);
  
  while (millis() - start < timeout) {          
    if (getEspSerial()->available() > 0) {
      if (waiting) {
        waits++;
        waiting = false; 
      }
      
      uint8_t in = getEspSerial()->read();
      cbuf[pos] = in;
      
      lastReadAt = millis() - start;
      pos++;
      
      if (in <= 32 || in >= 127) {
        // not printable. print the char value
        #ifdef DEBUG
          getDebugSerial()->print("("); 
          getDebugSerial()->print(in); 
          getDebugSerial()->print(")");
        #else
          delay(2);
        #endif
      } else {
        // pass through
        #ifdef DEBUG
          getDebugSerial()->write(in);
        #endif
      }
    } else {
      waiting = true;
    }
  }
  
  // null terminate
  cbuf[pos] = 0;
  
  #ifdef DEBUG
    getDebugSerial()->print("<--[");
    getDebugSerial()->print(pos);
    getDebugSerial()->print("c],[");
    getDebugSerial()->print(lastReadAt);
    getDebugSerial()->print("ms],[");
    getDebugSerial()->print(waits);  
    getDebugSerial()->println("w]");    
  #endif
  
  return pos;
}

int Esp8266::getCharDigitsLength(int number) {
  if (number < 10) {
    return 1;
  } else if (number < 100) {
    return 2;
  } else if (number < 1000) {
    return 3;
  }
}  

// int Esp8266::send(int channel, char message[]) {
//   send(channel, message, NULL);
// }

// int Esp8266::send(int channel, uint8_t message[], uint8_t length) {
//   send(channel, NULL, message);
// }

int Esp8266::send(int channel, char message[]) {
  return send(channel, message, NULL, 0);
}

// TODO
int Esp8266::send(int channel, uint8_t message[], int length) {
  return send(channel, NULL, message, length);
}

int Esp8266::send(int channel, char charMessage[], uint8_t byteMessage[], int byteLength) {
  
  if (!connections[channel]) {
    // not connected on this id
    return ERR_NOT_CONNECTED;
  }
  // NOTE: print or write works for char data, must use print for non-char to print ascii
  
  int sendLen = 0;

  if (charMessage != NULL) {
    sendLen = strlen(charMessage) + 2;
  } else if (byteMessage != NULL) {
    sendLen = byteLength + 2;
  } else {
    return ERROR;
  }
  
  // send AT command with channel and message length
  // TODO get write len
  getEspSerial()->print("AT+CIPSEND="); 
  getEspSerial()->print(channel); 
  getEspSerial()->print(","); 
  getEspSerial()->print(sendLen); 
  getEspSerial()->print("\r\n");
  // don't use flush!
  //getEspSerial()->flush();
  
  // replies with
  //(13)(10)(13)(10)OK(13)(10)AT+CIPSEND=0,4(13)(13)(10)>(32)<--[27]
  
  // should be same size everytime + send length                    
  int cmdLen = 26 + getCharDigitsLength(sendLen);
  
  int rCmdLen = readChars(cbuf, 0, cmdLen, 5000);
  
  if (rCmdLen == -1) {
    #ifdef DEBUG
      getDebugSerial()->println("CIPSEND timeout");
    #endif
    
    return ERR_CIPSEND_TIMEOUT;
  } else if (rCmdLen != cmdLen) {
    #ifdef DEBUG
      getDebugSerial()->print("Error unexp. reply len: "); 
      getDebugSerial()->println(rCmdLen);
    #endif
    
    return ERR_CIPSEND_REPLY_LEN;
  }
  
  if (strstr(cbuf, "busy") != NULL) {
    #ifdef DEBUG
      getDebugSerial()->print("Busy");
    #endif
    
    return ERR_BUSY;
  } else if (strstr(cbuf, "OK") == NULL) {
    #ifdef DEBUG
      getDebugSerial()->print("Error: ");
      getDebugSerial()->println(cbuf);
    #endif
    
    return -8;
  }
  
  #ifdef DEBUG 
    getDebugSerial()->println("CIPSEND reply");
    printCbuf(cbuf, cmdLen);
  #endif
  
  // send data to client
  if (charMessage != NULL) {
    getEspSerial()->print(charMessage);
  } else if (byteMessage != NULL) {
    getEspSerial()->write(byteMessage, byteLength);
  } else {
    return ERROR;
  }

  getEspSerial()->print("\r\n");
  getEspSerial()->flush();
  
  // reply
  //ok(13)(13)(10)SEND(32)OK(13)(10)<--[14]

  // fixed length reply
  // timed out a few times over 12h period with 1s timeout. increasing timeout to 5s
  // TODO make timeouts configurable

  // Stream find doesn't work and easier to write a function than figure out where it fails
  // if (getEspSerial()->find("SEND OK")) {

  // look for SEND OK
  if (find("SEND OK", 5000)) {
    getEspSerial()->read();
    getEspSerial()->read();
  } else {
    return ERROR;
  }

  // int len =  0;
  // if (charMessage != NULL) {
  //   // FIXME charMessage must not exceed cbuf size + 12
  //   len = readChars(cbuf, 0, strlen(charMessage) + 12, 5000);
  // } else if (byteMessage != NULL) {
  //   len = readBytes(NULL, 0, byteLength + 12, 5000);
  // }
  
  // if (len == -1) {
  //   #ifdef DEBUG 
  //     getDebugSerial()->println("Data send timeout");            
  //   #endif
  //   return -11;
  // } else if (len != 12 + strlen(charMessage)) {
  //   #ifdef DEBUG 
  //     getDebugSerial()->println("Reply len err");                  
  //   #endif     
  //   return -12; 
  // }

  // if (strstr(cbuf, "OK") == NULL) {
  //   #ifdef DEBUG     
  //     getDebugSerial()->print("Error: ");
  //     getDebugSerial()->println(cbuf);
  //   #endif
  //   return -9;
  // }
  
  // #ifdef DEBUG     
  //   getDebugSerial()->println("Data reply");
  //   printCbuf(cbuf, len);        
  // #endif  
  
  return SUCCESS;
}

int Esp8266::handleData() {
  //(13)(10)+IPD,0,4:hi(13)(10)(13)(10)OK(13)(10)
  
  #ifdef DEBUG      
    getDebugSerial()->println("Received data (IPD)");
  #endif      

  //ex +IPD,0,10:hi(32)again(13)
  //cbuf ,0,10:
  
  // serial buffer is at comma after D
  char* ipd = cbuf + COMMAND_LEN;
  
  // max channel + data length + 2 commas + colon = 9
  int len = readBytesUntil(ipd, ':', BUFFER_SIZE - COMMAND_LEN, 3000);
  
  if (len == 0) {
    // not found
    return -1;
  } else if (len == -1) {
    // timeout
    return -2;
  }

  // space ,0,1:(32)(13)(10)OK(13)(10)(13)(10)
  
  // parse channel
  // null term after channel for atoi
  ipd[2] = 0;
  channel = atoi(ipd + 1);
  // reset
  ipd[2] = ',';
  
  #ifdef DEBUG
    getDebugSerial()->print("On channel "); 
    getDebugSerial()->println(channel);
  #endif
  
  //ipd[9] = 0;
  // length starts at pos 3
  len = atoi(ipd + 3);
  
  // subtract 2, don't want lf/cr
  len-=2;
  
  if (len <= 0) {
    #ifdef DEBUG
      getDebugSerial()->println("no data");
    #endif
    return -2; 
  }
  
  dataLength = len;

  // reset so we can print
  
  #ifdef DEBUG
    getDebugSerial()->print("Data len "); 
    getDebugSerial()->println(dataLength);
  #endif

  int rlen = 0;

  if (dataByteArray != NULL) {
    // TODO check the data array length is <= to dataLength
    if (dataLength > byteArraySize) {
      return -98;
    }

    rlen = readBytes(dataByteArray, 0, dataLength, 3000);
  } else if (dataCharacterArray != NULL) {
    // read input into data buffer
    if (dataLength > characterArraySize) {
      return -98;
    }

    rlen = readChars(dataCharacterArray, 0, dataLength, 3000);
  } else {
    return -99;
  }

  if (rlen == -1) {
    // timeout
    return -4;
  } else if (rlen != dataLength) {
    #ifdef DEBUG
      getDebugSerial()->print("Unable to read expected bytes: "); 
      getDebugSerial()->println(dataLength);            
    #endif
    return -4;
  }
  
  #ifdef DEBUG
    getDebugSerial()->print("Data:");   
    if (dataByteArray != NULL) {
      printByteArray(dataByteArray, dataLength);
    } else if (dataCharacterArray != NULL) {
      printCbuf(dataCharacterArray, dataLength);      
    }
    getDebugSerial()->println("");  
  #endif

  return SUCCESS;
}

int Esp8266::parseChannel(char *cbuf, bool open) {

  cbuf[1] = 0;
  int temp = atoi(cbuf);
  // replace char
  cbuf[1] = ',';
  
  if (temp >= 0 && temp <= 10) {
    connections[temp] = open;  
    return temp;  
  } else {
    return -1;
  }  
}

int Esp8266::handleConnected() {
  //0,CONNECT(13)(10)  
  channel = parseChannel(cbuf, true);
      
  #ifdef DEBUG  
    getDebugSerial()->print("Connected on ");
    getDebugSerial()->println(channel);
  #endif

  //CONNECT_CMD_LEN - (COMMAND_LEN + 1)
  int len = readBytesUntil(cbuf + COMMAND_LEN, 10, CONNECT_CMD_LEN - COMMAND_LEN, 3000);
  
  if (len > 0) {
    return SUCCESS;
  } else {
    return ERROR; 
  }
}

int Esp8266::handleClosed() {
  //0,CLOSED(13)(10)
  channel = parseChannel(cbuf, false);

  #ifdef DEBUG  
    getDebugSerial()->print("Disconnected on ");
    getDebugSerial()->println(channel);
  #endif
  
  // consume rest of command
  int len = readBytesUntil(cbuf + COMMAND_LEN, 10, CLOSED_CMD_LEN - COMMAND_LEN, 3000);  
  
  if (len > 0) {    
    return SUCCESS;
  } else {
    return ERROR; 
  }  
}

boolean Esp8266::readSerial() {
  // reset
  lastResult = SUCCESS;
  lastCommand = NO_COMMAND;
  resetCbuf(cbuf, BUFFER_SIZE);    
  // TODO clear data/char arrays
  
  // the tricky part of AT commands is they vary in length and format, so we don't know how much to read without knowing what it is
  // There are no commands less than 6 chars and with 6 chars we should be able to identify all possible commands
  if (getEspSerial()->available() >= COMMAND_LEN) {
    #ifdef DEBUG      
      getDebugSerial()->print("\n\nSerial available "); 
      getDebugSerial()->println(getEspSerial()->available());
    #endif
    
    int len = readChars(cbuf, 0, COMMAND_LEN, 1000);
    
    #ifdef DEBUG
      printCbuf(cbuf, COMMAND_LEN);
    #endif
    
    // not using Serial.find because if it doesn't match we lose the data. so not helpful

    if (strstr(cbuf, "+IPD") != NULL) {
      //(13)(10)+IPD,0,4:hi(13)(10)(13)(10)OK(13)(10) 
      lastResult = handleData();
      lastCommand = IPD_COMMAND;
    } else if (strstr(cbuf, ",CONN") != NULL) {      
      //0,CONNECT(13)(10)      
      lastResult = handleConnected();
      lastCommand = CONNECTED_COMMAND;
    } else if (strstr(cbuf, "CLOS") != NULL) {
      lastResult = handleClosed();
      lastCommand = DISCONNECTED_COMMAND;     
    } else {
      #ifdef DEBUG
        getDebugSerial()->println("Unexpected..");
      #endif

      lastResult = ERROR;
      lastCommand = UNKNOWN_COMMAND;
      
      readFor(2000);
    }

    #ifdef DEBUG
      if (lastResult != SUCCESS) {
        getDebugSerial()->print("Loop error [");
        getDebugSerial()->print(lastCommand);
        getDebugSerial()->print("]: ");      
        getDebugSerial()->println(lastResult);
      } else {
        getDebugSerial()->println("ok");
      }
    #endif
    
    // if (false) {
    //   // health check        
    //   if (sendAt() != 1) {
        
    //   }
    // }
  }

  if (lastResult == SUCCESS) {
    return true;
  }

  return false;
}
