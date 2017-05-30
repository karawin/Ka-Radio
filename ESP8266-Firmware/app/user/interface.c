/******************************************************************************
 * Copyright 2015 Piotr Sperka (http://www.piotrsperka.info)/*
 * Copyright 2016 karawin (http://www.karawin.fr)
*/
#include "interface.h"
#include "user_interface.h"
//#include "osapi.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "eeprom.h"
#include "ntp.h"
char parslashquote[] = {"(\""};
char parquoteslash[] = {"\")"};
char msgsys[] = {"##SYS."};
char msgcli[] = {"##CLI."};

uint16_t currentStation = 0;

//extern uint16_t currentStation;
extern void wsVol(char* vol);
extern void playStation(char* id);
extern void setVolume(char* vol);
extern void setRelVolume(int8_t vol);
extern uint16_t getVolume(void);
extern uint8_t playing;

ICACHE_FLASH_ATTR void clientVol(char *s);

#define MAX_WIFI_STATIONS 50
char errmsg[]={"##CMD_ERROR#\n"};
bool inside = false;

void setVolumePlus()
{
	setRelVolume(10);
}
void setVolumeMinus()
{
	setRelVolume(-10);
}		
void setVolumew(char* vol)
{
	setVolume(vol);	
	wsVol(vol);
}	

unsigned short adcdiv;	
// Read the command panel
void switchCommand() {
	int adc;
	char Vol[22];
	if (adcdiv == 0) return; // no panel

		adc = system_adc_read(); 
		adc *= adcdiv;
//		if (adc < 930) 
//			printf("adc: %d  div: %d\n",adc,adcdiv);
//		if (adc >940) vTaskDelay(10);
		if (inside&&(adc > 930)) inside = false;
		
		if ((adc >400) && (adc < 580)) // volume +
		{
			setVolumePlus();
		}
		else if ((adc >730) && (adc < 830)) // volume -
		{
			setVolumeMinus();
		}		
		if (!inside)
		{	
			if (adc < 220) // stop
			{
				inside = true;
				clientDisconnect("Adc Stop");
			}
			else if ((adc >278) && (adc < 380)) //start
			{
				inside = true;
				sprintf(Vol,"%d",currentStation);
				playStation	(Vol);
			}
			else if ((adc >830) && (adc < 920)) // station+
			{
				inside = true;
				wsStationNext();
			}
			else if ((adc >590) && (adc < 710)) // station-
			{
				inside = true;
				wsStationPrev();
			}
		}
}




uint8_t startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

ICACHE_FLASH_ATTR void printInfo(char* s)
{
	printf("#INFO:\"%s\"#\n", s);
}

ICACHE_FLASH_ATTR void wifiScanCallback(void *arg, STATUS status)
{
	if(status == OK)
	{
		int i = MAX_WIFI_STATIONS;
		char msg[] = {"#WIFI.LIST#"};
		char* buf;
		struct bss_info *bss_link = (struct bss_info *) arg;
		printf("\n%s",msg);
		buf = malloc(128);
		if (buf == NULL) return;
		while(i > 0)
		{
			i--;
			bss_link = bss_link->next.stqe_next;
			if(bss_link == NULL) break;
			sprintf(buf, "\n%s;%d;%d;%d", bss_link->ssid, bss_link->channel, bss_link->rssi, bss_link->authmode);
			printf(buf);
		}
		printf("\n#%s",msg);
		free(buf);
	}
}

ICACHE_FLASH_ATTR void wifiScan()
{
	wifi_station_scan(NULL, wifiScanCallback);
}

