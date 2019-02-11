/* (c)jp cocatrix May 2017
 *
 * Copyright 2017 karawin (http://www.karawin.fr)

	Main task for the web websocket and telnet servers.
*/

#include "telnet.h"
#include "websocket.h"
#include "webserver.h"
#include "interface.h"
#include <stdio.h>
#include <stdarg.h>

#define stack  500

const char strsTELNET[] ICACHE_RODATA_ATTR STORE_ATTR  = {"Servers Telnet Socket fails %s errno: %d\n"};
const char strsWEB[] ICACHE_RODATA_ATTR STORE_ATTR  = {"Servers Web Socket fails %s errno: %d\n"};
const char strsWSOCK[] ICACHE_RODATA_ATTR STORE_ATTR  = {"WebServer Socket fails %s errno: %d\n"};

fd_set readfds;
xSemaphoreHandle semclient = NULL ;

const char strsocket[] = {"Socket"};
const char strbind[] = {"Bind"};
const char strlisten[] = {"Listen"};

ICACHE_FLASH_ATTR void serversTask(void* pvParams) {
	//telnet
	struct sockaddr_in tenetserver_addr, tenetclient_addr;
	int telnetServer_sock, telnetClient_sock;
	int server_sock;
	socklen_t telnetSin_size;
	//web
	struct sockaddr_in server_addr, client_addr;
	int  client_sock;
	socklen_t sin_size;
	os_timer_disarm(&sleepTimer);
	os_timer_disarm(&wakeTimer);
	os_timer_setfn(&sleepTimer, sleepCallback, NULL);
	os_timer_setfn(&wakeTimer, wakeCallback, NULL);
	semclient = xSemaphoreCreateCounting(3,3); 
	semfile = xSemaphoreCreateCounting(1,1); 
	
//	portBASE_TYPE uxHighWaterMark;
	

	struct timeval timeout;      
    timeout.tv_sec = 0; // bug *1000 for seconds
    timeout.tv_usec = 0;
	
	int max_sd;
	int activity;
	int	ret, sd;	
	
/*	
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	printf(PSTR("watermark serverstask %d\n"),uxHighWaterMark);
*/	


	int i;
	telnetinit();
	websocketinit();
	while(1)
	{
/////////////////////		
// telnet socket init
/////////////////////
		bzero(&tenetserver_addr, sizeof(struct sockaddr_in));
        tenetserver_addr.sin_family = AF_INET;
        tenetserver_addr.sin_addr.s_addr = INADDR_ANY;
        tenetserver_addr.sin_port = htons(23);
		
        if (-1 == (telnetServer_sock = socket(AF_INET, SOCK_STREAM, 0))) {
			printf (strsTELNET,strsocket, errno);
			vTaskDelay(5);	
            break;
        }
        if (-1 == bind(telnetServer_sock, (struct sockaddr *)(&tenetserver_addr), sizeof(struct sockaddr))) {
			printf (strsTELNET,strbind, errno);
			close(telnetServer_sock);
			vTaskDelay(10);	
            break;
        }
        if (-1 == listen(telnetServer_sock, 5)) {
			printf (strsTELNET,strlisten,errno);
			close(telnetServer_sock);
			vTaskDelay(10);	
            break;
        }
        telnetSin_size = sizeof(tenetclient_addr);	
////////////////////////		
// telnet socket init end
////////////////////////


////////////////////////
// webserver socket init
////////////////////////
		bzero(&server_addr, sizeof(struct sockaddr_in));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		server_addr.sin_port = htons(80);

		if (-1 == (server_sock = socket(AF_INET, SOCK_STREAM, 0))) {
			printf (strsWSOCK, strsocket, errno);
			vTaskDelay(5);	
			break;
		}
		if (-1 == bind(server_sock, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr))) {
			printf (strsWSOCK, strbind,errno);
			close(server_sock);
			vTaskDelay(10);	
            break;
		}
		if (-1 == listen(server_sock, 5)) {
			printf (strsWSOCK,strlisten,errno);
			close(server_sock);
			vTaskDelay(10);	
			break;
		}
		sin_size = sizeof(client_addr);
/////////////////////////////		
// webserver socket init end
////////////////////////////
		
		while (1)  //main loop
		{
			
			//clear the socket set
			FD_ZERO(&readfds);;
			
			//add server_sock to set (webserver)
			FD_SET(server_sock, &readfds);
			max_sd = server_sock ;  
				

			//add telnetServer_sock to set (telnet)
			FD_SET(telnetServer_sock, &readfds);
			max_sd = telnetServer_sock > max_sd ? telnetServer_sock : max_sd;  


			//add child sockets to set (wssocket)
			for (i = 0;i<NBCLIENT;i++) 
			{
				sd = webserverclients[i].socket;
				//if valid socket descriptor then add to read list
				if(sd != -1)
				{	
					FD_SET( sd , &readfds);   
//					printf("wssocket SD_set %d\n",sd);
					//highest file descriptor number, need it for the select function
					max_sd = sd > max_sd ? sd : max_sd;
				}				
			}
			
			//add child sockets to set (telnet)
			for (i = 0;i<NBCLIENTT;i++) 
			{
				sd = telnetclients[i];
				//if valid socket descriptor then add to read list
				if(sd != -1)
				{	
					FD_SET( sd , &readfds);   
//					printf("SD_set %d, max_sd: %d\n",sd,max_sd);
					//highest file descriptor number, need it for the select function
					max_sd = sd > max_sd ? sd : max_sd;
				}				
			}	
			
//			printf("ws call select. Max sd: %d\n",max_sd);

			//wait for an activity on one of the sockets , 
			activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
//			if (activity != 0) printf ("Activity %d, max_fd: %d\n",activity,max_sd);
   
			if ((activity < 0) && (errno!=EINTR) && (errno!=0)) 
			{
				printf(strsTELNET,"select",errno);
				vTaskDelay(100);
				continue;
			}	
			if (activity == 0)	{vTaskDelay(10);continue;}	
	
//If something happened on the master webserver socket , then its an incoming connection
			if (FD_ISSET(server_sock, &readfds)) 
			{	
				FD_CLR(server_sock , &readfds); 
//printf(PSTR("Server web accept.%c"),0x0d);				
				if ((client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &sin_size)) < 0) {
						printf (strsWSOCK,"accept",errno);
						vTaskDelay(10);					
				} else
				{
					while (1) 
					{
//						printf ("Heap size server: %d\n",xPortGetFreeHeapSize( ));
//						printf ("Accept socket %d\n",client_sock);
						if (xSemaphoreTake(semclient,portMAX_DELAY))
						{
//printf (PSTR("Take client_sock: %d\n%c"),client_sock,0x0d);							
							while (xTaskCreate( serverclientTask,
								"t10",
								stack,
								(void *) client_sock,
								4, 
								NULL ) != pdPASS) 
							{								
								kprintf(PSTR("Low mem. Retrying...\n%c"),0x0d);
								vTaskDelay(200);
							}	
							//vTaskDelay(1);	
//printf(PSTR("serverClient launched.\n%c"),0x0d);							
//							xSemaphoreGive(semclient);	
							break; // while 1
						}
						else  // xSemaphoreTake fails
						{
							vTaskDelay(200); 
							kprintf(PSTR("Busy. Retrying...\n%c"),0x0d);
						}
						
					}
				}					
			}
			
			
//If something happened on the master telnet socket , then its an incoming connection
			if (FD_ISSET(telnetServer_sock, &readfds)) 
			{
				FD_CLR(telnetServer_sock , &readfds);  				
				if ((telnetClient_sock = accept(telnetServer_sock, (struct sockaddr *) &tenetclient_addr, &telnetSin_size)) < 0) 
				{
					printf (strsTELNET,"accept",errno);
					close(telnetClient_sock);
					vTaskDelay(50);					
				} else
				{
					if (!telnetAccept(telnetClient_sock))
					{
						printf (strsTELNET,"Accept1n",errno);
						close(telnetClient_sock);
						vTaskDelay(50);	
					}
				}
			} 			
						
// telnet sockets				
			for (i = 0; i < NBCLIENTT; i++) 
			{
				sd = telnetclients[i];
             
				if ((sd!=-1) &&(FD_ISSET( sd , &readfds))) 
				{
					FD_CLR(sd , &readfds);  
					ret =telnetRead(sd);
					//printf("Call telnetRead i: %d, socket: %d, ret: %d\n" ,i, sd,ret);  
					if (ret == 0) 
					{
						telnetremoveclient(sd);						
//						printf(strsTELNET,"Clear",errno); 
					}
//					if (--activity ==0) break;
				}
//				vTaskDelay(1);
			} 
			
			
// websocket sockets				
			for (i = 0; i < NBCLIENT; i++) 
			{
				sd = webserverclients[i].socket;
             
				if ((sd!=-1) &&(FD_ISSET( sd , &readfds))) 
				{
					FD_CLR(sd , &readfds);  
					ret =websocketRead(sd);
//					printf("Call websocketRead i: %d, socket: %d, ret: %d\n" ,i, sd,ret);  
					if (ret <= 0) 
					{
						websocketremoveclient(sd);						
						//printf("Clear i: %d, socket: %d, errno: %d\n" ,i, sd,errno); 
					}
//					if (--activity ==0) break;
				}
			}    				
			
/*			uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
			printf(PSTR("watermark middle ssTask: %d\n"),uxHighWaterMark);
*/			
		}			
					
	} 
	printf(PSTR("telnet task abnormal exit\n"));
	vTaskDelete( NULL );	// never called
}
