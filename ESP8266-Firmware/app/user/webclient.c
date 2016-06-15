/*
 * Copyright 2016 karawin (http://www.karawin.fr)
*/

#include "webclient.h"
#include "webserver.h"

#include "lwip/sockets.h"
#include "lwip/api.h"
#include "lwip/netdb.h"

#include "esp_common.h"

#include "freertos/semphr.h"

#include "vs1053.h"
#include "eeprom.h"

static enum clientStatus cstatus;
//static uint32_t metacount = 0;
//static uint16_t metasize = 0;


xSemaphoreHandle sConnect, sConnected, sDisconnect, sHeader;

static uint8_t connect = 0, playing = 0;


/* TODO:
	- METADATA HANDLING
	- IP SETTINGS
	- VS1053 - DELAY USING vTaskDelay
*/
struct icyHeader header = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL};

char metaint[10];
char clientURL[256]= {0,0};
char clientPath[256] = {0,0};
uint16_t clientPort = 80;

struct hostent *server = NULL;

///////////////
#define BUFFER_SIZE 12960
//#define BUFFER_SIZE 9600
//#define BUFFER_SIZE 8000
uint8_t buffer[BUFFER_SIZE];
uint16_t wptr = 0;
uint16_t rptr = 0;
uint8_t bempty = 1;

void *incmalloc(size_t n)
{
	void* ret;
//printf ("Client malloc of %d,  Heap size: %d\n",n,xPortGetFreeHeapSize( ));
	ret = malloc(n);
		if (ret == NULL) printf("Client: incmalloc fails for %d\n",n);
//	if (n <4) printf("Client: incmalloc size:%d\n",n);	
//	printf ("Client malloc after of %d bytes ret:%x  Heap size: %d\n",n,ret,xPortGetFreeHeapSize( ));
	return ret;
}	
void incfree(void *p,char* from)
{
	free(p);
//	printf ("Client incfree of %x, from %s           Heap size: %d\n",p,from,xPortGetFreeHeapSize( ));
}	

ICACHE_FLASH_ATTR uint16_t getBufferFree() {
	if(wptr > rptr ) return BUFFER_SIZE - wptr + rptr;
	else if(wptr < rptr) return rptr - wptr;
	else if(bempty) return BUFFER_SIZE; else return 0;
}

ICACHE_FLASH_ATTR uint16_t getBufferFilled() {
	return BUFFER_SIZE - getBufferFree();
}

ICACHE_FLASH_ATTR uint16_t bufferWrite(uint8_t *data, uint16_t size) {
//	uint16_t s = size;
	uint16_t i = 0;
	for(i=0; i<size; i++) {
		if(getBufferFree() == 0) { return i;}
		buffer[wptr++] = data[i];
		if(bempty) bempty = 0;
//		wptr++;
		if(wptr == BUFFER_SIZE) wptr = 0;
	}
	return size;
}

ICACHE_FLASH_ATTR uint16_t bufferRead(uint8_t *data, uint16_t size) {
	uint16_t s = size;
	size = (size>>6)<<6; //mod 32
	uint16_t i = 0;
	uint16_t bf = BUFFER_SIZE - getBufferFree();
	if(s > bf) s = bf;
	for (i = 0; i < s; i++) {
		if(bf == 0) { return i;}
		data[i] = buffer[rptr++];
//		rptr++;
		if(rptr == BUFFER_SIZE) rptr = 0;
		if(rptr == wptr) bempty = 1;
	}
	return s;
}

ICACHE_FLASH_ATTR void bufferReset() {
	playing = 0;	
	wptr = 0;
	rptr = 0;
	bempty = 1;
}

///////////////


ICACHE_FLASH_ATTR void clientInit() {
	vSemaphoreCreateBinary(sHeader);
	vSemaphoreCreateBinary(sConnect);
	vSemaphoreCreateBinary(sConnected);
	vSemaphoreCreateBinary(sDisconnect);
	xSemaphoreTake(sConnect, portMAX_DELAY);
	xSemaphoreTake(sConnected, portMAX_DELAY);
	xSemaphoreTake(sDisconnect, portMAX_DELAY);
}

ICACHE_FLASH_ATTR uint8_t clientIsConnected() {
	if(xSemaphoreTake(sConnected, 0)) {
		xSemaphoreGive(sConnected);
		return 0;
	}
	return 1;
}

