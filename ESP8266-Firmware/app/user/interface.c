/******************************************************************************
 * Copyright 2015 Piotr Sperka (http://www.piotrsperka.info)/*
 * Copyright 2016 karawin (http://www.karawin.fr)
*/
#include "interface.h"
#include "user_interface.h"
#include "osapi.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "eeprom.h"


uint16_t currentStation = 0;

//extern uint16_t currentStation;
extern void wsVol(char* vol);
extern void playStation(char* id);
extern void setVolume(char* vol);
#define MAX_WIFI_STATIONS 50
	bool inside = false;

void switchCommand() {
	int adc;
	uint8_t vol;
	char Vol[22];
//	vTaskDelay(100);
//	while (true)
	{	
		adc = system_adc_read(); 
//		if (adc < 940) printf("adc: %d\n",adc);
//		if (adc >940) vTaskDelay(10);
		if (inside&&(adc > 940)) inside = false;
		
		if ((adc >400) && (adc < 580)) // volume +
		{
			vol = VS1053_GetVolume();
			if (vol <244) 
			{	
				vol+=10;	
//				printf("vol %d   vol1 %d\n",vol,vol1);					
				sprintf(Vol,"%d",vol);
				setVolume(Vol);
				wsVol(Vol);	
			}
		}
		else if ((adc >730) && (adc < 830)) // volume -
		{
			vol = VS1053_GetVolume();
			if (vol >10) 
			{	
				vol-=10;
//				printf("vol %d   vol1 %d\n",vol,vol1);					
				sprintf(Vol,"%d",vol);
				setVolume(Vol);
				wsVol(Vol);	
			}
		}		
		if (!inside)
		{	
			if (adc < 200) // stop
			{
				inside = true;
				clientDisconnect();
			}
			else if ((adc >278) && (adc < 380)) //start
			{
				inside = true;
				sprintf(Vol,"%d",currentStation);
				playStation	(Vol);
				sprintf(Vol,"{\"wsstation\":\"%d\"}",currentStation);
				websocketbroadcast(Vol, strlen(Vol));
			}
			else if ((adc >830) && (adc < 930)) // station+
			{
				inside = true;
				wsStationNext();
			}
			else if ((adc >590) && (adc < 710)) // station-
			{
				inside = true;
				wsStationPrev();
			}
//			vTaskDelay(5);	
		}
	};
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
		char buf[64];
		struct bss_info *bss_link = (struct bss_info *) arg;
		printf("\n#WIFI.LIST#");
		while(i > 0)
		{
			i--;
			bss_link = bss_link->next.stqe_next;
			if(bss_link == NULL) break;
			sprintf(buf, "\n%s;%d;%d;%d", bss_link->ssid, bss_link->channel, bss_link->rssi, bss_link->authmode);
			printf(buf);
		}
		printf("\n##WIFI.LIST#");
	}
}

ICACHE_FLASH_ATTR void wifiScan()
{
	wifi_station_scan(NULL, wifiScanCallback);
}

ICACHE_FLASH_ATTR void wifiConnect(char* cmd)
{
	int i;
	struct station_config* cfg = malloc(sizeof(struct station_config));
	
	for(i = 0; i < 32; i++) cfg->ssid[i] = 0;
	for(i = 0; i < 64; i++) cfg->password[i] = 0;
	cfg->bssid_set = 0;
	
	wifi_station_disconnect();
	
	char *t = strstr(cmd, "(\"");
	if(t == 0)
	{
		printf("\n##WIFI.CMD_ERROR#");
		return;
	}
	char *t_end  = strstr(t, "\",\"");
	if(t_end == 0)
	{
		printf("\n##WIFI.CMD_ERROR#");
		return;
	}
	
	strncpy( cfg->ssid, (t+2), (t_end-t-2) );
	
	t = t_end+3;
	t_end = strstr(t, "\")");
	if(t_end == 0)
	{
		printf("\n##WIFI.CMD_ERROR#");
		return;
	}
	
	strncpy( cfg->password, t, (t_end-t)) ;
	
	wifi_station_set_config(cfg);

	if( wifi_station_connect() ) {
		struct device_settings* devset = getDeviceSettings();
		for(i = 0; i < 64; i++) devset->ssid[i] = 0;
		for(i = 0; i < 64; i++) devset->pass[i] = 0;
		for(i = 0; i < strlen(cfg->ssid); i++) devset->ssid[i] = cfg->ssid[i];
		for(i = 0; i < strlen(cfg->password); i++) devset->pass[i] = cfg->password[i];
		saveDeviceSettings(devset);
		free(devset);
		printf("\n##WIFI.CONNECTED#");
	}
	else printf("\n##WIFI.NOT_CONNECTED#");
	
	free(cfg);
}

