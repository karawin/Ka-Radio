/******************************************************************************
 * Copyright 2015 Piotr Sperka (http://www.piotrsperka.info)
 * Copyright 2016 karawin (http://www.karawin.fr)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
*******************************************************************************/
#include "esp_common.h"
#include "esp_softap.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "el_uart.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "interface.h"
#include "webserver.h"
#include "webclient.h"

#include "vs1053.h"

#include "eeprom.h"
void uart_div_modify(int no, unsigned int freq);

	struct station_config config;
	int FlashOn = 5,FlashOff = 5;
	sc_status status = 0;
void cb(sc_status stat, void *pdata)
{
	printf("SmartConfig status received: %d\n",status);
	status = stat;
	if (stat == SC_STATUS_LINK_OVER) if (pdata) printf("SmartConfig: %d:%d:%d:%d\n",((char*) pdata)[0],((char*)pdata)[1],((char*)pdata)[2],((char*)pdata)[3]);
}

void uartInterfaceTask(void *pvParameters) {
	char tmp[64];
	bool conn = false;
	int uxHighWaterMark;
/*	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	printf("watermark wsTask: %x  %d\n",uxHighWaterMark,uxHighWaterMark);
*/
	int t = 0;
	for(t = 0; t<64; t++) tmp[t] = 0;
	t = 0;
	uart_rx_init();
	printf("UART READY TO READ\n");
	
	wifi_station_set_auto_connect(false);
	wifi_station_set_hostname("WifiWebRadio");

	struct ip_info *info;
	struct device_settings *device;
	struct station_config *config;
	device = getDeviceSettings();
	config = malloc(sizeof(struct station_config));
	info = malloc(sizeof(struct ip_info));
	wifi_get_ip_info(STATION_IF, info);
	wifi_station_get_config_default(config);

		
	if ((strlen(device->ssid)==0)||(device->ssid[0]==0xff)/*||(device->ipAddr[0] ==0)*/) // first use
	{
		printf("first use\n");
		IP4_ADDR(&(info->ip), 192, 168, 1, 254);
		IP4_ADDR(&(info->netmask), 0xFF, 0xFF,0xFF, 0);
		IP4_ADDR(&(info->gw), 192, 168, 1, 254);
		IPADDR2_COPY(&device->ipAddr, &info->ip);
		IPADDR2_COPY(&device->mask, &info->netmask);
		IPADDR2_COPY(&device->gate, &info->gw);
		strcpy(device->ssid,config->ssid);
		strcpy(device->pass,config->password);
		device->dhcpEn = true;
		wifi_set_ip_info(STATION_IF, info);
		saveDeviceSettings(device);	
	}
	
		IP4_ADDR(&(info->ip), device->ipAddr[0], device->ipAddr[1],device->ipAddr[2], device->ipAddr[3]);
		IP4_ADDR(&(info->netmask), device->mask[0], device->mask[1],device->mask[2], device->mask[3]);
		IP4_ADDR(&(info->gw), device->gate[0], device->gate[1],device->gate[2], device->gate[3]);

		strcpy(config->ssid,device->ssid);
		strcpy(config->password,device->pass);
		wifi_station_set_config(config);
	
	if (!device->dhcpEn) {
//		if ((strlen(device->ssid)!=0)&&(device->ssid[0]!=0xff)&&(!device->dhcpEn))
//			conn = true;	//static ip
		wifi_station_dhcpc_stop();
		wifi_set_ip_info(STATION_IF, info);
	} 
	printf(" Station Ip: %d.%d.%d.%d\n",(info->ip.addr&0xff), ((info->ip.addr>>8)&0xff), ((info->ip.addr>>16)&0xff), ((info->ip.addr>>24)&0xff));
	printf("DHCP: 0x%x\nDevice: Ip: %d.%d.%d.%d\n",device->dhcpEn,device->ipAddr[0], device->ipAddr[1], device->ipAddr[2], device->ipAddr[3]);
	int i = 0;	
	printf("\nI: %d status: %d\n",i,wifi_station_get_connect_status());
	wifi_station_set_reconnect_policy(false);
	wifi_station_connect();
	while ((wifi_station_get_connect_status() != STATION_GOT_IP)&&(!conn))
	{	
		printf("\nIn I: %d status: %d\n",i,wifi_station_get_connect_status());
			FlashOn = FlashOff = 20;
//			while (wifi_station_get_connect_status() != STATION_GOT_IP) 
			{	
				vTaskDelay(100);// 100 ms
//				if (i++ >= 20) break; // 2 seconds
				i++;
			}	
			if (i >= 20)
			{
/*				printf("Config not found\nTrying smartconfig\n");
				FlashOn = FlashOff = 50;
				smartconfig_set_type(SC_TYPE_ESPTOUCH);
				smartconfig_start(cb);
				printf("smartConfig started. Waiting for ios or android 'ESP8266 SmartConfig' application\n");
				i = 0;
				while (status != SC_STATUS_LINK) 
				{	
					vTaskDelay(10); //100 ms
					if (i++ >= 100) break; // 100 seconds
					printf(".");
				}	
				if (i >= 100)*/
				{
					printf("\n");
//					smartconfig_stop();
					FlashOn = 10;FlashOff = 100;
					vTaskDelay(200);
					printf("Config not found\n");
					printf("\n");
					printf("The default AP is  WifiWebRadio. Connect your wifi to it.\nThen connect a webbrowser to 192.168.4.1 and go to Setting\n");
					printf("Erase the database and set ssid, password and ip's field\n");
					struct softap_config *apconfig;
					apconfig = malloc(sizeof(struct softap_config));
					wifi_set_opmode_current(SOFTAP_MODE);
					wifi_softap_get_config(apconfig);
					strcpy (apconfig->ssid,"WifiWebRadio");
					apconfig->ssid_len = 12;					
					wifi_softap_set_config(apconfig);
					conn = true; 
					free(apconfig);
					break;
				}
//				else smartconfig_stop();
			}// else {wifi_station_set_reconnect_policy(true);} // success

	}
	wifi_station_set_reconnect_policy(true);
	// update device info
	wifi_get_ip_info(STATION_IF, info);
	wifi_station_get_config(config);
	IPADDR2_COPY(&device->ipAddr, &info->ip);
	IPADDR2_COPY(&device->mask, &info->netmask);
	IPADDR2_COPY(&device->gate, &info->gw);
	strcpy(device->ssid,config->ssid);
	strcpy(device->pass,config->password);
	saveDeviceSettings(device);			

	free(info);
	free (device);
	free (config);
	
	FlashOn = 100;FlashOff = 10;	
	while(1) {
		while(1) {
			int c = uart_getchar_ms(100);
			if (c!= -1)
			{
				if((char)c == '\r') break;
				if((char)c == '\n') break;
				tmp[t] = (char)c;
				t++;
				if(t == 64) t = 0;
			}
			switchCommand() ;  // hardware panel of command
		}
		checkCommand(t, tmp);
/*	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	printf("watermark uartTask: %x  %d\n",uxHighWaterMark,uxHighWaterMark);
*/		
		for(t = 0; t<64; t++) tmp[t] = 0;
		t = 0;
//		vTaskDelay(20); // 250ms
	}
}