ICACHE_FLASH_ATTR struct icyHeader* clientGetHeader()
{	
	return &header;
}

	
ICACHE_FLASH_ATTR bool clientParsePlaylist(char* s)
{
  char* str; 
  char path[80] = "/";
  char url[80]; 
  char port[5] = "80";
  int remove;
  int i = 0; int j = 0;
  str = strstr(s,"<location>http://");  //for xspf
  if (str != NULL) remove = 17;
  if (str ==NULL) 
  {	  
	str = strstr(s,"http://");
	if (str != NULL) remove = 7;
  }
  if (str != NULL)
  {
	str += remove; //skip http://
	while ((str[i] != '/')&&(str[i] != ':')&&(str[i] != 0x0a)&&(str[i] != 0x0d)) {url[j] = str[i]; i++ ;j++;}
	url[j] = 0;
	j = 0;
	if (str[i] == ':')  //port
	{
		i++;
		while ((str[i] != '/')&&(str[i] != 0x0a)&&(str[i] != 0x0d)) {port[j] = str[i]; i++ ;j++;}
	}
	j = 0;
	if ((str[i] != 0x0a)&&(str[i] != 0x0d)&&(str[i] != 0)&&(str[i] != '"')&&(str[i] != '<'))
	{	
	  while ((str[i] != 0x0a)&&(str[i] != 0x0d)&&(str[i] != 0)&&(str[i] != '"')&&(str[i] != '<')) {path[j] = str[i]; i++; j++;}
	  path[j] = 0;
	}
	if (strncmp(url,"localhost",9)!=0) clientSetURL(url);
	clientSetPath(path);
	clientSetPort(atoi(port));
//	printf("##CLI.URL#: %s, path: %s, port: %s\n",url,path,port);
	return true;
  }
  else 
  { 
   cstatus = C_DATA;
   return false;
  }
}
ICACHE_FLASH_ATTR char* stringify(char* str,int len)
{
//		if ((strchr(str,'"') == NULL)&&(strchr(str,'/') == NULL)) return str;
		char* new = incmalloc(len+10);
		if (new != NULL)
		{
//			printf("stringify: enter: len:%d  \"%s\"\n",len,str);
			int i=0 ,j =0;
			for (i = 0;i< len+10;i++) new[i] = 0;
			for (i=0;i< len;i++)
			{
				if (str[i] == '"') {
					new[j++] = '\\';
					new[j++] =(str)[i] ;
				} else
				if (str[i] == '/') {
					new[j++] = '\\';
					new[j++] =(str)[i] ;
				}else	// pseudo ansi utf8 convertion
					if ((str[i] > 192) && (str[i+1] < 128)){
					new[j++] = 195;
					new[j++] =(str)[i]-64 ;
				} else new[j++] =(str)[i] ;
			}
			incfree(str,"str");
			new = realloc(new,j+1);
//			printf("stringify: exit: len:%d  \"%s\"\n",j,new);
			return new;		
		} else 
		{
			printf("stringify malloc fails\n");
		}	
		return str;
}

ICACHE_FLASH_ATTR void clientSaveMetadata(char* s,int len,bool catenate)
{
	    int oldlen = 0;
		char* t_end = NULL;
		char* t_quote;
		char* t ;
		bool found = false;
		if (catenate) printf("Entry meta len=%d catenate=%d  s= %s\n",len,catenate,s);
		if (catenate) oldlen = strlen(header.members.mArr[METADATA]);
		t = s;
		t_end = strstr(t,";StreamUrl='");
		if (t_end != NULL) { *t_end = 0;found = true;} 
		t = strstr(t,"StreamTitle='");
		if (t!= NULL) {t += 13;found = true;} else t = s;
		len = strlen(t);
//		printf("Len= %d t= %s\n",len,t);
		if ((t_end != NULL)&&(len >=3)) t_end -= 3;
		else if (len >3) t_end = t+len-3; else t_end = t;
		if (found)
		{	
			t_quote = strstr(t_end,"'");
			if (t_quote !=NULL){ t_end = t_quote; *t_end = 0;}
		} else {t = "";len = 0;}
//		printf("clientsaveMeta t= 0x%x t_end= 0x%x  t=%s\n",t,t_end,t);
		
//		s = t;
		if ((header.members.mArr[METADATA] != NULL)&&(catenate))
		{
			header.members.mArr[METADATA] = realloc(header.members.mArr[METADATA],(oldlen  +len+3)*sizeof(char));
			printf("clientsaveMeta  s=\"%s\"  t=\"%s\"   meta=\"%s\"\n",s,t,header.members.mArr[METADATA]);
		} else
		{
			if (header.members.mArr[METADATA] != NULL)
				incfree(header.members.mArr[METADATA],"metad");
			header.members.mArr[METADATA] = (char*)incmalloc((len+3)*sizeof(char));
		}
		if(header.members.mArr[METADATA] == NULL) 
			{printf("clientsaveMeta malloc fails\n"); return;}

		header.members.mArr[METADATA][oldlen +len] = 0;
		strncpy(&(header.members.mArr[METADATA][oldlen]), t,len);
		header.members.mArr[METADATA] = stringify(header.members.mArr[METADATA],oldlen +len);
		if (catenate)	printf("clientsaveMeta t=\"%s\"   meta=\"%s\"\n",t,header.members.mArr[METADATA]);
		printf("##CLI.META#: %s\n",header.members.mArr[METADATA]);
		char* title = incmalloc(strlen(header.members.mArr[METADATA])+15);
		if (title != NULL)
		{
			sprintf(title,"{\"meta\":\"%s\"}",header.members.mArr[METADATA]); 
			websocketbroadcast(title, strlen(title));
			incfree(title,"title");
		} else printf("clientsaveMeta malloc title fails\n"); 
}	

