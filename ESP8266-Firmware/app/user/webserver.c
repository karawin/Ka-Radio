/*
 * Copyright 2016 karawin (http://www.karawin.fr)
*/
#include "webserver.h"
#include "serv-fs.h"
char lowmemory[] = { "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\nContent-Length: 10\r\n\r\n\", \"low memory\")"};

xSemaphoreHandle semclient = NULL ;

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
	free(p);
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
#define PART 1024
	int length;
	int progress,part,gpart;
	char buf[140];
	char *content;
	struct servFile* f = findFile(name);
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
			con = (char*)inmalloc((gpart)*sizeof(char));
			gpart /=2;
		} while ((con == NULL)&&(gpart >=32));
		
		if ((con == NULL)||(gpart <32))
		{
				sprintf(buf, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", (f!=NULL ? f->type : "text/plain"), 0);
				write(conn, buf, strlen(buf));
				printf("serveFile inmalloc error\n");
				if (con) free(con);
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

ICACHE_FLASH_ATTR char* getParameterFromResponse(char* param, char* data, uint16_t data_length) {
	char* p = strstr(data, param);
	if(p != NULL) {
		p += strlen(param);
		char* p_end = strstr(p, "&");
		if(p_end ==NULL) p_end = data_length + data;
		if(p_end != NULL ) {
			if (p_end==p) return NULL;
			char* t = inmalloc(p_end-p + 1);
			if (t == NULL) { printf("getParameterFromResponse inmalloc fails\n"); return NULL;}
			int i;
			for(i=0; i<(p_end-p + 1); i++) t[i] = 0;
			strncpy(t, p, p_end-p);
//			printf("getParam: in: \"%s\"   \"%s\"\n",data,t);
			return t;
		}
	} else return NULL;
}

ICACHE_FLASH_ATTR void respOk(int conn)
{
		char resp[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2\r\n\r\nOK";
		write(conn, resp, strlen(resp));
}

ICACHE_FLASH_ATTR void setVolume(char* vol) {
		struct device_settings *device;
		device = getDeviceSettings();
		if(vol) {
//			printf("setVol: \"%s\"\n",vol);
			VS1053_SetVolume(atoi(vol));
			if (device != NULL)
				if (device->vol != (atoi(vol))){ device->vol = (atoi(vol));saveDeviceSettings(device);}
		}
		if (device != NULL) infree(device);			
}

// treat the received message
void websockethandle(int socket, wsopcode_t opcode, uint8_t * payload, size_t length)
{
	char answer[17];
	struct device_settings *device;
	//wsvol
	if (strstr(payload,"wsvol=")!= NULL)
	{
		if (strstr(payload,"&") != NULL)
			*strstr(payload,"&")=0;
		else return;
		setVolume(payload+6);
		sprintf(answer,"{\"wsvol\":\"%s\"}",payload+6);
		websocketlimitedbroadcast(socket,answer, strlen(answer));
	}
	//monitor
	else if (strstr(payload,"monitor")!= NULL){wsMonitor();}
}


	
ICACHE_FLASH_ATTR void playStation(char* id) {
	struct shoutcast_info* si;
	si = getStation(atoi(id));
	currentStation = atoi(id);
	if(si != NULL &&si->domain && si->file) {
		int i;
		vTaskDelay(5);
		clientDisconnect();
		for (i = 0;i<100;i++)
		{
			if(!clientIsConnected())break;
			vTaskDelay(4);
		}
	
		clientSetURL(si->domain);
		clientSetPath(si->file);
		clientSetPort(si->port);
		clientConnect();
		for (i = 0;i<100;i++)
		{
		  if (clientIsConnected()) break;
		  vTaskDelay(4);
		}
	}
	infree(si);
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
			char* port = getParameterFromResponse("port=", data, data_size);
			int i;
			if(url != NULL && path != NULL && port != NULL) {
				clientDisconnect();
				for (i = 0;i<100;i++)
				{
					if(!clientIsConnected())break;
					vTaskDelay(4);
				}
				
				clientSetURL(url);
				clientSetPath(path);
				clientSetPort(atoi(port));
				clientConnect();
				for (i = 0;i<100;i++)
				{
					if (clientIsConnected()) break;
					vTaskDelay(5);
				}
			} 
			if(url) infree(url);
			if(path) infree(path);
			if(port) infree(port);
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
				if ((atoi(id) >=0) && (atoi(id) < 192)) 
				{
					char ibuf [6];	
					char *buf;
					int i;
					for(i = 0; i<sizeof(ibuf); i++) ibuf[i] = 0;
					struct shoutcast_info* si;
					si = getStation(atoi(id));
					if (strlen(si->domain) > sizeof(si->domain)) si->domain[sizeof(si->domain)-1] = 0; //truncate if any (rom crash)
					if (strlen(si->file) > sizeof(si->file)) si->file[sizeof(si->file)-1] = 0; //truncate if any (rom crash)
					if (strlen(si->name) > sizeof(si->name)) si->name[sizeof(si->name)-1] = 0; //truncate if any (rom crash)
					sprintf(ibuf, "%d", si->port);
					int json_length = strlen(si->domain) + strlen(si->file) + strlen(si->name) + strlen(ibuf) + 40;
					buf = inmalloc(json_length + 75);
					if (buf == NULL)
					{	
						printf("getStation inmalloc fails\n");
						respOk(conn);
					}
					else {				
						for(i = 0; i<sizeof(buf); i++) buf[i] = 0;
						sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n{\"Name\":\"%s\",\"URL\":\"%s\",\"File\":\"%s\",\"Port\":\"%d\"}",
						json_length, si->name, si->domain, si->file, si->port);
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
			char* id = getParameterFromResponse("id=", data, data_size);
			char* url = getParameterFromResponse("url=", data, data_size);
			char* file = getParameterFromResponse("file=", data, data_size);
			char* name = getParameterFromResponse("name=", data, data_size);
			char* port = getParameterFromResponse("port=", data, data_size);
			if(id && url && file && name && port) {
				struct shoutcast_info *si = inmalloc(sizeof(struct shoutcast_info));
				if ((si != NULL) && (atoi(id) >=0) && (atoi(id) < 192))
				{	
					char* bsi = (char*)si;
					int i; for (i=0;i< sizeof(struct shoutcast_info);i++) bsi[i]=0; //clean 
					if (strlen(url) > sizeof(si->domain)) url[sizeof(si->domain)-1] = 0; //truncate if any
					strcpy(si->domain, url);
					if (strlen(file) > sizeof(si->file)) url[sizeof(si->file)-1] = 0; //truncate if any
					strcpy(si->file, file);
					if (strlen(name) > sizeof(si->name)) url[sizeof(si->name)-1] = 0; //truncate if any
					strcpy(si->name, name);
					si->port = atoi(port);
					saveStation(si, atoi(id));
				} else printf("setStation SI inmalloc failed or illegal id %d\n",atoi(id));
				if (si != NULL) infree (si);
			} 
			infree(port);
			infree(name);
			infree(file);
			infree(url);
			infree(id);
		}
	} else if(strcmp(name, "/play") == 0) {
		if(data_size > 4) {
//			char* id = getParameterFromResponse("id=", data, data_size);
			char * id = data+3;
			data[data_size-1] = 0;
//			if(id != NULL) {
				playStation(id);
//				infree(id);
//			}
		}
	} else if(strcmp(name, "/stop") == 0) {
	    int i;
		if (clientIsConnected())
		{	
			clientDisconnect();
			for (i = 0;i<100;i++)
			{
				if (!clientIsConnected()) break;
				vTaskDelay(4);
			}
		}
	} else if(strcmp(name, "/icy") == 0)	
	{	
//		printf("icy vol \n");
		char vol[5]; sprintf(vol,"%d",(VS1053_GetVolume()));
		char treble[5]; sprintf(treble,"%d",VS1053_GetTreble());
		char bass[5]; sprintf(bass,"%d",VS1053_GetBass());
		char tfreq[5]; sprintf(tfreq,"%d",VS1053_GetTrebleFreq());
		char bfreq[5]; sprintf(bfreq,"%d",VS1053_GetBassFreq());
		char spac[5]; sprintf(spac,"%d",VS1053_GetSpatial());
		
		struct icyHeader *header = clientGetHeader();
//		printf("icy start header %x\n",header);
		char* not2;
		not2 = header->members.single.notice2;
		if (not2 ==NULL) not2=header->members.single.audioinfo;
		if ((header->members.single.notice2 != NULL)&(strlen(header->members.single.notice2)==0)) not2=header->members.single.audioinfo;
		int json_length ;
		json_length =144+
		((header->members.single.description ==NULL)?0:strlen(header->members.single.description)) +
		((header->members.single.name ==NULL)?0:strlen(header->members.single.name)) +
		((header->members.single.bitrate ==NULL)?0:strlen(header->members.single.bitrate)) +
		((header->members.single.url ==NULL)?0:strlen(header->members.single.url))+ 
		((header->members.single.notice1 ==NULL)?0:strlen(header->members.single.notice1))+
		((not2 ==NULL)?0:strlen(not2))+
		((header->members.single.genre ==NULL)?0:strlen(header->members.single.genre))+
		((header->members.single.metadata ==NULL)?0:strlen(header->members.single.metadata))
		+	strlen(vol) +strlen(treble)+strlen(bass)+strlen(tfreq)+strlen(bfreq)+strlen(spac)
		;
//		printf("icy start header %x  len:%d vollen:%d vol:%s\n",header,json_length,strlen(vol),vol);
		
		char *buf = inmalloc( json_length + 75);
		if (buf == NULL)
		{	
			printf("post icy inmalloc fails\n");
			respOk(conn);
		}
		else {				
			sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type:application/json\r\nContent-Length:%d\r\n\r\n{\"descr\":\"%s\",\"name\":\"%s\",\"bitr\":\"%s\",\"url1\":\"%s\",\"not1\":\"%s\",\"not2\":\"%s\",\"genre\":\"%s\",\"meta\":\"%s\",\"vol\":\"%s\",\"treb\":\"%s\",\"bass\":\"%s\",\"tfreq\":\"%s\",\"bfreq\":\"%s\",\"spac\":\"%s\"}",
			json_length,
			(header->members.single.description ==NULL)?"":header->members.single.description,
			(header->members.single.name ==NULL)?"":header->members.single.name,
			(header->members.single.bitrate ==NULL)?"":header->members.single.bitrate,
			(header->members.single.url ==NULL)?"":header->members.single.url,
			(header->members.single.notice1 ==NULL)?"":header->members.single.notice1,
			(not2 ==NULL)?"":not2 ,
			(header->members.single.genre ==NULL)?"":header->members.single.genre,
			(header->members.single.metadata ==NULL)?"":header->members.single.metadata,			
			vol,treble,bass,tfreq,bfreq,spac);
//			printf("buf: %s\n",buf);
			write(conn, buf, strlen(buf));
			infree(buf);
		}
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
			char* pasw = getParameterFromResponse("pasw=", data, data_size);
			char* aip = getParameterFromResponse("ip=", data, data_size);
			char* amsk = getParameterFromResponse("msk=", data, data_size);
			char* agw = getParameterFromResponse("gw=", data, data_size);
			char* adhcp = getParameterFromResponse("dhcp=", data, data_size);
//			printf("rec:%s\nwifi received  valid:%s,val:%d, ssid:%s, pasw:%s, aip:%s, amsk:%s, agw:%s, adhcp:%s \n",data,valid,val,ssid,pasw,aip,amsk,agw,adhcp);
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
				saveDeviceSettings(device);		
			}
			int json_length ;
			json_length =56+
			strlen(device->ssid) +
			strlen(device->pass) +
			sprintf(tmpip,"%d.%d.%d.%d",device->ipAddr[0], device->ipAddr[1],device->ipAddr[2], device->ipAddr[3])+
			sprintf(tmpmsk,"%d.%d.%d.%d",device->mask[0], device->mask[1],device->mask[2], device->mask[3])+
			sprintf(tmpgw,"%d.%d.%d.%d",device->gate[0], device->gate[1],device->gate[2], device->gate[3])+
			sprintf(adhcp,"%d",device->dhcpEn);

			char *buf = inmalloc( json_length + 75);			
			if (buf == NULL)
			{	
				printf("post wifi inmalloc fails\n");
				respOk(conn);
			}
			else {				
				sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type:application/json\r\nContent-Length:%d\r\n\r\n{\"ssid\":\"%s\",\"pasw\":\"%s\",\"ip\":\"%s\",\"msk\":\"%s\",\"gw\":\"%s\",\"dhcp\":\"%s\"}",
				json_length,
				device->ssid,device->pass,tmpip,tmpmsk,tmpgw,adhcp	);
//				printf("wifi Buf: %s\n",buf);
				write(conn, buf, strlen(buf));
				infree(buf);
			}
			if (ssid) infree(ssid); if (pasw) infree(pasw); if (aip) infree(aip);if (amsk) infree(amsk);if (agw) infree(agw);
			if (valid) infree(valid);if (adhcp) infree(adhcp);
		}	
		infree(device);
		if (val){
			vTaskDelay(200);		
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
//	printf ("Heap size: %d\n",xPortGetFreeHeapSize( ));
	if( (c = strstr(buf, "GET ")) != NULL)
	{
//		printf("GET socket:%d ",conn);
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
			while (xTaskCreate( websocketTask,"t11",300,(void *) pvParams,4, NULL )!= pdPASS)  //280
			{
				printf("ws xTaskCreate  failed. Retry\n");
				vTaskDelay(100);
			}

			return false;
		} else
		{
			char fname[32];
			uint8_t i;
			for(i=0; i<32; i++) fname[i] = 0;
			c += 4;
			char* c_end = strstr(c, " ");
			if(c_end == NULL) return;
			uint8_t len = c_end-c;
			if(len > 32) return;
			strncpy(fname, c, len);
//			printf("GET in  socket:%d file:%s\n",conn,fname);
			serveFile(fname, conn);
//			printf("GET end socket:%d file:%s\n",conn,fname);
		}
	} else if( (c = strstr(buf, "POST ")) != NULL) {
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
		if(d_start > 0) {
			d_start += 4;
			uint16_t len = buflen - (d_start-buf);
			handlePOST(fname, d_start, len, conn);
		}
	}
	return true;
}




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
				printf("setsockopt failed\n");
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
//						printf("cl: %d\n",cl);
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
//							printf ("Server: try receive more:%d bytes. reclen = %d\n", recbytes,reclen);
							while(((recb = read(client_sock , bend, cl))==0));
//							printf ("Server: received more now: %d bytes\n", recbytes+recb);
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
	} else printf("Client entry buf malloc fails socket:%x \n",client_sock);
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
ICACHE_FLASH_ATTR void serverTask(void *pvParams) {
	struct sockaddr_in server_addr, client_addr;
	int server_sock, client_sock;
	socklen_t sin_size;
//	portBASE_TYPE uxHighWaterMark;
    semclient = xSemaphoreCreateCounting(2,2); 
	websocketinit();
	int stack = 340;
	
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
							printf ("Low memory %d\n",xPortGetFreeHeapSize( ));
							vTaskDelay(300);	
							printf ("Heap size low mem: %d\n",xPortGetFreeHeapSize( ));
							if (xPortGetFreeHeapSize( ) < 4000)
							{
								printf ("Low memory2 %d\n",xPortGetFreeHeapSize( ));					
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
							3, 
							NULL ) != pdPASS) 
							{
								printf("xTaskCreate 1 failed for stack %d. Retrying...\n",stack);
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
