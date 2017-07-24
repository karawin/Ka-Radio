/******************************************************************************
 * Copyright (C) 2014 -2016  Espressif System
 *
 * FileName: user_upgrade.c
 *
 * Description: downlaod upgrade userbin file from upgrade server
 *
 * Modification history:
 * 2015/7/3, v1.0 create this file.
*******************************************************************************/
//#include "version.h"
//#include "user_config.h"

#include "esp_common.h"
#include "lwip/mem.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "upgrade.h"
#include "ssl/ssl_ssl.h"


/*the size cannot be bigger than below*/
#define UPGRADE_DATA_SEG_LEN 1460
#define UPGRADE_RETRY_TIMES 10

LOCAL os_timer_t upgrade_10s;
LOCAL uint32 totallength = 0;
LOCAL uint32 sumlength = 0;
LOCAL BOOL flash_erased=0;
LOCAL BOOL giveup =0;

char *precv_buf=NULL;
os_timer_t upgrade_timer;
xTaskHandle *pxCreatedTask=NULL;

#ifdef UPGRADE_SSL_ENABLE
//#include "ssl/cert.h"
//#include "ssl/private_key.h"
unsigned char *default_certificate;
unsigned int default_certificate_len = 0;
unsigned char *default_private_key;
unsigned int default_private_key_len = 0;
#endif
/******************************************************************************
 * FunctionName : upgrade_deinit
 * Description  :
 * Parameters   :
 * Returns      : none
*******************************************************************************/
void  
LOCAL upgrade_deinit(void)
{
    if (system_upgrade_flag_check() != UPGRADE_FLAG_START) {
        system_upgrade_deinit();
        //system_upgrade_reboot();
    }
}

/******************************************************************************
 * FunctionName : upgrade_data_load
 * Description  : parse the data from server,send fw data to system interface 
 * Parameters   : pusrdata--data from server,
 *              : length--length of the pusrdata
 * Returns      : none
 *  
 * first data from server:
 * HTTP/1.1 200 OK
 * Server: nginx/1.6.2
 * Date: Tue, 14 Jul 2015 09:15:51 GMT
 * Content-Type: application/octet-stream
 * Content-Length: 282448
 * Connection: keep-alive
 * Content-Disposition: attachment;filename=user2.bin
 * Vary: Cookie
 * X-RateLimit-Remaining: 3599
 * X-RateLimit-Limit: 3600
 * X-RateLimit-Reset: 1436866251
*******************************************************************************/
BOOL upgrade_data_load(char *pusrdata, unsigned short length)
{
    char *ptr = NULL;
    char *ptmp2 = NULL;
    char lengthbuffer[32];

    
    if (totallength == 0 && (ptr = (char *)strstr(pusrdata, "\r\n\r\n")) != NULL &&
            (ptr = (char *)strstr(pusrdata, "Content-Length")) != NULL) {

 //       os_printf("\n pusrdata %s\n",pusrdata);

        ptr = (char *)strstr(pusrdata, "Content-Length: ");
        if (ptr != NULL) {
            ptr += 16;
            ptmp2 = (char *)strstr(ptr, "\r\n");

            if (ptmp2 != NULL) {
                memset(lengthbuffer, 0, sizeof(lengthbuffer));
                
                if((ptmp2 - ptr)<=32)
                     memcpy(lengthbuffer, ptr, ptmp2 - ptr);
                else
                     os_printf("ERR1:arr_overflow,%u,%d\n",__LINE__,(ptmp2 - ptr));
                
                sumlength = atoi(lengthbuffer);
                os_printf("userbin sumlength:%d \n",sumlength);
                
                ptr = (char *)strstr(pusrdata, "\r\n\r\n");
                length -= ptr - pusrdata;
                length -= 4;
                totallength += length;

                /*at the begining of the upgrade,we get the sumlength 
                 *and erase all the target flash sectors,return false
                 *to close the connection, and start upgrade again.  
                 */
                if(FALSE==flash_erased){
//					os_printf("userbin sumlength:%d  flash:%d\n",sumlength,flash_erased);
                    flash_erased=system_upgrade(ptr + 4, sumlength);
					os_printf("userbin sumlength:%d  flash:%d\n",sumlength,flash_erased);
                    return flash_erased;
                }else{
                    system_upgrade(ptr + 4, length);
                }
            } else {
                os_printf("ERROR:Get sumlength failed\n");
                return false;
            }
        } else {
            os_printf("ERROR:Get Content-Length failed\n");
            return false;
        }
        
    } 
    else {
        if(totallength != 0){
            totallength += length;
            
            if(totallength > sumlength){
                os_printf("strip the 400 error mesg\n");
                length =length -(totallength- sumlength);
            }
            
//            os_printf(">>>recv %dB, %dB left\n",totallength,sumlength-totallength);
            system_upgrade(pusrdata, length);
            
        } else {
            os_printf("server response with something else,check it!\n");
            return false;
        }
    }

    return true;
}
#ifdef UPGRADE_SSL_ENABLE