ICACHE_FLASH_ATTR void wifiConnectMem()
{
	int i;
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
    char *t = strstr(s, "(\"");
	if(t == 0)
	{
		printf("\n##CLI.CMD_ERROR#");
		return;
	}
	char *t_end  = strstr(t, "\")")-2;
    if(t_end <= 0)
    {
		printf("\n##CLI.CMD_ERROR#");
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
    char *t = strstr(s, "(\"");
	if(t == 0)
	{
		printf("\n##CLI.CMD_ERROR#");
		return;
	}
	char *t_end  = strstr(t, "\")")-2;
    if(t_end <= 0)
    {
		printf("\n##CLI.CMD_ERROR#");
		return;
    }
    char *path = (char*) malloc((t_end-t+1)*sizeof(char));
    if(path != NULL)
    {
        uint8_t tmp;
        for(tmp=0; tmp<(t_end-t+1); tmp++) path[tmp] = 0;
        strncpy(path, t+2, (t_end-t));
        clientSetPath(path);
        free(path);
    }
}

ICACHE_FLASH_ATTR void clientParsePort(char *s)
{
    char *t = strstr(s, "(\"");
	if(t == 0)
	{
		printf("\n##CLI.CMD_ERROR#");
		return;
	}
	char *t_end  = strstr(t, "\")")-2;
    if(t_end <= 0)
    {
		printf("\n##CLI.CMD_ERROR#");
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
    char *t = strstr(s, "(\"");
	if(t == 0)
	{
		printf("\n##CLI.CMD_ERROR#");
		return;
	}
	char *t_end  = strstr(t, "\")")-2;
    if(t_end <= 0)
    {
		printf("\n##CLI.CMD_ERROR#");
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

ICACHE_FLASH_ATTR void clientList()
{
	struct shoutcast_info* si;
	int i;
	printf("\n#CLI.LIST#\n");
	for (i = 0;i <192;i++)
	{
		si = getStation(i);
		if (si->port != 0)
			printf("%3d: %s, %s:%d%s\n",i,si->name,si->domain,si->port,si->file);	
		free(si);
	}	
	printf("\n##CLI.LIST#\n");
}

ICACHE_FLASH_ATTR void clientVol(char *s)
{
    char *t = strstr(s, "(\"");
	if(t == 0)
	{
		printf("\n##CLI.CMD_ERROR#");
		return;
	}
	char *t_end  = strstr(t, "\")")-2;
    if(t_end <= 0)
    {
		printf("\n##CLI.CMD_ERROR#");
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
			setVolume(vol);
			wsVol(vol);		}	
        free(vol);
    }	
}
ICACHE_FLASH_ATTR void checkCommand(int size, char* s)
{
	char *tmp = (char*)malloc((size+1)*sizeof(char));
	int i;
	for(i=0;i<size;i++) tmp[i] = s[i];
	tmp[size] = 0;
	if(strcmp(tmp, "wifi.list") == 0) wifiScan();
	else if(strcmp(tmp, "wifi.con") == 0) wifiConnectMem();
	else if(startsWith("wifi.con", tmp)) wifiConnect(tmp);
	else if(strcmp(tmp, "wifi.discon") == 0) wifiDisconnect();
	else if(strcmp(tmp, "wifi.status") == 0) wifiStatus();
	else if(strcmp(tmp, "wifi.station") == 0) wifiGetStation();
    else if(startsWith("cli.url", tmp)) clientParseUrl(tmp);
    else if(startsWith("cli.path", tmp)) clientParsePath(tmp);
    else if(startsWith("cli.port", tmp)) clientParsePort(tmp);
    else if(strcmp(tmp, "cli.start") == 0) clientConnect();
    else if(strcmp(tmp, "cli.stop") == 0) clientDisconnect();
    else if(strcmp(tmp, "cli.list") == 0) clientList();
    else if(strcmp(tmp, "cli.next") == 0) wsStationNext();
    else if(strncmp(tmp, "cli.previous",8) == 0) wsStationPrev();
    else if(startsWith("cli.play",tmp)) clientPlay(tmp);
	else if(startsWith("cli.vol",tmp)) clientVol(tmp);
    else if(strcmp(tmp, "sys.erase") == 0) eeEraseAll();
	else printInfo(tmp);
	free(tmp);
	
}
