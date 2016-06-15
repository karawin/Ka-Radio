/* (c)jp cocatrix May 2006 
	quick and dirty websocket inplementation for wifi webradio
Inspirated by:
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the WebSockets for Arduino.
/*
 * Copyright 2016 karawin (http://www.karawin.fr)
*/

#ifndef __WEBSOCKET_H__
#define __WEBSOCKET_H__
// max size of the WS Message Header
#define WEBSOCKETS_MAX_HEADER_SIZE  (14)

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "esp_common.h"
#include "esp_softap.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "ssl/ssl_crypto.h"
#include "cencode_inc.h"
#include "c_types.h"

#define NBCLIENT 4
#define MAXDATA	 528
typedef enum {
    WSop_continuation = 0x00, ///< %x0 denotes a continuation frame
    WSop_text = 0x01,         ///< %x1 denotes a text frame
    WSop_binary = 0x02,       ///< %x2 denotes a binary frame
                              ///< %x3-7 are reserved for further non-control frames
    WSop_close = 0x08,        ///< %x8 denotes a connection close
    WSop_ping = 0x09,         ///< %x9 denotes a ping
    WSop_pong = 0x0A          ///< %xA denotes a pong
                              ///< %xB-F are reserved for further control frames
} wsopcode_t;

typedef struct {
        bool fin;
//        bool rsv1;
//        bool rsv2;
//        bool rsv3;
		wsopcode_t opCode;
        bool mask;
		size_t payloadLen;
        uint8_t * maskKey;
} wsMessageHeader_t;


typedef struct {
	int socket;	
} client_t;

struct websocketparam {
	int socket;
	char* buf;
	int len;
};	

// public:
// init some data
void websocketinit(void);
// a socket with a websocket . 
bool websocketnewclient(int socket);
// a socket with a websocket closed
void websocketremoveclient(int socket);
// is socket a websocket?
bool iswebsocket( int socket);
//read a message. close the connection if error
void websocketparsedata(int socket,char* buf, int recbytes);
// treat the received message (must be in the application);
void websockehandle(int socket, wsopcode_t opcode, uint8_t * payload, size_t length);
//write a txt data
void websocketwrite(int socket,char* buf, int len);
//broadcast a txt data to all clients
void websocketbroadcast(char* buf, int len);
//broadcast a txt data to all clients but the sender
void websocketlimitedbroadcast(int socket,char* buf, int len);
// the websocket server task
void websocketTask(void* pvParams) ;
//private:
bool sendFrame(int socket, wsopcode_t opcode, uint8_t * payload , size_t length );
// parse a new client request and prepare the answer
uint32_t decodeHttpMessage (char * inputMessage, char * outputMessage);
void wsclientDisconnect(int socket, uint16_t code, char * reason , size_t reasonLen );
#endif