// websocket: next station
ICACHE_FLASH_ATTR void wsStationNext()
{
	char answer[22];
	struct shoutcast_info* si =NULL;
	if (currentStation <191)
		si = getStation(++currentStation);
	else 
	{
		currentStation = 0;
		si = getStation(currentStation);
	}		
	if(si != NULL && (strcmp(si->domain,"")!=0) && (strcmp( si->file,"")!= 0))
	{
		sprintf(answer,"%d",currentStation);
		playStation	(answer);
	} else currentStation--;
	incfree(si,"wsstation");
	sprintf(answer,"{\"wsstation\":\"%d\"}",currentStation);
	websocketbroadcast(answer, strlen(answer));
}
// websocket: previous station
ICACHE_FLASH_ATTR void wsStationPrev()
{
	char answer[22];
	struct shoutcast_info* si = NULL;
	if (currentStation >0)
		si = getStation(--currentStation);
	else return;
	if(si != NULL && (strcmp(si->domain,"")!=0) && (strcmp( si->file,"")!= 0))
	{
		sprintf(answer,"%d",currentStation);
		playStation	(answer);
	} else currentStation++;
	incfree(si,"wsstation");
	sprintf(answer,"{\"wsstation\":\"%d\"}",currentStation);
	websocketbroadcast(answer, strlen(answer));
}

// websocket: broadcast volume to all client
ICACHE_FLASH_ATTR void wsVol(char* vol)
{
	char answer[21];
	if (vol != NULL)
	{	
		sprintf(answer,"{\"wsvol\":\"%s\"}",vol);
		websocketbroadcast(answer, strlen(answer));
	} 
}	
// websocket: broadcast monitor url
ICACHE_FLASH_ATTR void wsMonitor()
{
		char answer[300];
		memset(&answer,0,300);
		if ((clientPath[0]!= 0))
		{
			sprintf(answer,"{\"monitor\":\"http://%s:%d%s\"}",clientURL,clientPort,clientPath);
			websocketbroadcast(answer, strlen(answer));
		}
}						
//websocket: broadcast all icy and meta info to web client.
ICACHE_FLASH_ATTR void wsHeaders()
{
	uint8_t header_num;
	char* not2;
	not2 = header.members.single.notice2;
	if (not2 ==NULL) not2=header.members.single.audioinfo;
	if ((header.members.single.notice2 != NULL)&(strlen(header.members.single.notice2)==0)) not2=header.members.single.audioinfo;

	int json_length ;
	json_length =93+
		((header.members.single.description ==NULL)?0:strlen(header.members.single.description)) +
		((header.members.single.name ==NULL)?0:strlen(header.members.single.name)) +
		((header.members.single.bitrate ==NULL)?0:strlen(header.members.single.bitrate)) +
		((header.members.single.url ==NULL)?0:strlen(header.members.single.url))+ 
		((header.members.single.notice1 ==NULL)?0:strlen(header.members.single.notice1))+
		((not2 ==NULL)?0:strlen(not2))+
		((header.members.single.genre ==NULL)?0:strlen(header.members.single.genre))+
		((header.members.single.metadata ==NULL)?0:strlen(header.members.single.metadata))
		;
	char* wsh = incmalloc(json_length+1);
	if (wsh == NULL) {printf("wsHeader malloc fails\n");return;}

	sprintf(wsh,"{\"wsicy\":{\"descr\":\"%s\",\"meta\":\"%s\",\"name\":\"%s\",\"bitr\":\"%s\",\"url1\":\"%s\",\"not1\":\"%s\",\"not2\":\"%s\",\"genre\":\"%s\"}}",
			(header.members.single.description ==NULL)?"":header.members.single.description,
			(header.members.single.metadata ==NULL)?"":header.members.single.metadata,	
			(header.members.single.name ==NULL)?"":header.members.single.name,
			(header.members.single.bitrate ==NULL)?"":header.members.single.bitrate,
			(header.members.single.url ==NULL)?"":header.members.single.url,
			(header.members.single.notice1 ==NULL)?"":header.members.single.notice1,
			(not2 ==NULL)?"":not2 ,
			(header.members.single.genre ==NULL)?"":header.members.single.genre); 
//	printf("WSH: len:%d  \"%s\"\n",strlen(wsh),wsh);
	websocketbroadcast(wsh, strlen(wsh));	
	incfree (wsh,"wsh");
}	

