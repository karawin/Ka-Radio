/*
 * Copyright 2016 karawin (http://www.karawin.fr)
*/


#include "webserver.h"
#include "serv-fs.h"
#include "interface.h"
#include "servers.h"
static char apMode[]= {"*Hidden*"};								   
xSemaphoreHandle semfile = NULL ;

const char tryagain[] ICACHE_RODATA_ATTR STORE_ATTR= {"try again"};
const char strsROK[]  ICACHE_RODATA_ATTR STORE_ATTR=  {"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: keep-alive\r\n\r\n%s"};
const char lowmemory[] ICACHE_RODATA_ATTR STORE_ATTR  = { "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 11\r\n\r\nlow memory\n"};
const char strsMALLOC[] ICACHE_RODATA_ATTR STORE_ATTR = {"WebServer inmalloc fails for %d\n"};
const char strsMALLOC1[] ICACHE_RODATA_ATTR STORE_ATTR = {"WebServer %s malloc fails\n"};
const char strsSOCKET[] ICACHE_RODATA_ATTR STORE_ATTR = {"WebServer Socket fails %s errno: %d\n"};
const char strsID[] ICACHE_RODATA_ATTR STORE_ATTR = {"getstation, no id or Wrong id %d\n"};
//const char strsRAUTO[] ICACHE_RODATA_ATTR STORE_ATTR  = {"HTTP/1.1 200 OK\r\nContent-Type:application/json\r\nContent-Length:13\r\n\r\n{\"rauto\":\"%c\"}"};
const char strsTHEME[] ICACHE_RODATA_ATTR STORE_ATTR  = {"HTTP/1.1 200 OK\r\nContent-Type:application/json\r\nContent-Length:13\r\n\r\n{\"theme\":\"%c\"}"};
const char strsICY[] ICACHE_RODATA_ATTR STORE_ATTR  = {"HTTP/1.1 200 OK\r\nContent-Type:application/json\r\nContent-Length:%d\r\n\r\n{\"curst\":\"%s\",\"descr\":\"%s\",\"name\":\"%s\",\"bitr\":\"%s\",\"url1\":\"%s\",\"not1\":\"%s\",\"not2\":\"%s\",\"genre\":\"%s\",\"meta\":\"%s\",\"vol\":\"%s\",\"treb\":\"%s\",\"bass\":\"%s\",\"tfreq\":\"%s\",\"bfreq\":\"%s\",\"spac\":\"%s\",\"auto\":\"%c\"}"};




os_timer_t sleepTimer;
uint32_t sleepDelay;
os_timer_t wakeTimer;
uint32_t wakeDelay;
int8_t clientOvol = 0;
uint8_t clientIvol = 0;

