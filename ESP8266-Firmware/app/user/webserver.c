/*
 * Copyright 2016 karawin (http://www.karawin.fr)
*/
#include "webserver.h"
#include "serv-fs.h"

char lowmemory[] = { "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 10\r\n\r\n\", \"low memory\")"};
xSemaphoreHandle semclient = NULL ;

LOCAL os_timer_t sleepTimer;
uint32_t sleepDelay;
LOCAL os_timer_t wakeTimer;
uint32_t wakeDelay;
int8_t clientOvol = 0;
uint8_t clientIvol = 0;

void *inmalloc(size_t n)
{
	void* ret;
//	printf ("server Malloc of %d,  Heap size: %d\n",n,xPortGetFreeHeapSize( ));
	ret = malloc(n);
	if (ret == NULL) printf("Server: Malloc fails for %d\n",n);
//	printf ("server Malloc after of %d bytes ret:%x  Heap size: %d\n",n,ret,xPortGetFreeHeapSize( ));
//	if (n <4) printf("Server: incmalloc size:%d\n",n);	
	return ret;
}	
void infree(void *p)
{
	if (p != NULL)free(p);
//	printf ("server free of %x,                      Heap size: %d\n",p,xPortGetFreeHeapSize( ));
}	



ICACHE_FLASH_ATTR struct servFile* findFile(char* name)
{
	struct servFile* f = (struct servFile*)&indexFile;
	while(1)
	{
		if(strcmp(f->name, name) == 0) return f;
		else f = f->next;
		if(f == NULL) return NULL;
	}
}

