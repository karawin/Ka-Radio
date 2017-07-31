/* (c)jp cocatrix May 2017
 *
 * Copyright 2017 karawin (http://www.karawin.fr)

	Main task for the websocket and telnet servers.
*/

#include "telnet.h"
#include "websocket.h"
#include "interface.h"
#include <stdio.h>
#include <stdarg.h>

const char strsSOCKET[] STORE_ATTR ICACHE_RODATA_ATTR = {"Servers Socket fails %s errno: %d\n"};

fd_set readfds;

ICACHE_FLASH_ATTR void serversTask(void* pvParams) {
	struct sockaddr_in server_addr, client_addr;
	int server_sock, client_sock;
	socklen_t sin_size;
	portBASE_TYPE uxHighWaterMark;
	

	struct timeval timeout;      
    timeout.tv_sec = 2; // bug *1000 for seconds
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
        bzero(&server_addr, sizeof(struct sockaddr_in));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(23);

        do {		
            if (-1 == (server_sock = socket(AF_INET, SOCK_STREAM, 0))) {
				printf (strsSOCKET,"create", errno);
				vTaskDelay(5);	
                break;
            }

            if (-1 == bind(server_sock, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr))) {
				printf (strsSOCKET,"Bind", errno);
				close(server_sock);
				vTaskDelay(10);	
                break;
            }

            if (-1 == listen(server_sock, 5)) {
				printf (strsSOCKET,"Listen",errno);
				close(server_sock);
				vTaskDelay(10);	
                break;
            }

            sin_size = sizeof(client_addr);	
			while (1)  //main loop
			{
				
				//clear the socket set
				FD_ZERO(&readfds);;
				
				//add server_sock to set (telnet
				FD_SET(server_sock, &readfds);
				max_sd = server_sock;
//
				
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
//			
				//add child sockets to set (telnet)
				for (i = 0;i<NBCLIENTT;i++) 
				{
					sd = telnetclients[i];
					//if valid socket descriptor then add to read list
					if(sd != -1)
					{	
						FD_SET( sd , &readfds);   
//						printf("SD_set %d, max_sd: %d\n",sd,max_sd);
						//highest file descriptor number, need it for the select function
						max_sd = sd > max_sd ? sd : max_sd;
					}				
				}	
				
//				printf("ws call select. Max sd: %d\n",max_sd);





				//wait for an activity on one of the sockets , 
				activity = select( max_sd + 1 , &readfds , NULL , NULL ,  &timeout);
//				if (activity != 0) printf ("Activity %d, max_fd: %d\n",activity,max_sd);
    
				if ((activity < 0) && (errno!=EINTR) && (errno!=0)) 
				{
					printf(strsSOCKET,"select",errno);
					vTaskDelay(100);
					continue;
				}	
				if (activity == 0)	{continue;}	

				//If something happened on the master socket , then its an incoming connection
				if (FD_ISSET(server_sock, &readfds)) 
				{
					FD_CLR(server_sock , &readfds);  				
					if ((client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &sin_size)) < 0) 
					{
						printf (strsSOCKET,"accept",errno);
						close(client_sock);
						vTaskDelay(50);					
					} else
					{
						if (!telnetAccept(client_sock))
						{
							printf (strsSOCKET,"Accept1n",errno);
							close(client_sock);
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
//							printf(strsSOCKET,"Clear",errno); 
						}
						if (--activity ==0) break;
					}
//					vTaskDelay(1);
				} 
				
				
// websocket sockets				
				for (i = 0; i < NBCLIENT; i++) 
				{
					sd = webserverclients[i].socket;
              
					if ((sd!=-1) &&(FD_ISSET( sd , &readfds))) 
					{
						FD_CLR(sd , &readfds);  
						ret =websocketRead(sd);
//						printf("Call websocketRead i: %d, socket: %d, ret: %d\n" ,i, sd,ret);  
						if (ret <= 0) 
						{
							websocketremoveclient(sd);						
							//printf("Clear i: %d, socket: %d, errno: %d\n" ,i, sd,errno); 
						}
						if (--activity ==0) break;
					}
				}    				
				
/*			uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
			printf(PSTR("watermark middle ssTask: %d\n"),uxHighWaterMark);
*/			
			}			

		} while (0);		
					

	} 

//	printf("telnet task exit\n");
/*
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	printf(PSTR("watermark end ssTask: %x  %d\n"),uxHighWaterMark,uxHighWaterMark);
*/	
	vTaskDelete( NULL );	
}
