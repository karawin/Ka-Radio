/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
/*
 * Copyright 2016 jp Cocatrix (http://www.karawin.fr)
*/

#include "esp_common.h"
#include "esp_system.h"
#include "upgrade.h"
#include "math.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#define pheadbuffer "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36 \r\n\
Accept: */*\r\n\
Accept-Encoding: gzip,deflate,sdch\r\n\
\r\n"

#define pheadbuffer1 "Connection: close\r\n\
Accept: */*\r\n\
Accept-Encoding: gzip,deflate,sdch\r\n\
Cache-Control: no-cache\r\n\
\r\n"

/******************************************************************************
 * FunctionName : user_esp_upgrade_rsp
 * Description  : upgrade check call back function
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_esp_upgrade_rsp(void *arg)
{
	struct upgrade_server_info *server = (struct upgrade_server_info *)arg;
	if(server->upgrade_flag == true){
		printf(" +OK: FW upgrade success.\n");
		system_upgrade_reboot();
	} else {
		printf("-ERR: FW upgrade failed.\n");
	}
    free(server->url);
    server->url = NULL;
	
    free(server);
    server = NULL;	
}

/******************************************************************************
 * FunctionName : update_firmware
 * Description  : config remote server param and init bin info
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void update_firmware(void)
{
    printf("update  firmware\r\n");
    uint8 current_id = system_upgrade_userbin_check();
    os_printf("current id %d\n", current_id);

	char* client_url = "karadio.karawin.fr";
//	char* client_url = "192.168.1.2";
#define bin_url  "user%d.4096.new.4.bin"
    struct upgrade_server_info *server = NULL;
    server = (struct upgrade_server_info *)malloc(sizeof(struct upgrade_server_info));
    memset(server, 0, sizeof(struct upgrade_server_info));
    server->check_cb = user_esp_upgrade_rsp;   //set upgrade check call back function
    server->check_times = 360000;
	server->upgrade_flag = true;
	
    if (server->url == NULL) {
        server->url = (uint8 *)malloc(512);
    }

    flash_size_map f_size = system_get_flash_size_map();
    os_printf("flash size  %d\n",f_size);
    if (current_id == 0) {
		 current_id = 2;
	} else current_id = 1;
    sprintf(server->url, "GET /"bin_url" HTTP/1.0\r\nHost: %s:80\r\n"pheadbuffer"",current_id, client_url);
//   os_printf("http_req : %s\n", server->url);
	
	struct hostent *serv ;
	serv =(struct hostent*)gethostbyname(client_url);
    server->sockaddrin.sin_family = AF_INET;
    server->sockaddrin.sin_port   = htons(80);
    server->sockaddrin.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)(serv-> h_addr_list[0]))); // remote server ip
//	os_printf("distant ip: %x   ADDR:%s\n",server->sockaddrin.sin_addr.s_addr,inet_ntoa(*(struct in_addr*)(serv-> h_addr_list[0])));
    if (system_upgrade_start(server) == false) {
        os_printf("upgrade error\n");
    }
	

}