ICACHE_FLASH_ATTR void clearHeaders()
{
	uint8_t header_num;
	for(header_num=0; header_num<ICY_HEADER_COUNT; header_num++) {
		if(header_num != METAINT) if(header.members.mArr[header_num] != NULL) {
			header.members.mArr[header_num][0] = 0;				
		}
	}
	header.members.mArr[METAINT] = 0;
	wsHeaders();
}
	
ICACHE_FLASH_ATTR bool clientSaveOneHeader(char* t, uint16_t len, uint8_t header_num)
{
	if(header.members.mArr[header_num] != NULL) 
		incfree(header.members.mArr[header_num],"headernum");
	header.members.mArr[header_num] = incmalloc((len+1)*sizeof(char));
	if(header.members.mArr[header_num] == NULL)
	{
		printf("clientSaveOneHeader malloc fails\n");
		return false;
	}	
	int i;
	for(i = 0; i<len+1; i++) header.members.mArr[header_num][i] = 0;
	strncpy(header.members.mArr[header_num], t, len);
	printf("##CLI.ICY%d#: %s\n",header_num,header.members.mArr[header_num]);
	header.members.mArr[header_num] = stringify(header.members.mArr[header_num],len);
//	printf("header after num:%d addr:0x%x  cont:\"%s\"\n",header_num,header.members.mArr[header_num],header.members.mArr[header_num]);
	return true;
}

	
ICACHE_FLASH_ATTR bool clientParseHeader(char* s)
{
	// icy-notice1 icy-notice2 icy-name icy-genre icy-url icy-br
	uint8_t header_num;
	bool ret = false;
//	printf("ParseHeader: %s\n",s);
	xSemaphoreTake(sHeader,portMAX_DELAY);
	if ((cstatus != C_HEADER1)&& (cstatus != C_PLAYLIST))// not ended. dont clear
	{
		clearHeaders();
	}
	for(header_num=0; header_num<ICY_HEADERS_COUNT; header_num++)
	{
//				printf("icy deb: %d\n",header_num);		
		char *t;
		t = strstr(s, icyHeaders[header_num]);
		if( t != NULL )
		{
			t += strlen(icyHeaders[header_num]);
			char *t_end = strstr(t, "\r\n");
			if(t_end != NULL)
			{
//				printf("icy in: %d\n",header_num);		
				uint16_t len = t_end - t;
				if(header_num != METAINT) // Text header field
				{
					ret = clientSaveOneHeader(t, len, header_num);
				}
				else // Numerical header field
				{					
						int i;
						for(i = 0; i<len+1; i++) metaint[i] = 0;
						strncpy(metaint, t, len);
						header.members.single.metaint = atoi(metaint);
//						printf("MetaInt= %s, Metaint= %d\n",metaint,header.members.single.metaint);
						ret = true;
//						printf("icy: %s: %d\n",icyHeaders[header_num],header.members.single.metaint);					
				}
			}
		}
	}
	if (ret == true) {wsHeaders();wsMonitor();}
	xSemaphoreGive(sHeader);
		return ret;
}