ICACHE_FLASH_ATTR void serveFile(char* name, int conn)
{
#define PART 2048
	int length;
	int progress,part,gpart;
	char buf[140];
	char *content;
	if (strcmp(name,"/style.css") == 0)
	{
		struct device_settings *device;
		device = getDeviceSettings();
		if (device != NULL)	 {
			if (device->options & T_THEME) strcpy(name , "/style1.css");
//			printf("name: %s, theme:%d\n",name,device->options&T_THEME);
			infree(device);
		}
	}
	struct servFile* f = findFile(name);
//	printf("find %s at %x\n",name,f);
//	printf ("Heap size: %d\n",xPortGetFreeHeapSize( ));
	gpart = PART;
	if(f != NULL)
	{
		length = f->size;
		content = f->content;
		progress = 0;
	}
	else length = 0;
	if(length > 0)
	{
		char *con = NULL;
		do {
			gpart /=2;
			con = (char*)inmalloc((gpart)*sizeof(char));
		} while ((con == NULL)&&(gpart >=32));
		
		if ((con == NULL)||(gpart <32))
		{
				sprintf(buf, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", (f!=NULL ? f->type : "text/plain"), 0);
				write(conn, buf, strlen(buf));
				printf("serveFile inmalloc error\n");
				incfree(con);
				return ;
		}	
//		printf("serveFile socket:%d,  %s. Length: %d  sliced in %d\n",conn,name,length,gpart);		
		sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: keep-alive\r\n\r\n", (f!=NULL ? f->type : "text/plain"), length);
//		sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", (f!=NULL ? f->type : "text/plain"), length);
		write(conn, buf, strlen(buf));

		progress = length;
		part = gpart;
		if (progress <= part) part = progress;

		while (progress > 0) {
			flashRead(con, (uint32_t)content, part);
			write(conn, con, part);
			content += part;
			progress -= part;
			if (progress <= part) part = progress;
			vTaskDelay(1);
		} 

		infree(con);
	}
	else
	{
		sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", (f!=NULL ? f->type : "text/plain"), 0);
		write(conn, buf, strlen(buf));
	}
}

ICACHE_FLASH_ATTR char* getParameter(char* sep,char* param, char* data, uint16_t data_length) {
	char* p = strstr(data, param);
	if(p != NULL) {
		p += strlen(param);
		char* p_end = strstr(p, sep);
		if(p_end ==NULL) p_end = data_length + data;
		if(p_end != NULL ) {
			if (p_end==p) return NULL;
			char* t = inmalloc(p_end-p + 1);
			if (t == NULL) { printf("getParameterF inmalloc fails\n"); return NULL;}
			int i;
			for(i=0; i<(p_end-p + 1); i++) t[i] = 0;
			strncpy(t, p, p_end-p);
//printf("getParam: in: \"%s\"   \"%s\"\n",data,t);
			return t;
		} else return NULL;
	} else return NULL;
}
ICACHE_FLASH_ATTR char* getParameterFromResponse(char* param, char* data, uint16_t data_length) {
	return getParameter("&",param,data, data_length) ;
}
ICACHE_FLASH_ATTR char* getParameterFromComment(char* param, char* data, uint16_t data_length) {
	return getParameter("\"",param,data, data_length) ;
}

ICACHE_FLASH_ATTR void respOk(int conn)
{
		char resp[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\n\r\nOK";
		write(conn, resp, strlen(resp));
}

ICACHE_FLASH_ATTR void clientSetOvol(int8_t ovol)
{
	clientOvol = ovol;
	printf("##CLI.OVOLSET#: %d\n",ovol);
	vTaskDelay(20);
}

// set the current volume with its offset
ICACHE_FLASH_ATTR void setOffsetVolume(void) {
		struct device_settings *device;
		device = getDeviceSettings();
		int16_t uvol;
		printf("##CLI.VOL#: %d\n",device->vol);
		uvol = device->vol+clientOvol;
		if (uvol > 254) uvol = 254;
		if (uvol <=0) uvol = 1;
//			printf("setOffsetVol: %d\n",clientOvol);
			VS1053_SetVolume(uvol);
		infree(device);			
}

// set the volume with vol, save it to device and add offset
ICACHE_FLASH_ATTR void setVolume(char* vol) {
		struct device_settings *device;
//		uint16_t ivol = atoi(vol);
		clientIvol = atoi(vol);
		uint16_t uvol = atoi(vol);
		uvol += clientOvol;
		if (uvol > 254) uvol = 254;
		if (uvol <0) uvol = 0;
		if(vol) {
//			printf("setVol: \"%s + %d, uvol: %d, clientIvol:%d\"\n",vol,clientOvol,uvol,clientIvol);
//			device = getDeviceSettings();
			VS1053_SetVolume(uvol);
			printf("##CLI.VOL#: %d\n",clientIvol);
//			if (device != NULL)
//				if (device->vol != clientIvol){ device->vol = clientIvol;saveDeviceSettings(device);}
//			infree(device);			
		}
}

ICACHE_FLASH_ATTR uint16_t getVolume() {
	return (VS1053_GetVolume()-clientOvol);
}

// Set the volume with increment vol
ICACHE_FLASH_ATTR void setRelVolume(int8_t vol) {
	struct device_settings *device;
	char Vol[5];
	int16_t rvol;
	device = getDeviceSettings();
	rvol = device->vol+vol;
	if (rvol <0) rvol = 0;
	if (rvol > 254) rvol = 254;
	sprintf(Vol,"%d",rvol);
	infree(device);		
	setVolume(Vol);
	wsVol(Vol);
}

// flip flop the theme indicator
ICACHE_FLASH_ATTR void theme() {
		struct device_settings *device;
		device = getDeviceSettings();
		if (device != NULL)	 {
			if ((device->options&T_THEME)!=0) device->options&=NT_THEME; else device->options |= T_THEME;
			saveDeviceSettings(device);
//			printf("theme:%d\n",device->options&T_THEME);
			infree(device);	
		}
}

ICACHE_FLASH_ATTR void sleepCallback(void *pArg) {
	if (--sleepDelay == 0)
	{
		os_timer_disarm(&sleepTimer);
		clientSilentDisconnect(); // stop the player
	}		
}
ICACHE_FLASH_ATTR void wakeCallback(void *pArg) {
	if (--wakeDelay == 0)
	{
		os_timer_disarm(&wakeTimer);
		clientSilentDisconnect();
		clientSilentConnect(); // start the player
	}		
}

ICACHE_FLASH_ATTR void startSleep(uint32_t delay)
{
//	printf("Delay:%d\n",delay);
	if (delay == 0) return;
	sleepDelay = delay*60; // minutes to seconds
	os_timer_disarm(&sleepTimer);
	os_timer_arm(&sleepTimer, 1000, true); // 1 second and rearm	
}
ICACHE_FLASH_ATTR void stopSleep(){
//	printf("stopDelayDelay\n");
	os_timer_disarm(&sleepTimer);
}
ICACHE_FLASH_ATTR void startWake(uint32_t delay)
{
//	printf("Wake Delay:%d\n",delay);
	if (delay == 0) return;
	wakeDelay = delay*60; // minutes to seconds
	os_timer_disarm(&wakeTimer);
	os_timer_arm(&wakeTimer, 1000, true); // 1 second and rearm	
}
ICACHE_FLASH_ATTR void stopWake(){
//	printf("stopDelayWake\n");
	os_timer_disarm(&wakeTimer);
}

// treat the received message of the websocket
void websockethandle(int socket, wsopcode_t opcode, uint8_t * payload, size_t length)
{
	struct device_settings *device;
	//wsvol
//	printf("websocketHandle: %s\n",payload);
	if (strstr(payload,"wsvol=")!= NULL)
	{
		char answer[17];
		if (strstr(payload,"&") != NULL)
			*strstr(payload,"&")=0;
		else return;
		setVolume(payload+6);
		sprintf(answer,"{\"wsvol\":\"%s\"}",payload+6);
		websocketlimitedbroadcast(socket,answer, strlen(answer));
	}
	else if (strstr(payload,"startSleep=")!= NULL)
	{
		if (strstr(payload,"&") != NULL)
			*strstr(payload,"&")=0;
		else return;
		startSleep(atoi(payload+11));
	}
	else if (strstr(payload,"stopSleep")!= NULL){stopSleep();}
	else if (strstr(payload,"startWake=")!= NULL)
	{
		if (strstr(payload,"&") != NULL)
			*strstr(payload,"&")=0;
		else return;
		startWake(atoi(payload+10));
	}
	else if (strstr(payload,"stopWake")!= NULL){stopWake();}
	//monitor
	else if (strstr(payload,"monitor")!= NULL){wsMonitor();}
	else if (strstr(payload,"upgrade")!= NULL){update_firmware();}
	else if (strstr(payload,"theme")!= NULL){theme();}
}


ICACHE_FLASH_ATTR void playStationInt(int sid) {
	struct shoutcast_info* si;
	char answer[22];
	struct device_settings *device;
	si = getStation(sid);

		if(si != NULL &&si->domain && si->file) {
			int i;
			vTaskDelay(5);
			clientDisconnect("playStationInt");
			for (i = 0;i<100;i++)
			{
				if(!clientIsConnected())break;
				vTaskDelay(4);
			}
			clientSetName(si->name,sid);
			clientSetURL(si->domain);
			clientSetPath(si->file);
			clientSetPort(si->port);
			clientSetOvol(si->ovol);
			clientConnect();
			setOffsetVolume();
			for (i = 0;i<100;i++)
			{
				if (clientIsConnected()) break;
				vTaskDelay(4);
			}
		}

	infree(si);
	sprintf(answer,"{\"wsstation\":\"%d\"}",sid);
	websocketbroadcast(answer, strlen(answer));
	device = getDeviceSettings();
	if (device->currentstation != sid)
	{
		device->currentstation = sid;
		saveDeviceSettings(device);
	}
	incfree(device,"playStation");

}
	
ICACHE_FLASH_ATTR void playStation(char* id) {
	struct shoutcast_info* si;
	char answer[22];
	int uid;
	struct device_settings *device;
	uid = atoi(id) ;
//	printf ("playstation: %d\n",uid);
	if (uid < 255)
		currentStation = atoi(id) ;
	playStationInt(currentStation);	
}

// replace special  json char
ICACHE_FLASH_ATTR void pathParse(char* str)
{
	int i = 0;
	char *pend;
	char  num[3]= {0,0,0};
	uint8_t cc;
	if (str == NULL) return;
	for (i; i< strlen(str);i++)
	{
		if (str[i] == '%')
		{
			num[0] = str[i+1]; num[1] = str[i+2];
			cc = strtol(num, &pend,16);
			str[i] = cc;			
			str[i+1]=0;
			if (str[i+3] !=0)strcat(str, str+i+3);
		}
	}
}

ICACHE_FLASH_ATTR void handlePOST(char* name, char* data, int data_size, int conn) {
//	printf("HandlePost %s\n",name);
	char* head = NULL;
	int i;
	bool changed = false;
	struct device_settings *device;
	if(strcmp(name, "/instant_play") == 0) {
		if(data_size > 0) {
			char* url = getParameterFromResponse("url=", data, data_size);
			char* path = getParameterFromResponse("path=", data, data_size);
			pathParse(path);
			char* port = getParameterFromResponse("port=", data, data_size);
//			int i;
			if(url != NULL && path != NULL && port != NULL) {
				clientDisconnect("Post instant_play");
				for (i = 0;i<100;i++)
				{
					if(!clientIsConnected())break;
					vTaskDelay(4);
				}
				
				clientSetURL(url);
				clientSetPath(path);
				clientSetPort(atoi(port));
				clientSetOvol(0);
				clientConnectOnce();
				setOffsetVolume();
				for (i = 0;i<100;i++)
				{
					if (clientIsConnected()) break;
					vTaskDelay(5);
				}
			} 
			infree(url);
			infree(path);
			infree(port);
		}
	} else if(strcmp(name, "/soundvol") == 0) {
		if(data_size > 0) {
//			char* vol = getParameterFromResponse("vol=", data, data_size);
			char * vol = data+4;
			data[data_size-1] = 0;
//			printf("/sounvol vol: %s num:%d \n",vol, atoi(vol));
			setVolume(vol); 
		}
	} else if(strcmp(name, "/sound") == 0) {
		if(data_size > 0) {
			char* bass = getParameterFromResponse("bass=", data, data_size);
			char* treble = getParameterFromResponse("treble=", data, data_size);
			char* bassfreq = getParameterFromResponse("bassfreq=", data, data_size);
			char* treblefreq = getParameterFromResponse("treblefreq=", data, data_size);
			char* spacial = getParameterFromResponse("spacial=", data, data_size);
			device = getDeviceSettings();
			changed = false;
			if(bass) {
				VS1053_SetBass(atoi(bass));
				if (device->bass != atoi(bass)){ device->bass = atoi(bass); changed = true;}
				infree(bass);
			}
			if(treble) {
				VS1053_SetTreble(atoi(treble));
				if (device->treble != atoi(treble)){ device->treble = atoi(treble); changed = true;}
				infree(treble);
			}
			if(bassfreq) {
				VS1053_SetBassFreq(atoi(bassfreq));
				if (device->freqbass != atoi(bassfreq)){ device->freqbass = atoi(bassfreq); changed = true;}
				infree(bassfreq);
			}
			if(treblefreq) {
				VS1053_SetTrebleFreq(atoi(treblefreq));
				if (device->freqtreble != atoi(treblefreq)){ device->freqtreble = atoi(treblefreq); changed = true;}
				infree(treblefreq);
			}
			if(spacial) {
				VS1053_SetSpatial(atoi(spacial));
				if (device->spacial != atoi(spacial)){ device->spacial = atoi(spacial); changed = true;}
				infree(spacial);
			}
			if (changed) saveDeviceSettings(device);
			infree(device);
		}
	} else if(strcmp(name, "/getStation") == 0) {
		if(data_size > 0) {
			char* id = getParameterFromResponse("idgp=", data, data_size);
			if (id ) 
			{
				if ((atoi(id) >=0) && (atoi(id) < 255)) 
				{
					char ibuf [6];	
					char *buf;
//					int i;
					for(i = 0; i<sizeof(ibuf); i++) ibuf[i] = 0;
					struct shoutcast_info* si;
					si = getStation(atoi(id));
					if (strlen(si->domain) > sizeof(si->domain)) si->domain[sizeof(si->domain)-1] = 0; //truncate if any (rom crash)
					if (strlen(si->file) > sizeof(si->file)) si->file[sizeof(si->file)-1] = 0; //truncate if any (rom crash)
					if (strlen(si->name) > sizeof(si->name)) si->name[sizeof(si->name)-1] = 0; //truncate if any (rom crash)
					sprintf(ibuf, "%d%d", si->ovol,si->port);
					int json_length = strlen(si->domain) + strlen(si->file) + strlen(si->name) + strlen(ibuf) + 50;
					buf = inmalloc(json_length + 75);
					if (buf == NULL)
					{	
						printf("getStation inmalloc fails\n");
						respOk(conn);
					}
					else {				
						for(i = 0; i<sizeof(buf); i++) buf[i] = 0;
						sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n{\"Name\":\"%s\",\"URL\":\"%s\",\"File\":\"%s\",\"Port\":\"%d\",\"ovol\":\"%d\"}",
						json_length, si->name, si->domain, si->file,si->port,si->ovol);
//				printf("getStation Buf len:%d : %s\n",strlen(buf),buf);						
						write(conn, buf, strlen(buf));
						infree(buf);
					}
					infree(si);
					infree(id);
					return;
				} else printf("getstation, no id or Wrong id %d\n",atoi(id));
				infree (id);
			} 			
		}
	} else if(strcmp(name, "/setStation") == 0) {
		if(data_size > 0) {
//			printf("data:%s\n",data);
			char* nb = getParameterFromResponse("nb=", data, data_size);
			uint16_t unb,uid = 0;
//			printf("nb init:%s\n",nb);
			if (nb) {unb = atoi(nb); infree(nb);}
			else unb = 1;
//			printf("unb init:%d\n",unb);
			char* id; char* url; char* file; char* name; char* port; char* ovol;
			struct shoutcast_info *si =  inmalloc(sizeof(struct shoutcast_info)*unb);
			struct shoutcast_info *nsi ;
			if (si == NULL) {
				printf("setStation SI inmalloc failed or illegal nb %d\n",unb);
				respOk(conn);
				return;
			}
			char* bsi = (char*)si;
			int j;
			for (j=0;j< sizeof(struct shoutcast_info)*unb;j++) bsi[j]=0; //clean 

			for (i=0;i<unb;i++)
			{
				nsi = si + i;
				id = getParameterFromResponse("id=", data, data_size);
				url = getParameterFromResponse("url=", data, data_size);
				file = getParameterFromResponse("file=", data, data_size);
				pathParse(file);
				name = getParameterFromResponse("name=", data, data_size);
				port = getParameterFromResponse("port=", data, data_size);
				ovol = getParameterFromResponse("ovol=", data, data_size);
//printf("nb:%d,si:%x,nsi:%x,id:%s,url:%s,file:%s\n",i,si,nsi,id,url,file);
				if(id ) {
					if (i == 0) uid = atoi(id);
					if ((atoi(id) >=0) && (atoi(id) < 255))
					{	
						if(url && file && name && port) {
							if (strlen(url) > sizeof(nsi->domain)) url[sizeof(nsi->domain)-1] = 0; //truncate if any
							strcpy(nsi->domain, url);
							if (strlen(file) > sizeof(nsi->file)) url[sizeof(nsi->file)-1] = 0; //truncate if any
							strcpy(nsi->file, file);
							if (strlen(name) > sizeof(nsi->name)) url[sizeof(nsi->name)-1] = 0; //truncate if any
							strcpy(nsi->name, name);
							nsi->ovol = (ovol==NULL)?0:atoi(ovol);
							nsi->port = atoi(port);
						}
					} 					
				} 
				infree(ovol);
				infree(port);
				infree(name);
				infree(file);
				infree(url);
				infree(id);
				data = strstr(data,"&&")+2;
//				printf("si:%x, nsi:%x, addr:%x\n",si,nsi,data);
			}
//printf("save station: %d, unb:%d, addr:%x\n",uid,unb,si);
			saveMultiStation(si, uid,unb);
			infree (si);
		}
	} else if(strcmp(name, "/play") == 0) {
		if(data_size > 4) {
			char * id = data+3;
			data[data_size-1] = 0;
				playStation(id);
		}
	} else if(strcmp(name, "/auto") == 0) {
		if(data_size > 4) {
			char * id = data+3;
			data[data_size-1] = 0;
			device = getDeviceSettings();
			device->autostart = strcmp(id,"true")?0:1;
//			printf("autostart: %s, num:%d\n",id,device->autostart);
			saveDeviceSettings(device);
			infree(device);			
		}
	} else if(strcmp(name, "/rauto") == 0) {
		char *buf = inmalloc( 75 + 13);
		if (buf == NULL)
		{	
			printf("post rauto inmalloc fails\n");
			respOk(conn);
		}
		else {			
			device = getDeviceSettings();		
			sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type:application/json\r\nContent-Length:13\r\n\r\n{\"rauto\":\"%c\"}",
(device->autostart)?'1':'0' );
			write(conn, buf, strlen(buf));
			infree(device);	
			infree(buf);
			}
		infree(device);	
		return;		
	} else if(strcmp(name, "/stop") == 0) {
//	    int i;
		if (clientIsConnected())
		{	
			clientDisconnect("Post Stop");
			for (i = 0;i<100;i++)
			{
				if (!clientIsConnected()) break;
				vTaskDelay(4);
			}
		}
	} else if(strcmp(name, "/icy") == 0)	
	{	
//		printf("icy vol \n");
		char currentSt[5]; sprintf(currentSt,"%d",currentStation);
		char vol[5]; sprintf(vol,"%d",(VS1053_GetVolume()));
		char treble[5]; sprintf(treble,"%d",VS1053_GetTreble());
		char bass[5]; sprintf(bass,"%d",VS1053_GetBass());
		char tfreq[5]; sprintf(tfreq,"%d",VS1053_GetTrebleFreq());
		char bfreq[5]; sprintf(bfreq,"%d",VS1053_GetBassFreq());
		char spac[5]; sprintf(spac,"%d",VS1053_GetSpatial());
		device = getDeviceSettings();
				
		struct icyHeader *header = clientGetHeader();
//		printf("icy start header %x\n",header);
		char* not2;
		not2 = header->members.single.notice2;
		if (not2 ==NULL) not2=header->members.single.audioinfo;
		if ((header->members.single.notice2 != NULL)&(strlen(header->members.single.notice2)==0)) not2=header->members.single.audioinfo;
		int json_length ;
		json_length =166+ //144 155
		((header->members.single.description ==NULL)?0:strlen(header->members.single.description)) +
		((header->members.single.name ==NULL)?0:strlen(header->members.single.name)) +
		((header->members.single.bitrate ==NULL)?0:strlen(header->members.single.bitrate)) +
		((header->members.single.url ==NULL)?0:strlen(header->members.single.url))+ 
		((header->members.single.notice1 ==NULL)?0:strlen(header->members.single.notice1))+
		((not2 ==NULL)?0:strlen(not2))+
		((header->members.single.genre ==NULL)?0:strlen(header->members.single.genre))+
		((header->members.single.metadata ==NULL)?0:strlen(header->members.single.metadata))
		+ strlen(currentSt)+	strlen(vol) +strlen(treble)+strlen(bass)+strlen(tfreq)+strlen(bfreq)+strlen(spac)
		;
//		printf("icy start header %x  len:%d vollen:%d vol:%s\n",header,json_length,strlen(vol),vol);
		
		char *buf = inmalloc( json_length + 75);
		if (buf == NULL)
		{	
			printf("post icy inmalloc fails\n");
			respOk(conn);
		}
		else {				
			sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type:application/json\r\nContent-Length:%d\r\n\r\n{\"curst\":\"%s\",\"descr\":\"%s\",\"name\":\"%s\",\"bitr\":\"%s\",\"url1\":\"%s\",\"not1\":\"%s\",\"not2\":\"%s\",\"genre\":\"%s\",\"meta\":\"%s\",\"vol\":\"%s\",\"treb\":\"%s\",\"bass\":\"%s\",\"tfreq\":\"%s\",\"bfreq\":\"%s\",\"spac\":\"%s\",\"auto\":\"%c\"}",
			json_length,
			currentSt,
			(header->members.single.description ==NULL)?"":header->members.single.description,
			(header->members.single.name ==NULL)?"":header->members.single.name,
			(header->members.single.bitrate ==NULL)?"":header->members.single.bitrate,
			(header->members.single.url ==NULL)?"":header->members.single.url,
			(header->members.single.notice1 ==NULL)?"":header->members.single.notice1,
			(not2 ==NULL)?"":not2 ,
			(header->members.single.genre ==NULL)?"":header->members.single.genre,
			(header->members.single.metadata ==NULL)?"":header->members.single.metadata,			
			vol,treble,bass,tfreq,bfreq,spac,
			(device->autostart)?'1':'0' );
//			printf("buf: %s\n",buf);
			write(conn, buf, strlen(buf));
			infree(buf);
		}
		infree(device);	
		return;
	} else if(strcmp(name, "/wifi") == 0)	
	{
		bool val = false;
		char tmpip[16],tmpmsk[16],tmpgw[16];
		struct device_settings *device;
		device = getDeviceSettings();
		uint8_t a,b,c,d;
				
		if(data_size > 0) {
			char* valid = getParameterFromResponse("valid=", data, data_size);
			if(valid != NULL) if (strcmp(valid,"1")==0) val = true;
			char* ssid = getParameterFromResponse("ssid=", data, data_size);
			pathParse(ssid);
			char* pasw = getParameterFromResponse("pasw=", data, data_size);
			pathParse(pasw);
			char* ssid2 = getParameterFromResponse("ssid2=", data, data_size);
			pathParse(ssid2);
			char* pasw2 = getParameterFromResponse("pasw2=", data, data_size);
			pathParse(pasw2);
			char* aip = getParameterFromResponse("ip=", data, data_size);
			char* amsk = getParameterFromResponse("msk=", data, data_size);
			char* agw = getParameterFromResponse("gw=", data, data_size);
			char* aua = getParameterFromResponse("ua=", data, data_size);
			pathParse(aua);
			char* adhcp = getParameterFromResponse("dhcp=", data, data_size);
// printf("rec:%s\nwifi received  valid:%s,val:%d, ssid:%s, pasw:%s, aip:%s, amsk:%s, agw:%s, adhcp:%s, aua:%s \n",data,valid,val,ssid,pasw,aip,amsk,agw,adhcp,aua);
			if (val) {
				ip_addr_t valu;
				ipaddr_aton(aip, &valu);
				memcpy(device->ipAddr,&valu,sizeof(uint32_t));
				ipaddr_aton(amsk, &valu);
				memcpy(device->mask,&valu,sizeof(uint32_t));
				ipaddr_aton(agw, &valu);
				memcpy(device->gate,&valu,sizeof(uint32_t));
				if (adhcp!= NULL) if (strlen(adhcp)!=0) if (strcmp(adhcp,"true")==0)device->dhcpEn = 1; else device->dhcpEn = 0;
				strcpy(device->ssid,(ssid==NULL)?"":ssid);
				strcpy(device->pass,(pasw==NULL)?"":pasw);
				strcpy(device->ssid2,(ssid2==NULL)?"":ssid2);
				strcpy(device->pass2,(pasw2==NULL)?"":pasw2);				
			}
			if (strlen(device->ua)==0)
			{
				if (aua==NULL) {aua=malloc(strlen("Karadio/1.1")+1); strcpy(aua,"Karadio/1.1");}
			}	
			if (aua!=NULL) strcpy(device->ua,aua);
			saveDeviceSettings(device);		
			uint8_t *macaddr = malloc(10*sizeof(uint8_t));
			char* macstr = malloc(20*sizeof(char));
			wifi_get_macaddr ( 0, macaddr );			
			int json_length ;
			json_length =95+ //64 //86
			strlen(device->ssid) +
			strlen(device->pass) +
			strlen(device->ssid2) +
			strlen(device->pass2) +
			strlen(device->ua)+
			sprintf(tmpip,"%d.%d.%d.%d",device->ipAddr[0], device->ipAddr[1],device->ipAddr[2], device->ipAddr[3])+
			sprintf(tmpmsk,"%d.%d.%d.%d",device->mask[0], device->mask[1],device->mask[2], device->mask[3])+
			sprintf(tmpgw,"%d.%d.%d.%d",device->gate[0], device->gate[1],device->gate[2], device->gate[3])+
			sprintf(adhcp,"%d",device->dhcpEn)+
			sprintf(macstr,MACSTR,MAC2STR(macaddr));

			char *buf = inmalloc( json_length + 75);			
			if (buf == NULL)
			{	
				printf("post wifi inmalloc fails\n");
				respOk(conn);
			}
			else {				
				sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type:application/json\r\nContent-Length:%d\r\n\r\n{\"ssid\":\"%s\",\"pasw\":\"%s\",\"ssid2\":\"%s\",\"pasw2\":\"%s\",\"ip\":\"%s\",\"msk\":\"%s\",\"gw\":\"%s\",\"ua\":\"%s\",\"dhcp\":\"%s\",\"mac\":\"%s\"}",
				json_length,
				device->ssid,device->pass,device->ssid2,device->pass2,tmpip,tmpmsk,tmpgw,device->ua,adhcp,macstr);
//				printf("wifi Buf len:%d : %s\n",strlen(buf),buf);
				write(conn, buf, strlen(buf));
				infree(buf);
			}
			infree(ssid); infree(pasw);infree(ssid2); infree(pasw2);  
			infree(aip);infree(amsk);infree(agw);infree(aua);
			infree(valid); infree(adhcp); infree(macaddr); infree(macstr);
		}	
		infree(device);
		if (val){
			vTaskDelay(400);		
			system_restart_enhance(SYS_BOOT_NORMAL_BIN, system_get_userbin_addr());	
		}	
		return;
	} else if(strcmp(name, "/clear") == 0)	
	{
		eeEraseStations();	//clear all stations
	}
	respOk(conn);
}

ICACHE_FLASH_ATTR bool httpServerHandleConnection(int conn, char* buf, uint16_t buflen) {
	char* c;
	char* d;
	xTaskHandle pxCreatedTask;
//	printf ("Heap size: %d\n",xPortGetFreeHeapSize( ));
	if( (c = strstr(buf, "GET ")) != NULL)
	{
//		printf("GET socket:%d str:\n%s\n",conn,buf);
		if( ((d = strstr(buf,"Connection:")) !=NULL)&& ((d = strstr(d," Upgrade")) != NULL))
		{  // a websocket request
			// prepare the parameter of the websocket task
//			printf("websocket request\n");
			struct websocketparam* pvParams = inmalloc(sizeof(struct websocketparam));
			if (pvParams == NULL)
			{	
				printf("websocket request: pvParams null \n");
			}
			char* pbuf = inmalloc(buflen+1);
			if (pbuf == NULL)
			{	
				printf("websocket request: pbuf null\n");
			}			
			if ((pbuf == NULL)|| (pvParams == NULL))
			{
				char resp[] = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
				write(conn, resp, strlen(resp));
				return true;
			}	
			memcpy(pbuf,buf,buflen);
			pvParams->socket = conn;
			pvParams->buf = pbuf;
			pvParams->len = buflen;
//			printf("GET websocket\n");
			while (xTaskCreate( websocketTask,"t11",320,(void *) pvParams,5, &pxCreatedTask )!= pdPASS)  //310
			{
				printf("ws xTaskCreate  failed. Retry\n");
				vTaskDelay(100);
			}
//			printf("t11 task: %x\n",pxCreatedTask);
			return false;
		} else
		{
			char fname[32];
			uint8_t i;
			for(i=0; i<32; i++) fname[i] = 0;
			c += 4;
			char* c_end = strstr(c, "HTTP");
			if(c_end == NULL) return false;
			*(c_end-1) = 0;
			c_end = strstr(c,"?");
//			
// command api, 		
			if(c_end != NULL) // commands api
			{
				char* param;
//				printf("GET commands  socket:%d command:%s\n",conn,c);
// uart command
				param = strstr(c,"uart") ;
				if (param != NULL) {UART_SetBaudrate(0, 115200);}	
// volume command				
				param = getParameterFromResponse("volume=", c, strlen(c)) ;
				if ((param != NULL)&&(atoi(param)>=0)&&(atoi(param)<=254))
				{	
					setVolume(param);
					wsVol(param);
				}	
				incfree(param);
// play command				
				param = getParameterFromResponse("play=", c, strlen(c)) ;
				if (param != NULL) {playStation(param);free(param);}
// start command				
				param = strstr(c,"start") ;
				if (param != NULL) {playStationInt(currentStation);}
// stop command				
				param = strstr(c,"stop") ;
				if (param != NULL) {clientDisconnect("Web stop");free(param);}
// instantplay command				
				param = getParameterFromComment("instant=", c, strlen(c)) ;
				if (param != NULL) {
					clientDisconnect("Web Instant");
					pathParse(param);
//					printf("Instant param:%s\n",param);
					clientParsePlaylist(param);clientConnectOnce();
					free(param);
				}
				
				respOk(conn); // response OK to the origin
			}
			else // file
// file get			
			{
				if(strlen(c) > 32) return false;
//				printf("GET file  socket:%d file:%s\n",conn,c);
				serveFile(c, conn);
//				printf("GET end socket:%d file:%s\n",conn,c);
			}
		}
	} else if( (c = strstr(buf, "POST ")) != NULL) {
// a post request		
//		printf("POST socket: %d\n",conn);
		char fname[32];
		uint8_t i;
		for(i=0; i<32; i++) fname[i] = 0;
		c += 5;
		char* c_end = strstr(c, " ");
		if(c_end == NULL) return;
		uint8_t len = c_end-c;
		if(len > 32) return;
		strncpy(fname, c, len);
//		printf("Name: %s\n", fname);
		// DATA
		char* d_start = strstr(buf, "\r\n\r\n");
//		printf("dstart:%s\n",buf);
		if(d_start > 0) {
			d_start += 4;
			uint16_t len = buflen - (d_start-buf);
			handlePOST(fname, d_start, len, conn);
		}
	}
	return true;
}



// Server child to handle a request from a browser.
ICACHE_FLASH_ATTR void serverclientTask(void *pvParams) {
#define RECLEN	638
	struct timeval timeout; 
    timeout.tv_sec = 2000; // bug *1000 for seconds
    timeout.tv_usec = 0;
	int recbytes ,recb,i;
//	portBASE_TYPE uxHighWaterMark;
	int  client_sock =  (int)pvParams;
	uint16_t reclen = 	RECLEN;	
    char *buf = (char *)inmalloc(reclen);
	bool result = true;
//	for (i = 0;i< reclen+1;i++) buf[i] = 0;
//	printf("Client entry  socket:%x  reclen:%d\n",client_sock,reclen+1);
	if (buf != NULL)
	{
		memset(buf,0,reclen);
		if (setsockopt (client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
				printf("server setsockopt SO_RCVTIMEO failed\n");
			while (((recbytes = read(client_sock , buf, reclen)) > 0)) { // For now we assume max. reclen bytes for request
			if (recbytes < 0) {
				if (errno != EAGAIN )
				{
					printf ("Socket %d read fails %d\n",client_sock, errno);
					vTaskDelay(10);	
					break;
				} else printf("try again\n");
			}	
			char* bend = NULL;
			do {
				bend = strstr(buf, "\r\n\r\n");
				if (bend != NULL) 
				{	
					bend += 4;
//					printf("Server: header len : %d,recbytes = %d,reclen: %d\n",bend - buf,recbytes,reclen);	
					if (strstr(buf,"POST") ) //rest of post?
					{
						uint16_t cl = atoi(strstr(buf, "Content-Length: ")+16);
//						printf("cl: %d, rec:%s\n",cl,buf);
						if ((reclen == RECLEN) && ((bend - buf +cl)> reclen))
						{
							buf = realloc(buf,(2*RECLEN) );
							if (buf == NULL) { printf("serverclientTask realloc fails\n");   break;}
							reclen = 2*RECLEN;
							bend = strstr(buf, "\r\n\r\n")+4;
						}
						vTaskDelay(3);
						if ((bend - buf +cl)> recbytes)
						{	
//							printf ("Server: try receive more:%d bytes. reclen = %d, must be %d\n", recbytes,reclen,bend - buf +cl);
							while(((recb = read(client_sock , buf+recbytes, cl))==0));
							buf[recbytes+recb] = 0;
//							printf ("Server: received more now: %d bytes, rec:%s\n", recbytes+recb,buf);
							if (recb < 0) {
								if (errno != EAGAIN )
								{
									printf ("Socket %d read fails %d\n",client_sock, errno);
									vTaskDelay(10);	
									break;
								} else printf("try again1\n");
							}
							recbytes += recb;
						}
					}
				} 
				else { 
					
//					printf ("Server: try receive more for end:%d bytes\n", recbytes);					
					if (reclen == RECLEN) 
					{
						buf = realloc(buf,(2*RECLEN) +1);
						if (buf == NULL) {printf ("Server: realloc more fails\n");break;}
						reclen = 2*RECLEN;
					}	
					while(((recb= read(client_sock , buf+recbytes, reclen-recbytes))==0))vTaskDelay(1);
//					printf ("Server: received more for end now: %d bytes\n", recbytes+recb);
					if (recb < 0) {
						if (errno != EAGAIN )
						{
							printf ("Socket %d read fails %d\n",client_sock, errno);
							vTaskDelay(10);	
							break;
						} else printf("try again2\n");					
					}
					recbytes += recb;
				} //until "\r\n\r\n"
			} while (bend == NULL);
			result = httpServerHandleConnection(client_sock, buf, recbytes);
			if (!result) 
			{
				break;
			}	
		}
		infree(buf);
	} else printf("Client buf malloc fails socket:%x \n",client_sock);
	if (result)
	{
		int err;
		err = shutdown(client_sock,SHUT_RDWR);
		vTaskDelay(20);
		err = close(client_sock);
		if (err != ERR_OK) 
		{
			err=close(client_sock);
			printf ("closeERR:%d\n",err);
		}
	}
	xSemaphoreGive(semclient);	
/*	
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	printf("watermark serverClientTask: %x  %d\n",uxHighWaterMark,uxHighWaterMark);	
*/

//	printf("Client exit socket:%d result %d \n",client_sock,result);
	vTaskDelete( NULL );	
}	

// main server task. Create a child per request.
ICACHE_FLASH_ATTR void serverTask(void *pvParams) {
	struct sockaddr_in server_addr, client_addr;
	int server_sock, client_sock;
	socklen_t sin_size;
//	portBASE_TYPE uxHighWaterMark;
    semclient = xSemaphoreCreateCounting(2,2); 
	websocketinit();
	os_timer_disarm(&sleepTimer);
	os_timer_disarm(&wakeTimer);
	os_timer_setfn(&sleepTimer, sleepCallback, NULL);
	os_timer_setfn(&wakeTimer, wakeCallback, NULL);
	int stack = 340; //340
	
	
	while (1) {
        bzero(&server_addr, sizeof(struct sockaddr_in));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(80);

        do {		
            if (-1 == (server_sock = socket(AF_INET, SOCK_STREAM, 0))) {
				printf ("Socket fails %d\n", errno);
				vTaskDelay(10);	
                break;
            }

            if (-1 == bind(server_sock, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr))) {
				printf ("Bind fails %d\n", errno);
				close(server_sock);
				vTaskDelay(100);	
                break;
            }

            if (-1 == listen(server_sock, 5)) {
				printf ("Listen fails %d\n",errno);
				close(server_sock);
				vTaskDelay(100);	
                break;
            }

            sin_size = sizeof(client_addr);
            while(1) {				
                if ((client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &sin_size)) < 0) {
					printf ("Accept fails %d\n",errno);
					vTaskDelay(100);					
                } else
				{
					while (1) 
					{	
						if (xPortGetFreeHeapSize( ) < 4000)
						{
//							printf ("Low memory %d\n",xPortGetFreeHeapSize( ));
							vTaskDelay(300);	
							printf ("Heap size low mem: %d\n",xPortGetFreeHeapSize( ));
							if (xPortGetFreeHeapSize( ) < 4000)
							{
								printf ("Low memory %d\n",xPortGetFreeHeapSize( ));					
								write(client_sock, lowmemory, strlen(lowmemory));
								close (client_sock);
								break;							
							}	
						} 
//						printf ("Heap size server: %d\n",xPortGetFreeHeapSize( ));
//						printf ("Accept socket %d\n",client_sock);
						if (xSemaphoreTake(semclient,400))
						{ 
							while (xTaskCreate( serverclientTask,
							"t10",
							stack,
							(void *) client_sock,
							4, 
							NULL ) != pdPASS) 
							{
								printf("xTaskCreate t10 failed for stack %d. Retrying...\n",stack);
//								stack -=10;
								vTaskDelay(150);	
							}							
							break;
						} else {vTaskDelay(20);printf("server busy. Retrying...\n");}
					}
				}	
/*				
				uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
				printf("watermark serverTask: %x  %d\n",uxHighWaterMark,uxHighWaterMark);
*/				
			}
        } while (0);
    }
}
