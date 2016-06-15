/*
 * Copyright 2016 karawin (http://www.karawin.fr)
*/
#ifndef __WEBCLIENT_H__
#define __WEBCLIENT_H__

#include "c_types.h"
#include "websocket.h"

#define METADATA 9
#define METAINT 8
#define BITRATE 5
#define METANAME 0
#define ICY_HEADERS_COUNT 9
#define ICY_HEADER_COUNT 10
#define RECEIVE 2000
extern uint16_t currentStation;

struct icyHeader
{
	union
	{
		struct
		{
			char* name;
			char* notice1;
			char* notice2;
			char* url;
			char* genre;
			char* bitrate;
			char* description;
			char* audioinfo;
			int metaint;
			char* metadata;
		} single;
		char* mArr[ICY_HEADER_COUNT];
	} members;
};



static const char* icyHeaders[] = { "icy-name:", "icy-notice1:", "icy-notice2:",  "icy-url:", "icy-genre:", "icy-br:","icy-description:","ice-audio-info:", "icy-metaint:" };


enum clientStatus { C_HEADER, C_HEADER1,C_METADATA, C_DATA, C_PLAYLIST, C_PLAYLIST1 };

void clientInit();
uint8_t clientIsConnected();

void clientSetURL(char* url);
void clientSetPath(char* path);
void clientSetPort(uint16_t port);
struct icyHeader* clientGetHeader();
void clientConnect();
void clientDisconnect();
void clientTask(void *pvParams);
void vsTask(void *pvParams) ;
void wsVol(char* vol);
void wsMonitor();
void wsStationNext();
void wsStationPrev();
#endif