UART_SetBaudrate(uint8 uart_no, uint32 baud_rate) {
	uart_div_modify(uart_no, UART_CLK_FREQ / baud_rate);
}

void testtask(void* p) {
	gpio16_output_conf();
	while(1) {
		gpio16_output_set(0);
		vTaskDelay(FlashOff);
		gpio16_output_set(1);
		vTaskDelay(FlashOn);
	};
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
//	REG_SET_BIT(0x3ff00014, BIT(0));
//	system_update_cpu_freq(SYS_CPU_160MHZ);
//	system_update_cpu_freq(160); //- See more at: http://www.esp8266.com/viewtopic.php?p=8107#p8107
    Delay(300);
	UART_SetBaudrate(0,115200);
	wifi_set_opmode(STATION_MODE);
	Delay(100);	
	system_print_meminfo();
	printf ("Heap size: %d\n",xPortGetFreeHeapSize( ));
	clientInit();
	VS1053_HW_init();
	Delay(100);	
	TCP_WND = 2 * TCP_MSS;

	xTaskCreate(testtask, "t0", 80, NULL, 1, NULL); // DEBUG/TEST 80
	xTaskCreate(uartInterfaceTask, "t1", 265, NULL, 2, NULL); //240
	xTaskCreate(clientTask, "t3", 1024, NULL, 5, NULL); //1024
	xTaskCreate(serverTask, "t2", 180, NULL, 4, NULL); //200
	xTaskCreate(vsTask, "t4", 370, NULL,4, NULL); //370
}

