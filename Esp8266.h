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
 
#ifndef Esp8266_
#define Esp8266_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <inttypes.h>
#include <SoftwareSerial.h>

#define DEBUG

#define SUCCESS 0
#define ERROR -1
#define TIMEOUT -2 

#define UNKNOWN_COMMAND -1
#define IPD_COMMAND 1
#define CONNECTED_COMMAND 2
#define DISCONNECTED_COMMAND 3

#define NO_COMMAND 0
#define NO_RESULT 0

#define ERR_NOT_CONNECTED -10
#define ERR_CIPSEND_TIMEOUT -11
#define ERR_CIPSEND_REPLY_LEN -12
#define ERR_BUSY -13


// how many chars to read to identify command
#define COMMAND_LEN 6
#define CONNECT_CMD_LEN 11
#define CLOSED_CMD_LEN 10

// TODO determine min arrary size for parsing AT commands
#define BUFFER_SIZE 64 

class Esp8266 {
public:
	Esp8266();
	Stream* getDebugSerial();
	void setDebugSerial(Stream* debugSerial);	
	Stream* getEspSerial();
	void setEspSerial(Stream* espSerial);
	//void setup();
	void restartEsp8266();
	int printDebug(char* text);
	void configureEsp8266();
	boolean configureServer(int listenPort);
	void closeAllConnections();
	int sendAt();
	int sendRestart();
	int sendCwmode();
	int joinNetwork(char* network, char* password);
	int sendCifsr();
	int enableMultiConnections();
	int closeConnection(int id);	
	int startServer(int listenPort);
	int stopServer();
	boolean readSerial();
	// Data array for incoming IPD commands
	// choose one or the other. only one will be used if both are set
	// allocate enough memeory for the largest data packet sent to device.
	void setDataByteArray(uint8_t *data, uint8_t size);
	void setDataCharacterArray(char *data, uint8_t size);
	uint8_t getDataLength();
	// channel of last command
	uint8_t getChannel();
	int send(int channel, char message[]);
	int send(int channel, uint8_t message[], int length);
	int getLastCommand();
	int getLastResult();
	bool isSuccess();
	bool isError();	
	bool isData();	
	bool isConnect();
	bool isDisconnect();;
	void debug(uint8_t);	
	void debug(char cbuf[]);	
	void debug(char cbuf[], int len);	
	void debug (uint8_t message[], int length);	
private:	
	bool find(char pattern[], int timeout);
	int send(int channel, char charMessage[], uint8_t byteMessage[], int byteLength);
	int startStopServer(bool start, int listenPort);
	void clearRead();
	int readChars(char cbuf[], int startAt, int len, int timeout);
	int readBytes(uint8_t buf[], int startAt, int len, int timeout);
	int readBytesUntil(char cbuf[], uint8_t match, int maxReadLen, int timeout);
	int readFor(int timeout);
	int getCharDigitsLength(int number);
	int handleData();
	int parseChannel(char *cbuf, bool open);
	int handleConnected();
	int handleClosed();
	void resetCbuf(char cbuf[], int size);
	void checkReset();

	char cbuf[BUFFER_SIZE];
	// documentation seems to indciate a max of 5 connections but not very clear
	bool connections[10];	
	uint8_t *dataByteArray;
	uint8_t byteArraySize;
	char *dataCharacterArray;
	uint8_t characterArraySize;
	//int listenPort;
	Stream* debugStream;
	Stream* esp;
	long resetEvery;
	long lastRestart;
	uint8_t dataLength;
	uint8_t channel;
	int lastResult;
	int lastCommand;
	long startup;
};

#endif // guard