static const char default_certificate[] ICACHE_RODATA_ATTR STORE_ATTR = {
  0x30, 0x82, 0x03, 0x54, 0x30, 0x82, 0x02, 0x3c, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x03, 0x02, 0x34, 0x56, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86,
  0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x30, 0x42, 0x31,
  0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53,
  0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x0d, 0x47,
  0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x49, 0x6e, 0x63, 0x2e,
  0x31, 0x1b, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x12, 0x47,
  0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x47, 0x6c, 0x6f, 0x62,
  0x61, 0x6c, 0x20, 0x43, 0x41, 0x30, 0x1e, 0x17, 0x0d, 0x30, 0x32, 0x30,
  0x35, 0x32, 0x31, 0x30, 0x34, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d,
  0x32, 0x32, 0x30, 0x35, 0x32, 0x31, 0x30, 0x34, 0x30, 0x30, 0x30, 0x30,
  0x5a, 0x30, 0x42, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06,
  0x13, 0x02, 0x55, 0x53, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04,
  0x0a, 0x13, 0x0d, 0x47, 0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20,
  0x49, 0x6e, 0x63, 0x2e, 0x31, 0x1b, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04,
  0x03, 0x13, 0x12, 0x47, 0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20,
  0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x20, 0x43, 0x41, 0x30, 0x82, 0x01,
  0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
  0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01,
  0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xda, 0xcc, 0x18, 0x63, 0x30, 0xfd,
  0xf4, 0x17, 0x23, 0x1a, 0x56, 0x7e, 0x5b, 0xdf, 0x3c, 0x6c, 0x38, 0xe4,
  0x71, 0xb7, 0x78, 0x91, 0xd4, 0xbc, 0xa1, 0xd8, 0x4c, 0xf8, 0xa8, 0x43,
  0xb6, 0x03, 0xe9, 0x4d, 0x21, 0x07, 0x08, 0x88, 0xda, 0x58, 0x2f, 0x66,
  0x39, 0x29, 0xbd, 0x05, 0x78, 0x8b, 0x9d, 0x38, 0xe8, 0x05, 0xb7, 0x6a,
  0x7e, 0x71, 0xa4, 0xe6, 0xc4, 0x60, 0xa6, 0xb0, 0xef, 0x80, 0xe4, 0x89,
  0x28, 0x0f, 0x9e, 0x25, 0xd6, 0xed, 0x83, 0xf3, 0xad, 0xa6, 0x91, 0xc7,
  0x98, 0xc9, 0x42, 0x18, 0x35, 0x14, 0x9d, 0xad, 0x98, 0x46, 0x92, 0x2e,
  0x4f, 0xca, 0xf1, 0x87, 0x43, 0xc1, 0x16, 0x95, 0x57, 0x2d, 0x50, 0xef,
  0x89, 0x2d, 0x80, 0x7a, 0x57, 0xad, 0xf2, 0xee, 0x5f, 0x6b, 0xd2, 0x00,
  0x8d, 0xb9, 0x14, 0xf8, 0x14, 0x15, 0x35, 0xd9, 0xc0, 0x46, 0xa3, 0x7b,
  0x72, 0xc8, 0x91, 0xbf, 0xc9, 0x55, 0x2b, 0xcd, 0xd0, 0x97, 0x3e, 0x9c,
  0x26, 0x64, 0xcc, 0xdf, 0xce, 0x83, 0x19, 0x71, 0xca, 0x4e, 0xe6, 0xd4,
  0xd5, 0x7b, 0xa9, 0x19, 0xcd, 0x55, 0xde, 0xc8, 0xec, 0xd2, 0x5e, 0x38,
  0x53, 0xe5, 0x5c, 0x4f, 0x8c, 0x2d, 0xfe, 0x50, 0x23, 0x36, 0xfc, 0x66,
  0xe6, 0xcb, 0x8e, 0xa4, 0x39, 0x19, 0x00, 0xb7, 0x95, 0x02, 0x39, 0x91,
  0x0b, 0x0e, 0xfe, 0x38, 0x2e, 0xd1, 0x1d, 0x05, 0x9a, 0xf6, 0x4d, 0x3e,
  0x6f, 0x0f, 0x07, 0x1d, 0xaf, 0x2c, 0x1e, 0x8f, 0x60, 0x39, 0xe2, 0xfa,
  0x36, 0x53, 0x13, 0x39, 0xd4, 0x5e, 0x26, 0x2b, 0xdb, 0x3d, 0xa8, 0x14,
  0xbd, 0x32, 0xeb, 0x18, 0x03, 0x28, 0x52, 0x04, 0x71, 0xe5, 0xab, 0x33,
  0x3d, 0xe1, 0x38, 0xbb, 0x07, 0x36, 0x84, 0x62, 0x9c, 0x79, 0xea, 0x16,
  0x30, 0xf4, 0x5f, 0xc0, 0x2b, 0xe8, 0x71, 0x6b, 0xe4, 0xf9, 0x02, 0x03,
  0x01, 0x00, 0x01, 0xa3, 0x53, 0x30, 0x51, 0x30, 0x0f, 0x06, 0x03, 0x55,
  0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff,
  0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0xc0,
  0x7a, 0x98, 0x68, 0x8d, 0x89, 0xfb, 0xab, 0x05, 0x64, 0x0c, 0x11, 0x7d,
  0xaa, 0x7d, 0x65, 0xb8, 0xca, 0xcc, 0x4e, 0x30, 0x1f, 0x06, 0x03, 0x55,
  0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0xc0, 0x7a, 0x98, 0x68,
  0x8d, 0x89, 0xfb, 0xab, 0x05, 0x64, 0x0c, 0x11, 0x7d, 0xaa, 0x7d, 0x65,
  0xb8, 0xca, 0xcc, 0x4e, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
  0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00,
  0x35, 0xe3, 0x29, 0x6a, 0xe5, 0x2f, 0x5d, 0x54, 0x8e, 0x29, 0x50, 0x94,
  0x9f, 0x99, 0x1a, 0x14, 0xe4, 0x8f, 0x78, 0x2a, 0x62, 0x94, 0xa2, 0x27,
  0x67, 0x9e, 0xd0, 0xcf, 0x1a, 0x5e, 0x47, 0xe9, 0xc1, 0xb2, 0xa4, 0xcf,
  0xdd, 0x41, 0x1a, 0x05, 0x4e, 0x9b, 0x4b, 0xee, 0x4a, 0x6f, 0x55, 0x52,
  0xb3, 0x24, 0xa1, 0x37, 0x0a, 0xeb, 0x64, 0x76, 0x2a, 0x2e, 0x2c, 0xf3,
  0xfd, 0x3b, 0x75, 0x90, 0xbf, 0xfa, 0x71, 0xd8, 0xc7, 0x3d, 0x37, 0xd2,
  0xb5, 0x05, 0x95, 0x62, 0xb9, 0xa6, 0xde, 0x89, 0x3d, 0x36, 0x7b, 0x38,
  0x77, 0x48, 0x97, 0xac, 0xa6, 0x20, 0x8f, 0x2e, 0xa6, 0xc9, 0x0c, 0xc2,
  0xb2, 0x99, 0x45, 0x00, 0xc7, 0xce, 0x11, 0x51, 0x22, 0x22, 0xe0, 0xa5,
  0xea, 0xb6, 0x15, 0x48, 0x09, 0x64, 0xea, 0x5e, 0x4f, 0x74, 0xf7, 0x05,
  0x3e, 0xc7, 0x8a, 0x52, 0x0c, 0xdb, 0x15, 0xb4, 0xbd, 0x6d, 0x9b, 0xe5,
  0xc6, 0xb1, 0x54, 0x68, 0xa9, 0xe3, 0x69, 0x90, 0xb6, 0x9a, 0xa5, 0x0f,
  0xb8, 0xb9, 0x3f, 0x20, 0x7d, 0xae, 0x4a, 0xb5, 0xb8, 0x9c, 0xe4, 0x1d,
  0xb6, 0xab, 0xe6, 0x94, 0xa5, 0xc1, 0xc7, 0x83, 0xad, 0xdb, 0xf5, 0x27,
  0x87, 0x0e, 0x04, 0x6c, 0xd5, 0xff, 0xdd, 0xa0, 0x5d, 0xed, 0x87, 0x52,
  0xb7, 0x2b, 0x15, 0x02, 0xae, 0x39, 0xa6, 0x6a, 0x74, 0xe9, 0xda, 0xc4,
  0xe7, 0xbc, 0x4d, 0x34, 0x1e, 0xa9, 0x5c, 0x4d, 0x33, 0x5f, 0x92, 0x09,
  0x2f, 0x88, 0x66, 0x5d, 0x77, 0x97, 0xc7, 0x1d, 0x76, 0x13, 0xa9, 0xd5,
  0xe5, 0xf1, 0x16, 0x09, 0x11, 0x35, 0xd5, 0xac, 0xdb, 0x24, 0x71, 0x70,
  0x2c, 0x98, 0x56, 0x0b, 0xd9, 0x17, 0xb4, 0xd1, 0xe3, 0x51, 0x2b, 0x5e,
  0x75, 0xe8, 0xd5, 0xd0, 0xdc, 0x4f, 0x34, 0xed, 0xc2, 0x05, 0x66, 0x80,
  0xa1, 0xcb, 0xe6, 0x33
};
static unsigned int default_certificate_len = 856;

