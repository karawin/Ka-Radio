/* (c)jp cocatrix May 2016 
 *
 * Copyright 2016 karawin (http://www.karawin.fr)

	quick and dirty telnet inplementation for wifi webradio
	minimal implementaion for log and command
*/

#include "telnet.h"
#include "interface.h"
#include <stdio.h>
#include <stdarg.h>


//const char strtMALLOC1[] = {"Telnet %s malloc fails\n"};
const char strtSOCKET[] STORE_ATTR ICACHE_RODATA_ATTR = {"Telnet Socket fails %s errno: %d\n"};
const char strtWELCOME[] STORE_ATTR ICACHE_RODATA_ATTR ={"Karadio telnet\n> "};


int telnetclients[NBCLIENTT];
//set of socket descriptors
fd_set readfds;
// reception buffer
char brec[256];
char *obrec;
uint16_t irec;

///////////////////////
// init some data
void telnetinit(void)
{
	int i;
	for (i = 0;i<NBCLIENTT;i++) 
	{
		telnetclients[i] = -1;
	}
	memset(brec,0,sizeof(brec));
	irec = 0;
	obrec = malloc(2);
}

/////////////////////////////////////////////////////////////////////
// a socket with a websocket request. Note it and answer to the client
bool telnetnewclient(int socket)
{
	int i ;
//	printf("ws newclient:%d\n",socket);
	for (i = 0;i<NBCLIENTT;i++) if (telnetclients[i] == socket) return true;
	else
	for (i = 0;i<NBCLIENTT;i++) if (telnetclients[i] == -1) 
	{
		telnetclients[i] = socket;
		return true;
	}	
	return false; // no more room
}
/////////////////////////////////////////////////////////////////////
// remove the client in the list of clients
void telnetremoveclient(int socket)
{
	int i ;
//	printf("ws removeclient:%d\n",socket);
	for (i = 0;i<NBCLIENTT;i++) 
		if (telnetclients[i] == socket) 
		{
			telnetclients[i] = -1;
//			printf("ws removeclient:%d removed\n",socket);
			close(socket);
			return;
		}
}
////////////////////////
// is socket a telnet one?
bool istelnet( int socket)
{
	int i ;
	for (i = 0;i<NBCLIENTT;i++) 
		if ((telnetclients[i]!= -1)&&(telnetclients[i] == socket)) return true;
	return false;
}


ICACHE_FLASH_ATTR bool telnetAccept(int tsocket)
{
int32_t recbytes = 0;

	if ((!istelnet(tsocket ))&&(telnetnewclient(tsocket))) 
	{
			char * fmt = malloc(strlen(strtWELCOME)+1);
			flashRead(fmt,strtWELCOME,strlen(strtWELCOME));
//			printf("telnet write accept\n");
			write(tsocket, fmt, strlen(strtWELCOME));  // reply to accept	
			free(fmt);
			return true;
	}
	else close(tsocket);
	return false;
}


//broadcast a txt data to all clients
void telnetWrite(uint32_t lenb,const char *fmt, ...)
{
	int i ;
	char *buf = NULL;
	char* lfmt;
	int len ,rlen;
	buf = (char *)malloc(lenb+1);
	if (buf == NULL) return;
	buf[0] = 0;
	strcpy(buf,"ok\n");
	
	va_list ap;
	va_start(ap, fmt);	
	
	if (fmt> (char*)0x40100000)  // in flash
	{
		len = strlen(fmt);
		lfmt = (char *)malloc(len+1);
		flashRead( lfmt, fmt, len );
		lfmt[len] = 0; // if aligned, trunkate
//		printf("lfmt: %s\n",lfmt);
		rlen = vsprintf(buf,lfmt, ap);
		free (lfmt);
	}	
	else 
	{
		rlen = vsprintf(buf,fmt, ap);		
	}
	va_end(ap);
	buf = realloc(buf,rlen+1);
	// write to all clients
	for (i = 0;i<NBCLIENTT;i++)	
		if (istelnet( telnetclients[i]))
		{
			write( telnetclients[i],  buf, strlen(buf));
		}	
		
	free (buf);

}	
ICACHE_FLASH_ATTR int telnetCommand(int tsocket)
{
	if (irec == 0) return;
//printf(PSTR("%sHEAPd0: %d #\n"),"##SYS.",xPortGetFreeHeapSize( ));	
	brec[irec] = 0x0;
	write(tsocket,"\n> ",1);
//	printf("%s\n",brec);
	obrec = realloc(obrec,strlen(brec)+1);
	strcpy(obrec,brec);
	checkCommand(irec, brec);
	write(tsocket,"> ",2);
	irec = 0;
}