ICACHE_FLASH_ATTR void wifiConnect(char* cmd)
{
	int i;
	struct device_settings* devset = getDeviceSettings();
	for(i = 0; i < 32; i++) devset->ssid[i] = 0;
	for(i = 0; i < 64; i++) devset->pass[i] = 0;	
	char *t = strstr(cmd, parslashquote);
	if(t == 0)
	{
		printf(errmsg);
		return;
	}
	char *t_end  = strstr(t, "\",\"");
	if(t_end == 0)
	{
		printf(errmsg);
		return;
	}
	
	strncpy( devset->ssid, (t+2), (t_end-t-2) );
	
	t = t_end+3;
	t_end = strstr(t, parquoteslash);
	if(t_end == 0)
	{
		printf(errmsg);
		return;
	}
	
	strncpy( devset->pass, t, (t_end-t)) ;
	devset->dhcpEn = 1;
	saveDeviceSettings(devset);
	printf("\n##AP1: %s with dhcp on next reset#\n",devset->ssid);
	free(devset);
}

ICACHE_FLASH_ATTR void wifiConnectMem()
{
	
	struct device_settings* devset = getDeviceSettings();
	printf("\n##AP1: %s#",devset->ssid);
	printf("\n##AP2: %s#\n",devset->ssid2);
	free(devset);
	 
/*	int i;
	struct station_config* cfg = malloc(sizeof(struct station_config));
	
	for(i = 0; i < 32; i++) cfg->ssid[i] = 0;
	for(i = 0; i < 64; i++) cfg->password[i] = 0;
	cfg->bssid_set = 0;
	
	wifi_station_disconnect();
	
	struct device_settings* devset = getDeviceSettings();
	for(i = 0; i < strlen(devset->ssid); i++) cfg->ssid[i] = devset->ssid[i];
	for(i = 0; i < strlen(devset->pass); i++) cfg->password[i] = devset->pass[i];
	free(devset);

	wifi_station_set_config(cfg);

	if( wifi_station_connect() ) printf("\n##WIFI.CONNECTED#");
	else printf("\n##WIFI.NOT_CONNECTED#");
	
	free(cfg);
	*/
}

ICACHE_FLASH_ATTR void wifiDisconnect()
{
	if(wifi_station_disconnect()) printf("\n##WIFI.NOT_CONNECTED#");
	else printf("\n##WIFI.DISCONNECT_FAILED#");
}

ICACHE_FLASH_ATTR void wifiStatus()
{
	struct ip_info ipi;
	uint8_t t = wifi_station_get_connect_status();	
	wifi_get_ip_info(0, &ipi);
	printf("#WIFI.STATUS#\n%d\n%d.%d.%d.%d\n%d.%d.%d.%d\n%d.%d.%d.%d\n##WIFI.STATUS#\n",
			  t, (ipi.ip.addr&0xff), ((ipi.ip.addr>>8)&0xff), ((ipi.ip.addr>>16)&0xff), ((ipi.ip.addr>>24)&0xff),
			 (ipi.netmask.addr&0xff), ((ipi.netmask.addr>>8)&0xff), ((ipi.netmask.addr>>16)&0xff), ((ipi.netmask.addr>>24)&0xff),
			 (ipi.gw.addr&0xff), ((ipi.gw.addr>>8)&0xff), ((ipi.gw.addr>>16)&0xff), ((ipi.gw.addr>>24)&0xff));
}

ICACHE_FLASH_ATTR void wifiGetStation()
{
	struct station_config cfgg;
	wifi_station_get_config(&cfgg);
	printf("\n#WIFI.STATION#\n%s\n%s\n##WIFI.STATION#\n", cfgg.ssid, cfgg.password);
}

ICACHE_FLASH_ATTR void clientParseUrl(char* s)
{
    char *t = strstr(s, parslashquote);
	if(t == 0)
	{
		printf(errmsg);
		return;
	}
	char *t_end  = strstr(t, parquoteslash)-2;
    if(t_end <= 0)
    {
		printf(errmsg);
		return;
    }
    char *url = (char*) malloc((t_end-t+1)*sizeof(char));
    if(url != NULL)
    {
        uint8_t tmp;
        for(tmp=0; tmp<(t_end-t+1); tmp++) url[tmp] = 0;
        strncpy(url, t+2, (t_end-t));
        clientSetURL(url);
        free(url);
    }
}