/**
 * Display what session id we have.
 */
static void   display_session_id(SSL *ssl)
{
    int i;
    const uint8_t *session_id = ssl_get_session_id(ssl);
    int sess_id_size = ssl_get_session_id_size(ssl);

    if (sess_id_size > 0) {
        printf("-----BEGIN SSL SESSION PARAMETERS-----\n");

        for (i = 0; i < sess_id_size; i++) {
            printf("%02x", session_id[i]);
        }

        printf("\n-----END SSL SESSION PARAMETERS-----\n");
    }
}

/**
 * Display what cipher we are using
 */
static void   display_cipher(SSL *ssl)
{
    printf("CIPHER is ");

    switch (ssl_get_cipher_id(ssl)) {
        case SSL_AES128_SHA:
            printf("AES128-SHA");
            break;

        case SSL_AES256_SHA:
            printf("AES256-SHA");
            break;

        case SSL_RC4_128_SHA:
            printf("RC4-SHA");
            break;

        case SSL_RC4_128_MD5:
            printf("RC4-MD5");
            break;

        default:
            printf("Unknown - %d", ssl_get_cipher_id(ssl));
            break;
    }

    printf("\n");
}

/******************************************************************************
 * FunctionName : upgrade_task
 * Description  : task to connect with target server and get firmware data 
 * Parameters   : pvParameters--save the server address\port\request frame for
 *              : the upgrade server\call back functions to tell the userapp
 *              : the result of this upgrade task
 * Returns      : none
*******************************************************************************/
void upgrade_ssl_task(void *pvParameters)
{
    int recbytes;
    int sta_socket;
    int retry_count = 0;
    struct ip_info ipconfig;
    
    struct upgrade_server_info *server = pvParameters;

    flash_erased=FALSE;
    precv_buf = (char*)malloc(UPGRADE_DATA_SEG_LEN);//the max data length
    
    while (retry_count++ < UPGRADE_RETRY_TIMES) {
        if (giveup) break;
		
        wifi_get_ip_info(STATION_IF, &ipconfig);

        /* check the ip address or net connection state*/
        while (ipconfig.ip.addr == 0) {
            vTaskDelay(1000 / portTICK_RATE_MS);
            wifi_get_ip_info(STATION_IF, &ipconfig);
        }
        
        sta_socket = socket(PF_INET,SOCK_STREAM,0);
        if (-1 == sta_socket) {
            close(sta_socket);
            vTaskDelay(1000 / portTICK_RATE_MS);
            os_printf("socket fail !\r\n");
            continue;
        }

        /*for upgrade connection debug*/
        //server->sockaddrin.sin_addr.s_addr= inet_addr("192.168.1.170");
        if(0 != connect(sta_socket,(struct sockaddr *)(&server->sockaddrin),sizeof(struct sockaddr))) {
            close(sta_socket);
            vTaskDelay(1000 / portTICK_RATE_MS);
            os_printf("connect fail!\r\n");
            continue;
        }

        uint32_t options = SSL_DISPLAY_CERTS | SSL_NO_DEFAULT_KEY;
        int i=0;
        int quiet = 0;
        int cert_index = 0, ca_cert_index = 0;
        int cert_size, ca_cert_size;
        char **ca_cert, **cert;
        SSL *ssl;
        SSL_CTX *ssl_ctx;
        uint8_t *read_buf = NULL;

        cert_size = ssl_get_config(SSL_MAX_CERT_CFG_OFFSET);
        ca_cert_size = ssl_get_config(SSL_MAX_CA_CERT_CFG_OFFSET);
        ca_cert = (char **)calloc(1, sizeof(char *)*ca_cert_size);
        cert = (char **)calloc(1, sizeof(char *)*cert_size);

        if ((ssl_ctx= ssl_ctx_new(options, SSL_DEFAULT_CLNT_SESS)) == NULL) {
            printf("Error: Client context is invalid\n");
            close(sta_socket);
            continue;
        }

		ssl_obj_memory_load(ssl_ctx, SSL_OBJ_X509_CACERT, default_certificate, default_certificate_len, NULL);

        for (i = 0; i < cert_index; i++) {
            if (ssl_obj_load(ssl_ctx, SSL_OBJ_X509_CERT, cert[i], NULL)){
                printf("Certificate '%s' is undefined.\n", cert[i]);
            }
        }
        
        for (i = 0; i < ca_cert_index; i++) {
            if (ssl_obj_load(ssl_ctx, SSL_OBJ_X509_CACERT, ca_cert[i], NULL)){
                printf("Certificate '%s' is undefined.\n", ca_cert[i]);
            }
        }

        free(cert);
        free(ca_cert);

        ssl= ssl_client_new(ssl_ctx, sta_socket, NULL, 0);
        if (ssl == NULL){
            ssl_ctx_free(ssl_ctx);
            close(sta_socket);
            continue;
        }
        
        if(ssl_handshake_status(ssl) != SSL_OK){
            printf("client handshake fail.\n");
            ssl_free(ssl);
            ssl_ctx_free(ssl_ctx);
            close(sta_socket);
            continue;
        }
        
        //handshake sucesses,show cert and free x509_ctx here
        if (!quiet) {
            const char *common_name = ssl_get_cert_dn(ssl,SSL_X509_CERT_COMMON_NAME);
            if (common_name) {
                printf("Common Name:\t\t\t%s\n", common_name);
            }
            display_session_id(ssl);
            display_cipher(ssl);
            quiet = true;

            x509_free(ssl->x509_ctx);
            ssl->x509_ctx=NULL;
        }

        system_upgrade_init();
        system_upgrade_flag_set(UPGRADE_FLAG_START);

        if(ssl_write(ssl, server->url, strlen(server->url)+1) < 0) {
            ssl_free(ssl);
            ssl_ctx_free(ssl_ctx);
            close(sta_socket);
            vTaskDelay(1000 / portTICK_RATE_MS);
            os_printf("send fail\n");
            continue;
        }
        os_printf("Request send success\n");

        while((recbytes = ssl_read(ssl, &read_buf)) >= 0) {

            if(recbytes == 0){
                vTaskDelay(500 / portTICK_RATE_MS);
                continue;
            }
            
            if(recbytes > UPGRADE_DATA_SEG_LEN) {
                ssl_free(ssl);
                ssl_ctx_free(ssl_ctx);
                close(sta_socket);
                vTaskDelay(2000 / portTICK_RATE_MS);
                printf("bigger than UPGRADE_DATA_SEG_LEN\n");
            }

            if((recbytes)<=1460)
                memcpy(precv_buf,read_buf,recbytes);
            else
                os_printf("ERR2:arr_overflow,%u,%d\n",__LINE__,recbytes);

            if(FALSE==flash_erased){
                ssl_free(ssl);
                ssl_ctx_free(ssl_ctx);
                close(sta_socket);
                os_printf("pre erase flash!\n");
                upgrade_data_load(precv_buf,recbytes);
                break;
            }
            
            if(false == upgrade_data_load(read_buf,recbytes)) {
                os_printf("upgrade data error!\n");
                ssl_free(ssl);
                ssl_ctx_free(ssl_ctx);
                close(sta_socket);
                flash_erased=FALSE;
                vTaskDelay(1000 / portTICK_RATE_MS);
                break;
            }
            /*this two length data should be equal, if totallength is bigger, 
             *maybe data wrong or server send extra info, drop it anyway*/
            if(totallength >= sumlength) {
                os_printf("upgrade data load finish.\n");
                ssl_free(ssl);
                ssl_ctx_free(ssl_ctx);
                close(sta_socket);
                goto finish;
            }
            os_printf("upgrade_task %d word left\n",uxTaskGetStackHighWaterMark(NULL));
            
        }
        
        if(recbytes < 0) {
            os_printf("ERROR:read data fail! recbytes %d\r\n",recbytes);
            ssl_free(ssl);
            ssl_ctx_free(ssl_ctx);
            close(sta_socket);
            flash_erased=FALSE;
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
        
        os_printf("upgrade_task %d word left\n",uxTaskGetStackHighWaterMark(NULL));
        
        totallength =0;
        sumlength = 0;
    }
    
finish:

	if(upgrade_crc_check(system_get_fw_start_sec(),sumlength) != 0)
	{
		printf("upgrade crc check failed !\n");
		server->upgrade_flag = false;
        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);	
	}

    os_timer_disarm(&upgrade_timer);

    totallength = 0;
    sumlength = 0;
    flash_erased=FALSE;
    free(precv_buf);
    
    if(retry_count == UPGRADE_RETRY_TIMES){
        /*retry too many times, fail*/
        server->upgrade_flag = false;
        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);

    }else{
        if (server->upgrade_flag == true)
			system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
    }
    
    upgrade_deinit();
    
    os_printf("\n Exit upgrade task.\n");
    if (server->check_cb != NULL) {
        server->check_cb(server);
    }
    vTaskDelay(100 / portTICK_RATE_MS);
    vTaskDelete(NULL);
}
#else
/******************************************************************************
 * FunctionName : upgrade_task
 * Description  : task to connect with target server and get firmware data 
 * Parameters   : pvParameters--save the server address\port\request frame for
 *              : the upgrade server\call back functions to tell the userapp
 *              : the result of this upgrade task
 * Returns      : none
*******************************************************************************/
void upgrade_task(void *pvParameters)
{
    int recbytes;
    int sta_socket;
    int retry_count = 0;
    struct ip_info ipconfig;
    
    struct upgrade_server_info *server = pvParameters;

    flash_erased=FALSE;
    precv_buf = (char*)malloc(UPGRADE_DATA_SEG_LEN);
    if(NULL == precv_buf){
        os_printf("upgrade_task,memory exhausted, check it\n");
    }
    
    while (retry_count++ < UPGRADE_RETRY_TIMES) {
		
		if (giveup) {os_printf("giveup !\r\n");break;}
        
        wifi_get_ip_info(STATION_IF, &ipconfig);

        /* check the ip address or net connection state*/
        while (ipconfig.ip.addr == 0) {
            vTaskDelay(1000 / portTICK_RATE_MS);
            wifi_get_ip_info(STATION_IF, &ipconfig);
        }
        
        sta_socket = socket(PF_INET,SOCK_STREAM,0);
        if (-1 == sta_socket) {
            close(sta_socket);
            vTaskDelay(1000 / portTICK_RATE_MS);
            os_printf("socket fail !\r\n");
            continue;
        }

        /*for upgrade connection debug*/
        //server->sockaddrin.sin_addr.s_addr= inet_addr("192.168.1.170");

        if(0 != connect(sta_socket,(struct sockaddr *)(&server->sockaddrin),sizeof(struct sockaddr))) {
            close(sta_socket);
            vTaskDelay(1000 / portTICK_RATE_MS);
            os_printf("connect fail!\r\n");
            continue;
        }
        os_printf("Connect ok!\n");

        system_upgrade_init();
        system_upgrade_flag_set(UPGRADE_FLAG_START);

        if(write(sta_socket,server->url,strlen(server->url)) < 0) {
            close(sta_socket);
            vTaskDelay(1000 / portTICK_RATE_MS);
            os_printf("send fail\n");
            continue;
        }
        os_printf("Request send success\n");

        while((recbytes = read(sta_socket, precv_buf, UPGRADE_DATA_SEG_LEN)) > 0) {
            if(FALSE==flash_erased){
					close(sta_socket);
					os_printf("pre erase flash!\n");
					if(false == upgrade_data_load(precv_buf,recbytes)){
					os_printf("upgrade data error!\n");
					close(sta_socket);
					flash_erased=FALSE;
					vTaskDelay(1000 / portTICK_RATE_MS);
 //         	    break;
				}
//                upgrade_data_load(precv_buf,recbytes);
                break;                    
            }
            
            if(false == upgrade_data_load(precv_buf,recbytes)) {
                os_printf("upgrade data error!\n");
                close(sta_socket);
                flash_erased=FALSE;
                vTaskDelay(1000 / portTICK_RATE_MS);
                break;
            }
            /*this two length data should be equal, if totallength is bigger, 
             *maybe data wrong or server send extra info, drop it anyway*/
            if(totallength >= sumlength) {
                os_printf("upgrade data load finish.\n");
                close(sta_socket);
                goto finish;
            }

//            os_printf("upgrade_task %d word left\n",uxTaskGetStackHighWaterMark(NULL));
            
        }
        
        if(recbytes <= 0) {
            close(sta_socket);
            flash_erased=FALSE;
            vTaskDelay(1000 / portTICK_RATE_MS);
            os_printf("ERROR:read data fail!\r\n");
        }

        totallength =0;
        sumlength = 0;
    }
    
finish:

    os_timer_disarm(&upgrade_timer);

	if(upgrade_crc_check(system_get_fw_start_sec(),sumlength) != 0)
	{
		printf("upgrade crc check failed !\n");
		server->upgrade_flag = false;
        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);	
	}

    if(NULL != precv_buf) {
        free(precv_buf);
    }
    
    totallength = 0;
    sumlength = 0;
    flash_erased=FALSE;

    if(retry_count == UPGRADE_RETRY_TIMES){
        /*retry too many times, fail*/
        server->upgrade_flag = false;
        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);

    }else{
        if (server->upgrade_flag == true)
			system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
    }
    
    upgrade_deinit();
    
    os_printf("\n Exit upgrade task.\n");
    if (server->check_cb != NULL) {
        server->check_cb(server);
    }
    vTaskDelay(100 / portTICK_RATE_MS);
    vTaskDelete(NULL);
}
#endif
/******************************************************************************
 * FunctionName : upgrade_check
 * Description  : check the upgrade process, if not finished in 300S,exit
 * Parameters   : pvParameters--save the server address\port\request frame for
 * Returns      : none
*******************************************************************************/
LOCAL void  
upgrade_check(struct upgrade_server_info *server)
{
    /*network not stable, upgrade data lost, this may be called*/
//    vTaskDelete(pxCreatedTask);
	giveup = true;
    os_timer_disarm(&upgrade_timer);
    
    if(NULL != precv_buf) {
        free(precv_buf);
    }
    
    totallength = 0;
    sumlength = 0;
    flash_erased=FALSE;

    /*take too long to finish,fail*/
    server->upgrade_flag = false;
    system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
    
    upgrade_deinit();
    
    os_printf("\n upgrade fail,exit.\n");
//    if (server->check_cb != NULL) {
//        server->check_cb(server);
//    }

}