ICACHE_FLASH_ATTR void clientSetURL(char* url)
{
	int l = strlen(url)+1;
	if (url[0] == 0xff) return; // wrong url

	strcpy(clientURL, url);
	printf("##CLI.URLSET#: %s\n",clientURL);
}

ICACHE_FLASH_ATTR void clientSetPath(char* path)
{
	int l = strlen(path)+1;
	if (path[0] == 0xff) return; // wrong path
	strcpy(clientPath, path);
	printf("##CLI.PATHSET#: %s\n",clientPath);
}

ICACHE_FLASH_ATTR void clientSetPort(uint16_t port)
{
	clientPort = port;
	printf("##CLI.PORTSET#: %d\n",port);
}

ICACHE_FLASH_ATTR void clientConnect()
{
	cstatus = C_HEADER;
	if(server) incfree(server,"server");
	if((server = (struct hostent*)gethostbyname(clientURL))) {
		xSemaphoreGive(sConnect);
	} else {
		clientDisconnect();
	}
}

ICACHE_FLASH_ATTR void clientDisconnect()
{
	//connect = 0;
	xSemaphoreGive(sDisconnect);
	printf("##CLI.STOPPED#\n");
	clearHeaders();
}

IRAM_ATTR void clientReceiveCallback(int sockfd, char *pdata, int len)
{
	/* TODO:
		- What if header is in more than 1 data part? // ok now ...
		- Metadata processing // ok
		- Buffer underflow handling (?) ?
	*/
	static int metad ;
	static int rest ;
	static uint32_t chunked;
	static uint32_t cchunk;
	uint16_t l ;
	uint32_t lc;
	char* t1;
	char* t2;
	bool  icyfound;

//	if (cstatus != C_DATA){printf("cstatus= %d\n",cstatus);  printf("Len=%d, Byte_list = %s\n",len,pdata);}
	switch (cstatus)
	{
	case C_PLAYLIST:
 
        if (!clientParsePlaylist(pdata)) //need more
		  cstatus = C_PLAYLIST1;
		else clientDisconnect();  
    break;
	case C_PLAYLIST1:
       clientDisconnect();		  
        clientParsePlaylist(pdata) ;//more?
		cstatus = C_PLAYLIST;
	break;
	case C_HEADER:
		clearHeaders();
		metad = -1;
		t1 = strstr(pdata, "302 "); 
		if (t1 ==NULL) t1 = strstr(pdata, "301 "); 
		if (t1 != NULL) { // moved to a new address
			if( strcmp(t1,"Found")||strcmp(t1,"Temporarily")||strcmp(t1,"Moved"))
			{
				printf("Header: Moved\n");
				clientDisconnect();
				clientParsePlaylist(pdata);
				cstatus = C_PLAYLIST;				
			}	
			break;
		}
		//no break here
	case C_HEADER1:  // not ended
		{
			cstatus = C_HEADER1;
			do {
				t1 = strstr(pdata, "\r\n\r\n"); // END OF HEADER
//				printf("Header len: %d,  Header: %s\n",len,pdata);
				if ((t1 != NULL) && (t1 <= pdata+len-4)) 
				{
						icyfound = clientParseHeader(pdata);
						wsMonitor();											
						if(header.members.single.bitrate != NULL) 
/*							if (strcmp(header.members.single.bitrate,"320")==0)
								 system_update_cpu_freq(SYS_CPU_160MHZ);
							else system_update_cpu_freq(SYS_CPU_80MHZ);*/
						if(header.members.single.metaint > 0) 
							metad = header.members.single.metaint;
//						printf("t1: 0x%x, cstatus: %d, icyfound: %d  metad:%d Metaint:%d\n", t1,cstatus, icyfound,metad, header.members.single.metaint); 
						cstatus = C_DATA;	// a stream found
						VS1053_flush_cancel(1);
						t2 = strstr(pdata, "Transfer-Encoding: chunked"); // chunked stream? 
//						t2 = NULL;
						if ( t2 != NULL) 
						{
							t1+= 4; 
							chunked = (uint32_t) strtol(t1, NULL, 16) +2;
							*strchr(t1,0x0A) = 0;
//							printf("strlen: %d  \"%s\"\n",strlen(t1)+1,t1);
							t1 +=strlen(t1)+1; //+1 for char 0, +2 for cr lf at end of chunk
						}
						else {chunked = 0; t1 += 4;}
						
						int newlen = len - (t1-pdata) ;
						cchunk = chunked;
//						printf("newlen: %d   len: %d  t1:%x  t2:%  chunked:%d  pdata:%x \n",newlen,len,t1,t2,chunked,pdata);
						if(newlen > 0) clientReceiveCallback(sockfd,t1, newlen);
				} else
				{
					t1 = NULL;
					len += recv(sockfd, pdata+len, RECEIVE-len, 0);
				}
			} while (t1 == NULL);
		}
	break;
	default:		
// -----------	
//		rest = 0;
		lc = 0;
		if((chunked != 0)&&((cchunk ==0)||(len >= cchunk-1))) {
			if (len == cchunk)
			{ 
				len -= 2;
				cchunk = 0;
//				printf("lenoe:%d, chunked:%d  cchunk:%d, lc:%d, metad:%d\n",len,chunked,cchunk, lc,metad );
			} else
			{	
				if (len == cchunk-1)
				{ 
					len -= 1;
					cchunk = 1;
//					printf("leno1:%d, chunked:%d  cchunk:%d, lc:%d, metad:%d\n",len,chunked,cchunk, lc,metad );
				} else			{
					uint32_t clen;
					while (len < cchunk+10) len += recv(sockfd, pdata+len, RECEIVE+10-len, 0); //security
					*strchr((pdata+cchunk),0x0A) = 0;
//					printf("leni:%d, cchunk:%d, lc:%d,  str: %s\n",len,cchunk, lc,pdata+cchunk ); //chunk end with cr lf. skip it
					chunked = (uint32_t) strtol(pdata+cchunk, NULL, 16)+2;
					clen = strlen(pdata+cchunk)  +1;
					lc = len -cchunk  -clen; // rest after
//					printf("leno:%d, chunked:%d  cchunk:%d, lc:%d, metad:%d  str: %s\n",len,chunked,cchunk, lc,metad,pdata+cchunk );
					if (cchunk >1)
						memcpy (pdata+cchunk-2,pdata+len-lc, lc); 
					else
						memcpy (pdata,pdata+len-lc, lc); 
					len -=  clen;
					if (cchunk >1) len -= 2; else len -=cchunk;
					cchunk = chunked - lc ;
//					printf("lenf:%d, chunked:%d  cchunk:%d, lc:%d, metad:%d\n",len,chunked,cchunk, lc,metad );
				}
			}
		} 
		else if (chunked != 0) cchunk -= len;

		if((header.members.single.metaint != 0)&&(len > metad)) {
			l = pdata[metad]*16;
			rest = len - metad  -l -1;
			if (l !=0)
			{
//				printf("len:%d, metad:%d, l:%d, rest:%d, str: %s\n",len,metad, l,rest,pdata+metad+1 );
				if (rest <0) *(pdata+len) = 0; //truncated
				clientSaveMetadata(pdata+metad+1,l,false);
			}				
			while(getBufferFree() < metad){ vTaskDelay(1); /*printf(":");*/}
				bufferWrite(pdata, metad); 
			metad = header.members.single.metaint - rest ; //until next
			if (rest >0)
			{	
				while(getBufferFree() < rest) {vTaskDelay(1); /*printf(".");*/}
					bufferWrite(pdata+len-rest, rest); 
				rest = 0;
			} 	
		} else 
		{	
	        if (rest <0) 
			{
//				printf("Negative len= %d, metad= %d  rest= %d   pdata= \"%s\"\n",len,metad,rest,pdata);
				clientSaveMetadata(pdata,0-rest,true);
				/*buf =pdata+rest;*/ len +=rest;metad += rest; rest = 0;
			}	
			if (header.members.single.metaint != 0) metad -= len;
//			printf("len = %d, metad = %d  metaint= %d  cchunk= %d\n",len,metad,header.members.single.metaint,cchunk);
			while(getBufferFree() < len) {vTaskDelay(1); /*printf("-");*/}
			if (len >0) bufferWrite(pdata+rest, len);	
		}
// ---------------			
		if(!playing && (getBufferFree() < BUFFER_SIZE/3)) {
			playing=1;
			printf("##CLI.PLAYING#\n");
		}	
    }
}