ICACHE_FLASH_ATTR void clientParsePath(char* s)
{
    char *t = strstr(s, parslashquote);
	printf("cli.path: %s\n",t);
	if(t == 0)
	{
		printf(errmsg);
		return;
	}
	char *t_end  = strstr(t, parquoteslash)-2;
    if(t_end <= 0)
    {
		printf(errmsg);
		return;
    }
    char *path = (char*) malloc((t_end-t+1)*sizeof(char));
    if(path != NULL)
    {
        uint8_t tmp;
        for(tmp=0; tmp<(t_end-t+1); tmp++) path[tmp] = 0;
        strncpy(path, t+2, (t_end-t));
	printf("cli.path: %s\n",path);
        clientSetPath(path);
        free(path);
    }
}

ICACHE_FLASH_ATTR void clientParsePort(char *s)
{
    char *t = strstr(s, parslashquote);
	if(t == 0)
	{
		printf(errmsg);
		return;
	}
	char *t_end  = strstr(t, parquoteslash)-2;
    if(t_end <= 0)
    {
		printf(errmsg);
		return;
    }
    char *port = (char*) malloc((t_end-t+1)*sizeof(char));
    if(port != NULL)
    {
        uint8_t tmp;
        for(tmp=0; tmp<(t_end-t+1); tmp++) port[tmp] = 0;
        strncpy(port, t+2, (t_end-t));
        uint16_t porti = atoi(port);
        clientSetPort(porti);
        free(port);
    }
}


ICACHE_FLASH_ATTR void clientPlay(char *s)
{
    char *t = strstr(s, parslashquote);
	if(t == 0)
	{
		printf(errmsg);
		return;
	}
	char *t_end  = strstr(t, parquoteslash)-2;
    if(t_end <= 0)
    {
		printf(errmsg);
		return;
    }
   char *id = (char*) malloc((t_end-t+1)*sizeof(char));
    if(id != NULL)
    {
        uint8_t tmp;
        for(tmp=0; tmp<(t_end-t+1); tmp++) id[tmp] = 0;
        strncpy(id, t+2, (t_end-t));
		playStation(id);
        free(id);
    }	
}

ICACHE_FLASH_ATTR void clientList(char *s)
{
	struct shoutcast_info* si;
	uint8_t i = 0,j = 255;
	bool onlyOne = false;
	char stlinf[] = {"##CLI.LIST#\n"};
	char *t = strstr(s, parslashquote);
	if(t != NULL) // a number specified
	{	
		char *t_end  = strstr(t, parquoteslash)-2;
		if(t_end <= 0)
		{
			printf(errmsg);
			return;
		}	
		i = atoi(t+2);
		if (i>254) i = 0;
		j = i+1;
		onlyOne = true;
		
	} 
	{	
		printf("\n#CLI.LIST#\n");	
		for (i ;i <j;i++)
		{
			si = getStation(i);
			if ((si == NULL) || (si->port ==0))
			{
				printf("#CLI.LISTINFO#: %3d: not defined, Try\n",i);
				printf(stlinf);
//				printf("##CLI.NAMESET#: %3d\n",i);
				if (si != NULL) free(si);
				return;
			}

			if (si !=NULL)
			{
				if(si->port !=0)
				{	
					printf("#CLI.LISTINFO#: %3d: %s, %s:%d%s\n",i,si->name,si->domain,si->port,si->file);	
					vTaskDelay(1);
				}
				free(si);
			}	
		}	
		printf(stlinf);
	}
}
ICACHE_FLASH_ATTR void clientInfo()
{
	struct shoutcast_info* si;
	si = getStation(currentStation);
	ntp_print_time();
	clientSetName(si->name,currentStation);
	clientPrintHeaders();
	clientVol("");
	clientPrintState();
	free(si);
}