#ifdef UPGRADE_SSL_ENABLE
/******************************************************************************
 * FunctionName : system_upgrade_start_ssl
 * Description  : task to connect with target server and get firmware data 
 * Parameters   : pvParameters--save the server address\port\request frame for
 *              : the upgrade server\call back functions to tell the userapp
 *              : the result of this upgrade task
 * Returns      : true if task created successfully, false failed.
*******************************************************************************/
/******************************************************************************
 * NOTE:THE SYSTEM_UPGRADE_START_SSL TASK NEEDS 20K+ RAM SPACE, IT IS REALLY
 *      A BIG NUMBER. IF WANT TO RUN THIS TASK, YOU SHOULD MODIFY THE USER_CONFIG.H
 *      TO USE THE LIGHT WEB_SERVICE INSTEAD OF THE HTTPD SERVER TO SAVE MEMORY.
 *******************************************************************************/

BOOL  
system_upgrade_start_ssl(struct upgrade_server_info *server)
{
    portBASE_TYPE ret = 0;
    
    if(NULL == pxCreatedTask){
        ret = xTaskCreate(upgrade_ssl_task, "upgrade_task", 700, server, 5, pxCreatedTask);//1024, 890 left

        if(pdPASS == ret){
            os_timer_disarm(&upgrade_timer);
            os_timer_setfn(&upgrade_timer, (os_timer_func_t *)upgrade_check, server);
            os_timer_arm(&upgrade_timer, server->check_times, 0);
        }
    }
 
    return(pdPASS == ret);
}
#else
/******************************************************************************
 * FunctionName : system_upgrade_start
 * Description  : task to connect with target server and get firmware data 
 * Parameters   : pvParameters--save the server address\port\request frame for
 *              : the upgrade server\call back functions to tell the userapp
 *              : the result of this upgrade task
 * Returns      : true if task created successfully, false failed.
*******************************************************************************/

BOOL  
system_upgrade_start(struct upgrade_server_info *server)
{
    portBASE_TYPE ret = 0;
    
    if(NULL == pxCreatedTask){
        ret = xTaskCreate(upgrade_task, "upgrade_task", 324, server, 5, pxCreatedTask);//224   1024, 890 left

        if(pdPASS == ret){
            os_timer_disarm(&upgrade_timer);
            os_timer_setfn(&upgrade_timer, (os_timer_func_t *)upgrade_check, server);
            os_timer_arm(&upgrade_timer, server->check_times, 0);
        }
    }
 
    return(pdPASS == ret);
}
#endif