ICACHE_FLASH_ATTR int telnetRead(int tsocket)
{
	char *buf ;
	int32_t recbytes ;
	int i;	
	buf = (char *)malloc(MAXDATAT);	
    if (buf == NULL)
	{
		vTaskDelay(100); // wait a while and retry
		buf = (char *)malloc(MAXDATAT);	
	}	
	if (buf != NULL)
	{
		recbytes = read(tsocket , buf, MAXDATAT);

		if (recbytes <= 0) {
			if ((errno != EAGAIN )&& (errno != ENOTCONN) &&(errno != 0 ))
			{
				if (errno != ECONNRESET )
				{
					printf (strtSOCKET,"read", errno);	
				} 
			} 
			free(buf);
			return 0; // free the socket
		}	

		buf = realloc(buf,recbytes+2);
//		printf(PSTR("%sHEAPdi1: %d #\nrecbytes: %d\n"),"##SYS.",xPortGetFreeHeapSize(),recbytes);	
		if (buf != NULL)
		{
			for (i = 0;i< recbytes;i++)
			{
				switch(buf[i]){
				case '\r':
				case '\n':
					telnetCommand(tsocket);
					break;
				case 0x08:	//backspace
				case 0x7F:	//delete
					if (irec >0) --irec;
					break;
				case 0x1B:
					if (i+2 <= recbytes)
					{
						if ((buf[i+1]=='[') && (buf[i+2]=='A')) // arrow up
						{
							strcpy(brec,obrec); 
							write(tsocket,"\r",1);
							write(tsocket,brec,strlen(brec));
							irec = strlen(brec);
							buf = realloc(buf,2);
							vTaskDelay(2);	
							telnetCommand(tsocket);
						}						
						i =recbytes; // exit for
					}
					break;
				default:
					brec[irec++] = buf[i];
					if (irec == sizeof(brec)) irec = 0;	
				}
			}	
			free(buf);	
		}		
	}
	return recbytes;
}



ICACHE_FLASH_ATTR void telnetTask(void* pvParams) {
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
	
	
/*	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	printf(PSTR("watermark wstask %d\n"),uxHighWaterMark);
	
*/

	int i;
	telnetinit();
	while(1)
	{
        bzero(&server_addr, sizeof(struct sockaddr_in));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(23);

        do {		
            if (-1 == (server_sock = socket(AF_INET, SOCK_STREAM, 0))) {
				printf (strtSOCKET,"create", errno);
				vTaskDelay(5);	
                break;
            }

            if (-1 == bind(server_sock, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr))) {
				printf (strtSOCKET,"Bind", errno);
				close(server_sock);
				vTaskDelay(10);	
                break;
            }

            if (-1 == listen(server_sock, 5)) {
				printf (strtSOCKET,"Listen",errno);
				close(server_sock);
				vTaskDelay(10);	
                break;
            }

            sin_size = sizeof(client_addr);	
			while (1)  //main loop
			{
				
				//clear the socket set
				FD_ZERO(&readfds);;
				
				//add server_sock to set
				FD_SET(server_sock, &readfds);
				max_sd = server_sock;
				
         				
				//add child sockets to set
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
					printf(strtSOCKET,"select",errno);
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
						printf (strtSOCKET,"accept",errno);
						close(client_sock);
						vTaskDelay(50);					
					} else
					{
						if (!telnetAccept(client_sock))
						{
							printf (strtSOCKET,"Accept1n",errno);
							close(client_sock);
							vTaskDelay(50);	
						}
					}
				} 	
				
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
//							printf(strtSOCKET,"Clear",errno); 
						}
						if (--activity ==0) break;;
					}
					vTaskDelay(1);
				} 
			}			

		} while (0);		
/*			uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
			printf(PSTR("watermark middle wsTask: %d\n"),uxHighWaterMark);
					
*/
	} 

//	printf("telnet task exit\n");
/*
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	printf(PSTR("watermark end wsTask: %x  %d\n"),uxHighWaterMark,uxHighWaterMark);
*/	
	vTaskDelete( NULL );	
}