ICACHE_FLASH_ATTR void clientI2S(char* s)
{
    char *t = strstr(s, parslashquote);
	char message[] ={"\n##I2S speed: %d, 0=48kHz, 1=96kHz, 2=192kHz#\n"};
	struct device_settings *device;
	device = getDeviceSettings();
	if(t == NULL)
	{
		printf(message,device->i2sspeed);
		free(device);
		return;
	}
	char *t_end  = strstr(t, parquoteslash);
    if(t_end == NULL)
    {
		printf(errmsg);
		return;
    }	
	uint8_t speed = atoi(t+2);
	VS1053_I2SRate(speed);

	device->i2sspeed = speed;
	saveDeviceSettings(device);	
	printf(message,speed);
	free(device);
}
ICACHE_FLASH_ATTR void clientUart(char* s)
{
	bool empty = false;
	char *t ;
	char *t_end;
	if (s != NULL)
	{	
		t = strstr(s, parslashquote);
		if(t == NULL)
		{
			empty = true;
		} else
		{
			t_end  = strstr(t, parquoteslash);
			if(t_end == NULL)
			{
				empty = true;
			}	
		}
	}
	struct device_settings *device;
	device = getDeviceSettings();
	if ((!empty)&&(t!=NULL))
	{
		uint32_t speed = atoi(t+2);
		speed = checkUart(speed);
		device->uartspeed= speed;
		saveDeviceSettings(device);	
	}
	printf("\n%sUART= %d# on next reset\n",msgcli,device->uartspeed);	
	free(device);
}
ICACHE_FLASH_ATTR void clientVol(char *s)
{
    char *t = strstr(s, parslashquote);
	if(t == 0)
	{
		// no argument, return the current volume
		printf("%sVOL#: %d\n",msgcli,getVolume());
		return;
	}
	char *t_end  = strstr(t, parquoteslash)-2;
    if(t_end <= 0)
    {

		printf(errmsg);
		return;
    }
   char *vol = (char*) malloc((t_end-t+1)*sizeof(char));
    if (vol != NULL)
    {
        uint8_t tmp;
        for(tmp=0; tmp<(t_end-t+1); tmp++) vol[tmp] = 0;
        strncpy(vol, t+2, (t_end-t));
		if ((atoi(vol)>=0)&&(atoi(vol)<=254))
		{	
			setVolumew(vol);	}	
			free(vol);
    }	
}

ICACHE_FLASH_ATTR void syspatch(char* s)
{
    char *t = strstr(s, parslashquote);
	struct device_settings *device;
	device = getDeviceSettings();
	if(t == NULL)
	{
		printf("\n##VS1053 Patch is %s#\n",((device->options & T_PATCH)!= 0)?"Not loaded":"Loaded");
		free(device);
		return;
	}
	char *t_end  = strstr(t, parquoteslash);
    if(t_end == NULL)
    {
		printf(errmsg);
		return;
    }	
	uint8_t value = atoi(t+2);
	if (value ==0) 
		device->options |= T_PATCH; 
	else 
		device->options &= NT_PATCH; // 0 = load patch
	
	saveDeviceSettings(device);	
	printf("\n##VS1053 Patch will be %s after power Off and On#\n",((device->options & T_PATCH)!= 0)?"unloaded":"Loaded");
	free(device);	
}

ICACHE_FLASH_ATTR void sysled(char* s)
{
    char *t = strstr(s, parslashquote);
	struct device_settings *device;
	device = getDeviceSettings();
	extern bool ledStatus;
	if(t == NULL)
	{
		printf("##Led is in %s#\n",((device->options & T_LED)== 0)?"Blink mode":"Play mode");
		free(device);
		return;
	}
	char *t_end  = strstr(t, parquoteslash);
    if(t_end == NULL)
    {
		printf(errmsg);
		return;
    }	
	uint8_t value = atoi(t+2);
	if (value ==0) 
	{device->options |= T_LED; ledStatus = false; if (playing) gpio2_output_set(0);}
	else 
	{device->options &= NT_LED; ledStatus =true;} // options:0 = ledStatus true = Blink mode
	
	saveDeviceSettings(device);	
	printf("##LED is in %s#\n",((device->options & T_LED)== 0)?"Blink mode":"Play mode");
	free(device);
	
}