void *inmalloc(size_t n)
{
	void* ret;
//printf ("server Malloc of %d %d,  Heap size: %d\n",n,((n / 32) + 1) * 32,xPortGetFreeHeapSize( ));
	ret = malloc(n);
	
//	printf ("server Malloc of %x : %d bytes Heap size: %d\n",ret,n,xPortGetFreeHeapSize( ));
//	if (n <4) printf("Server: incmalloc size:%d\n",n);	
	return ret;
}	
void infree(void *p)
{
	if (p != NULL)
	{	free(p);
//		printf("server free of %x,  Heap size: %d",(int)p,xPortGetFreeHeapSize( ));
					   
	}
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


ICACHE_FLASH_ATTR void respOk(int conn,char* message)
{
	char rempty[] = {""};
	if (message == NULL) message = rempty;
	char fresp[strlen(strsROK)+strlen(message)+15]; // = inmalloc(strlen(strsROK)+strlen(message)+15);
	if(kasprintf(fresp,PSTR("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\nConnection: keep-alive\r\n\r\n%s"),strlen(message),message)) 
//printf("ok %s\n",fresp);
			write(conn, fresp, strlen(fresp));

//printf("respOk exit\n");
}

ICACHE_FLASH_ATTR void respKo(int conn)
{
	write(conn, lowmemory, strlen(lowmemory));		
}

ICACHE_FLASH_ATTR void serveFile(char* name, int conn)
{
#define PART 1024
#define LIMIT 128

	int length;
	int progress,part,gpart;
	char buf[150];
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
//kprintf("find %s at %x\n",name,f);
//	printf ("Heap size: %d\n",xPortGetFreeHeapSize( ));
	gpart = PART;
	if(f != NULL)
	{
		length = f->size;
		content = (char*)f->content;
		progress = 0;
	}
	else length = 0;
	if(length > 0)
	{
		char *con = NULL;
		
		if (xSemaphoreTake(semfile,portMAX_DELAY -1))
		{				
			do {
				gpart /=2;
				con = (char*)inmalloc((gpart)*sizeof(char));
				vTaskDelay(5);
			} while ((con == NULL)&&(gpart >=LIMIT));
		
			if ((con == NULL)||(gpart <LIMIT))
			{
				//sprintf(buf, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", (f!=NULL ? f->type : "text/plain"), 0);
				//write(conn, lowmemory, strlen(lowmemory));
				kprintf(PSTR("serveFile malloc fails. gpart:%d%c"),gpart,0x0d);
				if (gpart <LIMIT)infree(con);
				respKo(conn);
				xSemaphoreGive(semfile);
				return ;
			}	
		
//printf("serveFile socket:%d,  %s. Length: %d  sliced in %d\n",conn,name,length,gpart);		
			if(kasprintf(buf,PSTR( "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Encoding: gzip\r\nContent-Length: %d\r\nConnection: keep-alive\r\n\r\n"), (f!=NULL ? f->type : "text/plain"), length))
			{
//printf("serveFile send %d bytes\n%s\n",strlen(buf),buf);	
				vTaskDelay(2); // why i need it? Don't know. 
				write(conn, buf, strlen(buf));
				progress = length;
				part = gpart;
				if (progress <= part) part = progress;
				while (progress > 0) 
				{
//printf("serveFile socket:%d,  read at %x len: %d\n",conn,content,part);	
					flashRead(con, (uint32_t)content, part);
					write(conn, con, part);
					content += part;
					progress -= part;
					if (progress <= part) part = progress;
					vTaskDelay(1);
				} 
			}
			xSemaphoreGive(semfile);	
		} else {respKo(conn); kprintf(PSTR("semfile fails%c"),0x0D);}
		infree(con);
		//vTaskDelay(1);
	}
	else
	{
		respKo(conn);
	}
//	printf("serveFile socket:%d, end\n",conn);
}



ICACHE_FLASH_ATTR bool getSParameter(char* result,uint32_t len,const char* sep,const char* param, char* data, uint16_t data_length) {
	if ((data == NULL) || (param == NULL))return false;
	
	char* p = strstr(data, param);
	if(p != NULL) {
		p += strlen(param);
		char* p_end = strstr(p, sep);
		if(p_end ==NULL) p_end = data_length + data;
		if(p_end != NULL ) {
			if (p_end==p) return false;
			int i;
			if (len > (p_end-p )) len = p_end-p ;
			for(i=0; i<len; i++) result[i] = 0;
			strncpy(result, p, len);
			result[len]=0;
//			ESP_LOGV(TAG,"getSParam: in: \"%s\"   \"%s\"",data,result);
			return true;
		} else return false;
	} else return false;
}

ICACHE_FLASH_ATTR char* getParameter(char* sep,char* param, char* data, uint16_t data_length) {
	if ((data == NULL) || (param == NULL))return NULL;
	char* p = strstr(data, param);
	if(p != NULL) {
		p += strlen(param);
		char* p_end = strstr(p, sep);
		if(p_end ==NULL) p_end = data_length + data;
		if(p_end != NULL ) {
			if (p_end==p) return NULL;
			char* t = inmalloc(p_end-p + 1);
			if (t == NULL) { printf(PSTR("getParameterF fails%c"),0x0d); return NULL;}
//printf("getParameter malloc of %d  for %s\n",p_end-p + 1,param);
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
ICACHE_FLASH_ATTR bool getSParameterFromResponse(char* result,uint32_t size, const char* param, char* data, uint16_t data_length) {
	return getSParameter(result,size,"&",param,data, data_length) ;
}
ICACHE_FLASH_ATTR char* getParameterFromComment(char* param, char* data, uint16_t data_length) {
	return getParameter("\"",param,data, data_length) ;
}



ICACHE_FLASH_ATTR void clientSetOvol(int8_t ovol)
{
	clientOvol = ovol;
	kprintf(PSTR("##CLI.OVOLSET#: %d\n"),ovol);
	vTaskDelay(20);
}

// set the current volume with its offset
ICACHE_FLASH_ATTR void setOffsetVolume(void) {
		struct device_settings *device;
		device = getDeviceSettings();
		int16_t uvol = 0;
		if (device != NULL) 
		{
			kprintf(PSTR("##CLI.VOL#: %d\n"),device->vol);
			uvol = device->vol+clientOvol;
			infree(device);			
		}
		if (uvol > 254) uvol = 254;
		if (uvol <=0) uvol = 1;
//printf("setOffsetVol: %d\n",clientOvol);
		VS1053_SetVolume(uvol);
}

// set the volume with vol,  add offset
ICACHE_FLASH_ATTR void setVolume(char* vol) {
//		uint16_t ivol = atoi(vol);
		clientIvol = atoi(vol);
		int16_t uvol = atoi(vol);
		uvol += clientOvol;
		if (uvol > 254) uvol = 254;
		if (uvol <0) uvol = 1;
		if(vol) {
//			printf("setVol: \"%s + %d, uvol: %d, clientIvol:%d\"\n",vol,clientOvol,uvol,clientIvol);
			VS1053_SetVolume(uvol);
			kprintf(PSTR("##CLI.VOL#: %d\n"),clientIvol);		
		}
}

ICACHE_FLASH_ATTR uint16_t getVolume() {
	return (VS1053_GetVolume()-clientOvol);
}

// Set the volume with increment vol
ICACHE_FLASH_ATTR void setRelVolume(int8_t vol) {
//	struct device_settings *device;
	char Vol[5];
	int16_t rvol;
	rvol = clientIvol+vol;
	if (rvol <0) rvol = 0;
	if (rvol > 254) rvol = 254;
	if (kasprintf(Vol,PSTR("%d"),rvol))
	{
		setVolume(Vol);
		wsVol(Vol);
	}
}

// send the rssi
ICACHE_FLASH_ATTR void rssi(int socket) {
		char answer[25];
		if (kasprintf(answer,PSTR("{\"wsrssi\":\"%d\"}"),wifi_station_get_rssi()))
		{			
			websocketwrite(socket,answer, strlen(answer));
		}
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
//	struct device_settings *device;
	//wsvol
//	printf("websocketHandle: %s\n",payload);
	if (strstr(payload,"wsvol=")!= NULL)
	{
		char answer[17];
		if (strstr(payload,"&") != NULL)
			*strstr(payload,"&")=0;
		else return;
//		setVolume(payload+6);
		if (kasprintf(answer,PSTR("{\"wsvol\":\"%s\"}"),payload+6))	
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
//	else if (strstr(payload,"upgrade")!= NULL){update_firmware("new");}
	else if (strstr(payload,"theme")!= NULL){theme();}
	else if (strstr(payload,"wsrssi")!= NULL){rssi(socket);}
}


ICACHE_FLASH_ATTR void playStationInt(int sid) {
	struct shoutcast_info* si;
	char answer[24];
	struct device_settings *device;
	si = getStation(sid);

	if(si != NULL &&si->domain && si->file) {
			int i;
			vTaskDelay(4);
//			clientSilentDisconnect();
			clientDisconnect(PSTR("playStationInt"));
			for (i = 0;i<100;i++)
			{
				if(!clientIsConnected())break;
				vTaskDelay(5);
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
				vTaskDelay(5);
			}
	}
	infree(si);
	if (kasprintf(answer,PSTR("{\"wsstation\":\"%d\"}"),sid))
		websocketbroadcast(answer, strlen(answer));
	device = getDeviceSettings();
	if (device != NULL)
	{
//	printf ("playstationInt: %d, device: %d\n",sid,device->currentstation);
		if (device->currentstation != sid)
		{
			device->currentstation = sid;
			saveDeviceSettings(device);
		}
		infree(device);
	}
}
	
ICACHE_FLASH_ATTR void playStation(char* id) {
//	struct shoutcast_info* si;
//	char answer[22];
	int uid;
	uid = atoi(id) ;
//	printf ("playstation: %d\n",uid);
	if (uid < 255)
		currentStation = atoi(id) ;
	playStationInt(currentStation);	
}

// replace special  json char
ICACHE_FLASH_ATTR void pathParse(char* str)
{
	int i ;
	char *pend;
	char  num[3]= {0,0,0};
	uint8_t cc;
	if (str == NULL) return;
	for (i=0 ; i< strlen(str);i++)
	{
		if (str[i] == '%')
		{
			num[0] = str[i+1]; num[1] = str[i+2];
			cc = strtol(num, &pend,16);
			if (cc == '"') // for " in the string
			{
				str[i] = '\\';
				str[i+1] = cc;
				str[i+2] = 0;
			} else
			{
				str[i] = cc;			
				str[i+1]=0;
			}
			if (str[i+3] !=0)strcat(str, str+i+3);
		}
	}
}



ICACHE_FLASH_ATTR void handlePOST(char* name, char* data, int data_size, int conn) {
//printf("HandlePost %s\n",name);
//	char* head = NULL;
	int i;
	bool tst;
	bool changed = false;
	struct device_settings *device;
	if(strcmp(name, "/instant_play") == 0) {
		if(data_size > 0) {
			char url[100];
			tst = getSParameterFromResponse(url,100,"url=", data, data_size);
			char path[200];
			tst &=getSParameterFromResponse(path,200,"path=", data, data_size);
			pathParse(path);
			char port[10];
			tst &=getSParameterFromResponse(port,10,"port=", data, data_size);
			if(tst)  {
				clientDisconnect(PSTR("Post instant_play"));
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
		}
	} else if(strcmp(name, "/soundvol") == 0) {
		if(data_size > 0) {
			char * vol = data+4;
			data[data_size-1] = 0;
//			printf("/sounvol vol: %s num:%d \n",vol, atoi(vol));
			setVolume(vol); 
		}
	} else if(strcmp(name, "/sound") == 0) {
		if(data_size > 0) {
			char bass[6];
			char treble[6];
			char bassfreq[6];
			char treblefreq[6];
			char spacial[6];
			changed = false;
			device = getDeviceSettings();
			if (device != NULL)
			{
			if(getSParameterFromResponse(bass,6,"bass=", data, data_size)) {		
				if (device->bass != atoi(bass))
				{ 
						VS1053_SetBass(atoi(bass));
						changed = true;
						device->bass = atoi(bass); 
				}
			}
			if(getSParameterFromResponse(treble,6,"treble=", data, data_size)) {				
				if (device->treble != atoi(treble))
				{ 
						VS1053_SetTreble(atoi(treble));
						changed = true;
						device->treble = atoi(treble); 
				}
			}
			if(getSParameterFromResponse(bassfreq,6,"bassfreq=", data, data_size)) {					
				if (device->freqbass != atoi(bassfreq))
				{ 
						VS1053_SetBassFreq(atoi(bassfreq));
						changed = true;
						device->freqbass = atoi(bassfreq); 
				}
			}
			if(getSParameterFromResponse(treblefreq,6,"treblefreq=", data, data_size)) {					
				if (device->freqtreble != atoi(treblefreq))
				{
						VS1053_SetTrebleFreq(atoi(treblefreq)); 
						changed = true;
						device->freqtreble = atoi(treblefreq); 
				}
			}
			if(getSParameterFromResponse(spacial,6,"spacial=", data, data_size)) {					
				if (device->spacial != atoi(spacial))
				{
							VS1053_SetSpatial(atoi(spacial)); 
							changed = true;
							device->spacial = atoi(spacial); 
				}
			}
			if (changed) saveDeviceSettings(device);
			infree(device);
			}

		}
	} else if(strcmp(name, "/getStation") == 0) {
		if(data_size > 0) {
			char id[6];
//			char* id = getParameterFromResponse("idgp=", data, data_size);
			if (getSParameterFromResponse(id,6,"idgp=", data, data_size) ) 
			{
				if ((atoi(id) >=0) && (atoi(id) < 255)) 
				{
					char ibuf [6];	
					char *buf;
					for(i = 0; i<sizeof(ibuf); i++) ibuf[i] = 0;
					struct shoutcast_info* si;
					si = getStation(atoi(id));
					if (strlen(si->domain) > sizeof(si->domain)) si->domain[sizeof(si->domain)-1] = 0; //truncate if any (rom crash)
					if (strlen(si->file) > sizeof(si->file)) si->file[sizeof(si->file)-1] = 0; //truncate if any (rom crash)
					if (strlen(si->name) > sizeof(si->name)) si->name[sizeof(si->name)-1] = 0; //truncate if any (rom crash)
					if (kasprintf(ibuf, PSTR("%d%d"), si->ovol,si->port))
					{
						int json_length = strlen(si->domain) + strlen(si->file) + strlen(si->name) + strlen(ibuf) + 50;
						buf = inmalloc(json_length + 75);	
						if ((buf == NULL))
						{	
							printf(strsMALLOC1,("getStation"));
							//printf("getStation\n");
							respKo(conn);
						}
						else {
							for(i = 0; i<sizeof(buf); i++) buf[i] = 0;
							kasprintf(buf, PSTR("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n{\"Name\":\"%s\",\"URL\":\"%s\",\"File\":\"%s\",\"Port\":\"%d\",\"ovol\":\"%d\"}"),					
							json_length, si->name, si->domain, si->file,si->port,si->ovol);
//							printf("getStation Buf len:%d : %s\n",strlen(buf),buf);						
							write(conn, buf, strlen(buf));
							infree(buf);
						}
					} else respKo(conn);
					infree(si);
					return;
				} else printf(strsID,atoi(id));
//				infree (id);
			} 			
		}
	} else if(strcmp(name, "/setStation") == 0) {
		if(data_size > 0) {
//printf("data:%s\n",data);
			char nb[6] ;
			bool res;
			uint16_t unb,uid = 0;
//printf("nb init:%s\n",nb);
			bool pState = getState();  // remember if we are playing
			res=getSParameterFromResponse(nb,6,"nb=", data, data_size);
			if (res) 
				unb = atoi(nb);
			else unb = 1;

//printf("unb init:%d\n",unb);
			char* url; char* file; char* name;
			struct shoutcast_info *si =  inmalloc(sizeof(struct shoutcast_info)*unb);
			struct shoutcast_info *nsi ;
			
			if (si == NULL) {
				printf(strsMALLOC1,"setStation");
				respKo(conn);
				return;
			}
			char* bsi = (char*)si;
			int j;
			for (j=0;j< sizeof(struct shoutcast_info)*unb;j++) bsi[j]=0; //clean 
			
			char id[6];
			char port[6];
			char ovol[6];
			for (i=0;i<unb;i++)
			{
				nsi = si + i;
				url = getParameterFromResponse("url=", data, data_size);
				file = getParameterFromResponse("file=", data, data_size);
				pathParse(file);
				name = getParameterFromResponse("name=", data, data_size);
				if(getSParameterFromResponse(id,6,"id=", data, data_size)) {
					if (i == 0) uid = atoi(id);
					if ((atoi(id) >=0) && (atoi(id) < 255))
					{	
						if(url && file && name && getSParameterFromResponse(port,6,"port=", data, data_size)) {
							if (strlen(url) > sizeof(nsi->domain)) url[sizeof(nsi->domain)-1] = 0; //truncate if any
							strcpy(nsi->domain, url);
							if (strlen(file) > sizeof(nsi->file)) url[sizeof(nsi->file)-1] = 0; //truncate if any
							strcpy(nsi->file, file);
							if (strlen(name) > sizeof(nsi->name)) url[sizeof(nsi->name)-1] = 0; //truncate if any
							strcpy(nsi->name, name);
							nsi->ovol = (getSParameterFromResponse(ovol,6,"ovol=", data, data_size))?atoi(ovol):0;
							nsi->port = atoi(port);
						}
					} 						
				} 
				infree(name);
				infree(file);
				infree(url);
				
				data = strstr(data,"&&")+2;
//				printf("si:%x, nsi:%x, addr:%x\n",si,nsi,data);
			}
//printf("save station: %d, unb:%d, addr:%x\n",uid,unb,si);
			saveMultiStation(si, uid,unb);
//printf("save station return: %d, unb:%d, addr:%x\n",uid,unb,si);
			infree (si);	
			if (pState != getState()) 
				if (pState) {clientConnect();vTaskDelay(200);}	 //we was playing so start again the play		
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
			if (device != NULL)
			{
				if ((strcmp(id,"true"))&&(device->autostart==1))
				{
					device->autostart = 0;
//printf("autostart: %s, num:%d\n",id,device->autostart);
					saveDeviceSettings(device);
				}
				else
				if ((strcmp(id,"false"))&&(device->autostart==0))
				{
					device->autostart = 1;
//printf("autostart: %s, num:%d\n",id,device->autostart);
					saveDeviceSettings(device);
				}
				infree(device);	
			}			
		}
	} else if(strcmp(name, "/rauto") == 0) {
		char *buf = inmalloc(106);
		if (buf == NULL)
		{	
			printf(strsMALLOC1,("post rauto"));
			respKo(conn);
		}
		else {			
			device = getDeviceSettings();
			if (device != NULL)
			{
				if (kasprintf(buf,PSTR("HTTP/1.1 200 OK\r\nContent-Type:application/json\r\nContent-Length:13\r\n\r\n{\"rauto\":\"%c\"}"),(device->autostart)?'1':'0' ))
				{
					write(conn, buf, strlen(buf));
					infree(buf);
				} else respKo(conn);
				infree(device);	
			}
		}	
		return;		
	} else if(strcmp(name, "/theme") == 0) {
		char *buf = inmalloc( strlen(strsTHEME)+16);
		if (buf == NULL)
		{	
			printf(strsMALLOC1,("post theme"));
			respKo(conn);
		}
		else {			
			device = getDeviceSettings();
			if (device != NULL)
			{
				if (kasprintf(buf, strsTHEME,(device->options & T_THEME)?'1':'0' ))
				{
					write(conn, buf, strlen(buf));
					infree(buf);
					infree(device);	
				} else respKo(conn);
			}
		}		
		return;			
	} else if(strcmp(name, "/stop") == 0) {
//	    int i;
		if (clientIsConnected())
		{	
			clientDisconnect(PSTR("Post Stop"));
			for (i = 0;i<100;i++)
			{
				if (!clientIsConnected()) break;
				vTaskDelay(4);
			}
		}
	} else if(strcmp(name, "/upgrade") == 0)
	{
		update_firmware("new");  // start the OTA
	} else if(strcmp(name, "/icy") == 0)	{	
//		printf("icy vol \n");
		char currentSt[5]; kasprintf(currentSt,PSTR("%d"),currentStation);
		char vol[5]; kasprintf(vol,PSTR("%d"),(VS1053_GetVolume()));
		char treble[5]; kasprintf(treble,PSTR("%d"),VS1053_GetTreble());
		char bass[5]; kasprintf(bass,PSTR("%d"),VS1053_GetBass());
		char tfreq[5]; kasprintf(tfreq,PSTR("%d"),VS1053_GetTrebleFreq());
		char bfreq[5]; kasprintf(bfreq,PSTR("%d"),VS1053_GetBassFreq());
		char spac[5]; kasprintf(spac,PSTR("%d"),VS1053_GetSpatial());
				
		struct icyHeader *header = clientGetHeader();
//		printf("icy start header %x\n",header);
		char* not2;
		not2 = header->members.single.notice2;
		if (not2 ==NULL) not2=header->members.single.audioinfo;
		if ((header->members.single.notice2 != NULL)&&(strlen(header->members.single.notice2)==0)) not2=header->members.single.audioinfo;
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
		if ((buf == NULL) )
		{	
			printf(strsMALLOC1,"post icy");
			respKo(conn);
		}
		else 
		{	
			uint8_t vauto = 0;
			device = getDeviceSettings();
			if (device != NULL)
			{
				vauto = (device->autostart)?'1':'0' ;
				infree(device);	
			}

			if (kasprintf(buf, strsICY,
				
//			sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type:application/json\r\nContent-Length:%d\r\n\r\n{\"curst\":\"%s\",\"descr\":\"%s\",\"name\":\"%s\",\"bitr\":\"%s\",\"url1\":\"%s\",\"not1\":\"%s\",\"not2\":\"%s\",\"genre\":\"%s\",\"meta\":\"%s\",\"vol\":\"%s\",\"treb\":\"%s\",\"bass\":\"%s\",\"tfreq\":\"%s\",\"bfreq\":\"%s\",\"spac\":\"%s\",\"auto\":\"%c\"}",
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
			vauto ))
			{
//				printf("test: len fmt:%d %d\n%s\nfmt: %s",strlen(strsICY),strlen(fmt),buf,fmt);
				write(conn, buf, strlen(buf));
				infree(buf);
				wsMonitor();
			} else respKo (conn);			
		}		
		return;
		
	} else if(strcmp(name, "/wifi") == 0)	
	{
		bool val = false;
		char tmpip[16],tmpmsk[16],tmpgw[16],tmptzo[10];;
		struct device_settings *device;
		struct device_settings1 *device1;
//		uint8_t a,b,c,d;
		changed = false;		
		if(data_size > 0) {
			char valid[5];
			device = getDeviceSettings();
			device1 = getDeviceSettings1();
			if ((device ==NULL)||(device1==NULL))
			{
				infree(device);
				infree(device1);
				respKo(conn);
				return;
			}
			if(getSParameterFromResponse(valid,5,"valid=", data, data_size))
				if (strcmp(valid,"1")==0) val = true;
			char* aua = getParameterFromResponse("ua=", data, data_size);
			pathParse(aua);
			char* host = getParameterFromResponse("host=", data, data_size);
			pathParse(host);
			char* tzo = getParameterFromResponse("tzo=", data, data_size);
			pathParse(tzo);

			if (val) {
				char adhcp[4];
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
//				char* adhcp = getSParameterFromResponse("dhcp=",4, data, data_size);				
				changed = true;
				ip_addr_t valu;
				if (aip != NULL)
				{
					ipaddr_aton(aip, &valu);
					memcpy(device->ipAddr,&valu,sizeof(uint32_t));
					ipaddr_aton(amsk, &valu);
					memcpy(device->mask,&valu,sizeof(uint32_t));
					ipaddr_aton(agw, &valu);
					memcpy(device->gate,&valu,sizeof(uint32_t));
				}
				if (getSParameterFromResponse(adhcp,4,"dhcp=", data, data_size))
					if (strlen(adhcp)!=0) 
					{if (strcmp(adhcp,"true")==0) 
					device->dhcpEn = 1; else device->dhcpEn = 0;}
//				if (adhcp!= NULL) if (strlen(adhcp)!=0) if (strcmp(adhcp,"true")==0)device->dhcpEn = 1; else device->dhcpEn = 0;
				strcpy(device->ssid,(ssid==NULL)?"":ssid);
				strcpy(device->ssid2,(ssid2==NULL)?"":ssid2);
				
				if (pasw != NULL) {
					if (strcmp(pasw,apMode)!=0) strcpy(device->pass,pasw);
				}
				if (pasw2 != NULL){
					if (strcmp(pasw2,apMode)!=0) strcpy(device->pass2,pasw2);
					if (strcmp(pasw2,apMode)!=0) strcpy(device1->pass2,pasw2);					
				}						

				infree(ssid);infree(pasw);infree(ssid2); infree(pasw2);
				infree(aip);infree(amsk);infree(agw); 
			}
			if ((device->ua!= NULL)&&(strlen(device->ua)==0))
			{
				if (aua==NULL) {aua= inmalloc(12); strcpy(aua,"Karadio/2.1");}
			}	
			if (aua!=NULL) 
			{
				if (strcmp(device->ua,aua) != 0)
				{
					strcpy(device->ua,aua);
					changed = true;
				}
				infree(aua);
			}
			
			if (host!=NULL)
			{ 
				if (strlen(host) >0)
				{
					if ((strcmp(device1->hostname,host) != 0)&&(strcmp(host,"undefined") != 0))
					{
						strncpy(device1->hostname,host,HOSTLEN-1);
						printf(PSTR("HOSTAME  %s. Need to restart\n"),device1->hostname);
//						setHostname(device1->hostname);
						changed = true;
					}	
				}				
				infree(host);
			}
			
			if (tzo==NULL) 
			{
				tzo= inmalloc(10); 
				kasprintf(tmptzo,PSTR("%d"),device->tzoffset);
				strcpy(tzo,tmptzo); 
			}
			else if (strlen(tzo) ==0)
			{
				free (tzo);
				tzo= inmalloc(10); strcpy(tzo,"0");
			}

			if (strlen(tzo) >0)
			{
				if (strcmp(tzo,"undefined") != 0)
				{
					device->tzoffset= atoi(tzo);
//					addonDt();
					changed = true;
				}	
			}				
			infree(tzo);			
			
			if (changed)
			{
				saveDeviceSettings(device);	
				saveDeviceSettings1(device1);
			}	
			
			//uint8_t *macaddr = inmalloc(10*sizeof(uint8_t));
			uint8_t macaddr[10]; // = inmalloc(10*sizeof(uint8_t));
			char macstr[20]; // = inmalloc(20*sizeof(char));
			char adhcp[4];
			//char* macstr = inmalloc(20*sizeof(char));
			wifi_get_macaddr ( 0, macaddr );			
			
			int json_length ;
			json_length =95+ 19+
			strlen(device->ssid) +
//			strlen(ipass11) +
			strlen(device->ssid2) +
//			strlen(ipass22) +
			strlen(device->ua)+
			strlen(device1->hostname)+
			kasprintf(tmptzo,PSTR("%d"),device->tzoffset)+
			kasprintf(tmpip,PSTR("%d.%d.%d.%d"),device->ipAddr[0], device->ipAddr[1],device->ipAddr[2], device->ipAddr[3])+
			kasprintf(tmpmsk,PSTR("%d.%d.%d.%d"),device->mask[0], device->mask[1],device->mask[2], device->mask[3])+
			kasprintf(tmpgw,PSTR("%d.%d.%d.%d"),device->gate[0], device->gate[1],device->gate[2], device->gate[3])+
			kasprintf(adhcp,PSTR("%d"),device->dhcpEn)+
			kasprintf(macstr,PSTR(MACSTR),MAC2STR(macaddr));

			char *buf = inmalloc( json_length + 95);
			if ((buf == NULL) )
			{	
				printf(strsMALLOC1,("post wifi"));
				respKo(conn);
			}
			else {			
				if (kasprintf(buf, PSTR("HTTP/1.1 200 OK\r\nContent-Type:application/json\r\nContent-Length:%d\r\n\r\n{\"ssid\":\"%s\",\"pasw\":\"%s\",\"ssid2\":\"%s\",\"pasw2\":\"%s\",\
\"ip\":\"%s\",\"msk\":\"%s\",\"gw\":\"%s\",\"ua\":\"%s\",\"dhcp\":\"%s\",\"mac\":\"%s\",\"host\":\"%s\",\"tzo\":\"%s\"}"),
				json_length,
				device->ssid,"",device->ssid2,"",tmpip,tmpmsk,tmpgw,device->ua,adhcp,macstr,device1->hostname,tmptzo))
				{
//printf(PSTR("wifi Buf len:%d\n%s\nfmt:%s\n"),strlen(buf),buf,fmt);
					write(conn, buf, strlen(buf));
				}
				infree(buf);
			}
			
			infree(device);
			infree(device1);
			if (val){
				vTaskDelay(100);		
				system_restart_enhance(SYS_BOOT_NORMAL_BIN, system_get_userbin_addr());	
			}	
			return;
		}	
	} else if(strcmp(name, "/clear") == 0)	
	{
		eeEraseStations();	//clear all stations
	}
	respOk(conn,NULL);
}

ICACHE_FLASH_ATTR bool httpServerHandleConnection(int conn, char* buf, uint16_t buflen) {
	char* c;
	char* d;
//	char websocketmsg[] = {"websocket request: %s\n"};

//	xTaskHandle pxCreatedTask;
//	printf ("Heap size: %d\n",xPortGetFreeHeapSize( ));
//printf("httpServerHandleConnection  %20c \n",&buf);
	if( (c = strstr(buf, "GET ")) != NULL)
	{
//kprintfl("GET socket:%d str:\n%s\n",conn,buf);
		if( ((d = strstr(buf,"Connection:")) !=NULL)&& ((d = strstr(d," Upgrade")) != NULL))
		{  // a websocket request
//printf ("websocketAccept socket: %d\n",conn);
			websocketAccept(conn,buf,buflen);	
			return false;
		} else
		{
//			char fname[32];
//			uint8_t i;
//			for(i=0; i<32; i++) fname[i] = 0;
			c += 4;
			char* c_end = strstr(c, "HTTP");
			if(c_end == NULL) return true;
			*(c_end-1) = 0;
			c_end = strstr(c,"?");
//			
// web command api,
/////////////////// 		
			if(c_end != NULL) // commands api
			{
				char* param;
//				printf("GET commands  socket:%d command:%s\n",conn,c);
// uart command
				param = strstr(c,"uart") ;
				if (param != NULL) {uart_div_modify(0, UART_CLK_FREQ / 115200) ; } //UART_SetBaudrate(0, 115200);}	
// volume command				
				param = getParameterFromResponse("volume=", c, strlen(c)) ;
				if ((param != NULL)&&(atoi(param)>=0)&&(atoi(param)<=254))
				{	
					setVolume(param);
					wsVol(param);
				}	
				infree(param);
// play command				
				param = getParameterFromResponse("play=", c, strlen(c)) ;
				if (param != NULL) {playStation(param);infree(param);}
// start command				
				param = strstr(c,"start") ;
				if (param != NULL) {playStationInt(currentStation);}
// stop command				
				param = strstr(c,"stop") ;
				if (param != NULL) {clientDisconnect(PSTR("Web stop"));}
// next command				
				param = strstr(c,"next") ;
				if (param != NULL) {wsStationNext();}
// prev command				
				param = strstr(c,"prev") ;
				if (param != NULL) {wsStationPrev();}	
// instantplay command				
				param = getParameterFromComment("instant=", c, strlen(c)) ;
				if (param != NULL) {
					clientDisconnect(PSTR("Web Instant"));
					pathParse(param);
//					printf("Instant param:%s\n",param);
					clientParsePlaylist(param);clientConnectOnce();
					infree(param);
				}
// version command				
				param = strstr(c,"version") ;
				if (param != NULL) {
					char* vr = malloc(30);
					if (vr != NULL)
					{
						if (kasprintf(vr,PSTR("Release: %s, Revision: %s\n"),RELEASE,REVISION))
						{
							respOk(conn,vr); 
							infree(vr);
						} else respKo(conn);
						return true;
					}
				}
// infos command				
				param = strstr(c,"infos") ;				
				if (param != NULL) {
					char* vr = webInfo();	
//					printf("Infos vr:%s\n",vr);
					respOk(conn,vr); 
					infree(vr);
//					printf("Infos:%s\n",param);
					return true;
				}		
// list command	 ?list=1 to list the name of the station 1			
				param = getParameterFromResponse("list=", c, strlen(c)) ;
				if ((param != NULL)&&(atoi(param)>=0)&&(atoi(param)<=254))
				{
					char* vr = webList(atoi(param));
					respOk(conn,vr); 
					infree(vr);
					return true;
				}				
				respOk(conn,NULL); // response OK to the origin
			}
			else 
// file GET		
			{
				if(strlen(c) > 32) {
					respKo(conn); 
					return true;}
//printf("GET file  socket:%d file:%s\n",conn,c);
				serveFile(c, conn);
//printf("GET end socket:%d file:%s\n",conn,c);
			}
		}
	} else if( (c = strstr(buf, "POST ")) != NULL) {
// a post request		
//kprintf("POST socket: %d  buflen: %d\n",conn,buflen);
		char fname[32];
		uint8_t i;
		for(i=0; i<32; i++) fname[i] = 0;
		c += 5;
		char* c_end = strstr(c, " ");
		if(c_end == NULL) return true;
		uint8_t len = c_end-c;
		if(len > 32) return true;
		strncpy(fname, c, len);
//		printf("Name: %s\n", fname);
		// DATA
		char* d_start = strstr(buf, "\r\n\r\n");
//	printf("dstart:%s\n",d_start);
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
#define RECLEN	768
	struct timeval timeout; 
    timeout.tv_sec = 2000; // bug *1000 for seconds
    timeout.tv_usec = 0;
	int recbytes ,recb;
//	portBASE_TYPE uxHighWaterMark;
	int  client_sock =  (int)pvParams;
	uint32_t reclen = 	RECLEN;	
    char *buf = (char *)inmalloc(reclen);
	bool result = true;

//kprintf("Client entry  socket:%x  reclen:%d\n",client_sock,reclen);
	
	while (buf == NULL)
	{
		vTaskDelay(100);
		kprintf(PSTR("SC Buff null\n"));
		buf = (char *)inmalloc(reclen); // second chance
	}
//kprintf("Client entry 1 socket:%x  reclen:%d\n",client_sock,reclen);
	if (buf != NULL)
	{
//kprintf("Client entry 2 socket:%x  reclen:%d\n",client_sock,reclen);
		memset(buf,0,reclen);
		if (setsockopt (client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
			printf(strsSOCKET,"setsockopt",errno);
//kprintf("Client entry 3 socket:%x  reclen:%d\n",client_sock,reclen);

		while (((recbytes = read(client_sock , buf, reclen)) != 0)) 
		{ // For now we assume max. reclen bytes for request with 2*reclen extention if needed
			if (recbytes < 0) {
				break;
				if (errno != EAGAIN )
				{
					kprintf(strsSOCKET,"client_sock",errno);
					vTaskDelay(10);	
					break;
				} else {kprintf(strsSOCKET,tryagain,errno);break;}
			}	
			char* bend = NULL;
			do {
				bend = strstr(buf, "\r\n\r\n");
				if (bend != NULL) 
				{	
					bend += 4;
//kprintf("Server: header len : %d,recbytes = %d,reclen: %d\n%s\nend\n",bend - buf,recbytes,reclen,buf);	
					if (strstr(buf,"POST") ) //rest of post?
					{
						uint16_t cl = atoi(strstr(buf, "Content-Length: ")+16);
//printf("cl: %d, rec:%s\n",cl,buf);
						if ((reclen == RECLEN) && ((bend - buf +cl)> reclen))
						{
//printf("cl: %d, rec:%d\n",cl,recbytes);
							buf = realloc(buf,(2*RECLEN) );
							if (buf == NULL) { printf(strsSOCKET,"realloc",errno);   break;}
							reclen = 2*RECLEN;
							bend = strstr(buf, "\r\n\r\n")+4;
						}
						vTaskDelay(1);
						if ((bend - buf +cl)> recbytes)
						{	
//kprintf ("Server: try receive more:%d bytes. reclen = %d, must be %d\n", recbytes,reclen,bend - buf +cl);
							while(((recb = read(client_sock , buf+recbytes, cl))==0)){vTaskDelay(1);printf(PSTR("."));}
							buf[recbytes+recb] = 0;
//printf ("Server: received more now: %d bytes, rec:\n%s\nEnd\n", recbytes+recb,buf);
							if (recb < 0) {
								respKo(client_sock);
								break;
/*								if (errno != EAGAIN )
								{
									printf(strsSOCKET,"read",errno);
									vTaskDelay(10);	
									break;
								} else printf(strsSOCKET,tryagain,errno);
*/								
							}
							recbytes += recb;
						}
					}
				} 
				else { 
					
//kprintf ("Server: try receive more for end:%d bytes\n", recbytes);					
					if (reclen == RECLEN) 
					{
//kprintf ("Server: try receive more for end:%d bytes\n", recbytes);
						buf = realloc(buf,(2*RECLEN) +1);
						if (buf == NULL) {printf(strsSOCKET,"Realloc",errno);break;}
						reclen = 2*RECLEN;
					}	
					while(((recb= read(client_sock , buf+recbytes, reclen-recbytes))==0)) vTaskDelay(1);
//kprintf ("Server: received more for end now: %d bytes\n", recbytes+recb);
					if (recb < 0) {
						respKo(client_sock);
						break;
/*						if (errno != EAGAIN )
						{
							printf(strsSOCKET,"read",errno);
							vTaskDelay(10);	
							break;
						} else printf(strsSOCKET,tryagain,errno);	
*/	
					}
					recbytes += recb;
				} //until "\r\n\r\n"
			} while (bend == NULL);
//kprintf ("Server: call httpServerHandleConnection: %d bytes\n", recbytes);
			result = httpServerHandleConnection(client_sock, buf, recbytes);
//kprintf ("Server: exit httpServerHandleConnection: %d bytes\n", recbytes);
			if (reclen == 2*RECLEN)
			{
				reclen = RECLEN;
				buf = realloc(buf,reclen);
				vTaskDelay(5);	
			}
			//if (buf == NULL) printf("WARNING\n");
			memset(buf,0,reclen);
			if (!result) 
			{
				break; // only a websocket created. exit without closing the socket
			}	
			vTaskDelay(1);
		}
		infree(buf);
	} else  kprintf(strsMALLOC1,"buf");
	if (result)
	{
		int err;
		err = shutdown(client_sock,SHUT_RDWR);
		vTaskDelay(20);
		err = close(client_sock);
		if (err != ERR_OK) 
		{
			err=close(client_sock);
//			printf ("closeERR:%d\n",err);
		}
	}
	xSemaphoreGive(semclient);	
//kprintf (PSTR("Give client_sock: %d %s"),client_sock,"\n");		
/*	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	printf("watermark serverClientTask: %x  %d\n",uxHighWaterMark,uxHighWaterMark);	
*/

//	printf("Client exit socket:%d result %d \n",client_sock,result);
	vTaskDelete( NULL );	
}	