#define VSTASKBUF	1000
IRAM_ATTR void vsTask(void *pvParams) { 
	uint8_t b[VSTASKBUF];
//	portBASE_TYPE uxHighWaterMark;
	struct device_settings *device;
	register uint16_t size ,s;
	Delay(100);
	VS1053_Start();
	device = getDeviceSettings();
	Delay(300);

	VS1053_SetVolume( device->vol);	
	VS1053_SetTreble(device->treble);
	VS1053_SetBass(device->bass);
	VS1053_SetTrebleFreq(device->freqtreble);
	VS1053_SetBassFreq(device->freqbass);
	VS1053_SetSpatial(device->spacial);
	incfree(device,"device");	
	VS1053_SPI_SpeedUp();
	while(1) {
		if(playing) {
			s = 0; 
			size = bufferRead(b, VSTASKBUF);		
			while(s < size) 
			{
				s += VS1053_SendMusicBytes(b+s, size-s);
			}
			
			vTaskDelay(1);
		} else 
		{
//			VS1053_SPI_SpeedDown();
			vTaskDelay(20);
		
//	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
//	printf("watermark vstask: %x  %d\n",uxHighWaterMark,uxHighWaterMark);			
		}	
	}
}

ICACHE_FLASH_ATTR void clientTask(void *pvParams) {
//1440	for MTU 
	int sockfd, bytes_read;
	struct sockaddr_in dest;
	uint8_t bufrec[RECEIVE+10];
//	portBASE_TYPE uxHighWaterMark;
//	clearHeaders();
/*	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	printf("watermark: %x  %d\n",uxHighWaterMark,uxHighWaterMark);
*/	
	while(1) {
		xSemaphoreGive(sConnected);

		if(xSemaphoreTake(sConnect, portMAX_DELAY)) {

			xSemaphoreTake(sDisconnect, 0);

			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if(sockfd >= 0) ; //printf("WebClient Socket created\n");
			else printf("WebClient Socket creation failed\n");
			bzero(&dest, sizeof(dest));
			dest.sin_family = AF_INET;
			dest.sin_port = htons(clientPort);
			dest.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)(server -> h_addr_list[0])));

			/*---Connect to server---*/
			if(connect(sockfd, (struct sockaddr*)&dest, sizeof(dest)) >= 0) 
			{
//				printf("WebClient Socket connected\n");
				memset(bufrec,0, RECEIVE);
				
				char *t0 = strstr(clientPath, ".m3u");
				if (t0 == NULL)  t0 = strstr(clientPath, ".pls");
				if (t0 == NULL)  t0 = strstr(clientPath, ".xspf");				
				if (t0 != NULL)  // a playlist asked
				{
				  cstatus = C_PLAYLIST;
				  sprintf(bufrec, "GET %s HTTP/1.0\r\nHOST: %s\r\n\r\n", clientPath,clientURL); //ask for the playlist
			    } 
				else sprintf(bufrec, "GET %s HTTP/1.1\r\nHost: %s\r\nicy-metadata: 1\r\n\r\n", clientPath,clientURL); 
//				printf("st:%d, Client Sent:\n%s\n",cstatus,bufrec);
				send(sockfd, bufrec, strlen(bufrec), 0);
				
				xSemaphoreTake(sConnected, 0);

				do
				{
					bytes_read = recv(sockfd, bufrec, RECEIVE, 0);
					if (playing)
					{
						if (RECEIVE-bytes_read ) bytes_read += recv(sockfd, bufrec+bytes_read, RECEIVE-bytes_read, 0); //boost
					}
//					printf("s:%d   ", bytes_read);
					if ( bytes_read > 0 )
						clientReceiveCallback(sockfd,bufrec, bytes_read);
					if(xSemaphoreTake(sDisconnect, 0)) break;	
				}
				while ( bytes_read > 0 );
			} else printf("WebClient Socket fails to connect %d\n", errno);
			/*---Clean up---*/
			if (bytes_read == 0 ) 
			{
					
					if (playing) 
					{
						clientDisconnect(); 
						clientConnect();
					}	
					else{
						clientDisconnect(); 
						clientSaveOneHeader("Not Found", 9,METANAME);
					}	
					
			}//jpc
			bufferReset();
/*
			uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
			printf("watermark: %x  %d\n",uxHighWaterMark,uxHighWaterMark);
*/
			if (playing)
			{
				VS1053_flush_cancel(2);
				playing = 0;
				VS1053_flush_cancel(0);
			}	
			shutdown(sockfd,SHUT_RDWR);
			vTaskDelay(10);	
			close(sockfd);
//			printf("WebClient Socket closed\n");
			if (cstatus == C_PLAYLIST) 			
			{
			  clientConnect();
			}
		}
	}
}