ICACHE_FLASH_ATTR void tzoffset(char* s)
{
	char *t = strstr(s, parslashquote);
	struct device_settings *device;
	char msg[] = {"##SYS.TZO#: %d\n"};
	device = getDeviceSettings();
	if(t == NULL)
	{
		printf(msg,device->tzoffset);
		free(device);
		return;
	}
	char *t_end  = strstr(t, parquoteslash);
    if(t_end == NULL)
    {
		printf(errmsg);
		return;
    }	
	uint8_t value = atoi(t+2);
	device->tzoffset = value;	
	saveDeviceSettings(device);	
	printf(msg,device->tzoffset);
	free(device);		
}

ICACHE_FLASH_ATTR void heapSize()
{
	printf("%sHEAP: %d #\n",msgsys,xPortGetFreeHeapSize( ));
}

ICACHE_FLASH_ATTR void checkCommand(int size, char* s)
{
	char *tmp = (char*)malloc((size+1)*sizeof(char));
	int i;
	for(i=0;i<size;i++) tmp[i] = s[i];
	tmp[size] = 0;
//	printf("size: %d, cmd=%s\n",size,tmp);
	if(startsWith ("wifi.", tmp))
	{
		if     (strcmp(tmp+5, "list") == 0) 	wifiScan();
		else if(strcmp(tmp+5, "con") == 0) 	wifiConnectMem();
		else if(startsWith ("con", tmp+5)) 	wifiConnect(tmp);
		else if(strcmp(tmp+5, "discon") == 0) wifiDisconnect();
		else if(strcmp(tmp+5, "status") == 0) wifiStatus();
		else if(strcmp(tmp+5, "station") == 0) wifiGetStation();
	} else
	if(startsWith ("cli.", tmp))
	{
		if     (startsWith (  "url", tmp+4)) 	clientParseUrl(tmp);
		else if(startsWith (  "path", tmp+4))	clientParsePath(tmp);
		else if(startsWith (  "port", tmp+4)) 	clientParsePort(tmp);
		else if(strcmp(tmp+4, "instant") == 0) {clientDisconnect("cli instantplay");clientConnectOnce();}
		else if(strcmp(tmp+4, "start") == 0) 	clientPlay("(\"255\")"); // outside value to play the current station
		else if(strcmp(tmp+4, "stop") == 0) 	clientDisconnect("cli stop");
		else if(startsWith (  "list", tmp+4)) 	clientList(tmp);
		else if(strcmp(tmp+4, "next") == 0) 	wsStationNext();
		else if(strncmp(tmp+4,"previous",4) == 0) wsStationPrev();
		else if(startsWith (  "play",tmp+4)) 	clientPlay(tmp);
		else if(strcmp(tmp+4, "vol+") == 0) 	setVolumePlus();
		else if(strcmp(tmp+4, "vol-") == 0) 	setVolumeMinus();
		else if(strcmp(tmp+4, "info") == 0) 	clientInfo();
		else if(startsWith (  "vol",tmp+4)) 	clientVol(tmp);
	} else
	if(startsWith ("sys.", tmp))
	{
			 if(startsWith (  "i2s",tmp+4)) 	clientI2S(tmp);
		else if(startsWith (  "uart",tmp+4)) 	clientUart(tmp);
		else if(strcmp(tmp+4, "erase") == 0) 	eeEraseAll();
		else if(strcmp(tmp+4, "heap") == 0) 	heapSize();
		else if(strcmp(tmp+4, "boot") == 0) 	system_restart();
		else if(strcmp(tmp+4, "update") == 0) update_firmware();
		else if(startsWith (  "patch",tmp+4)) 	syspatch(tmp);
		else if(startsWith (  "led",tmp+4)) 	sysled(tmp);
		else if(strcmp(tmp+4, "date") == 0) 	ntp_print_time();
		else if(startsWith(   "tzo",tmp+4)) 	tzoffset(tmp);
		else if(startsWith(   "log",tmp+4)) 	; // do nothing
	}
	else printInfo(tmp);
	free(tmp);
	
}
