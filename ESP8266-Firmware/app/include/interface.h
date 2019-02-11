#ifndef __INTERFACE_H__
#define __INTERFACE_H__


#undef PSTR
#undef PROGMEM
//#define PROGMEM   ICACHE_RODATA_ATTR
#define PROGMEM  __attribute__((section(".irom.text")))
#define PSTR(s) (__extension__({ static const char __c[] STORE_ATTR __attribute__((section(".irom.text"))) = {s}; &__c[0];}))
#define MAXDATAT	 256


#define RELEASE "1.8"
#define REVISION "2"


extern unsigned short adcdiv;	
extern char* getIp() ;
void switchCommand(void );
void checkCommand(int size, char* s);
void wifiConnectMem();
char* webInfo();
char* webList(int id);
void setHostname(char* s);

int kasprintf(char *str, const char *format, ...);

#define kprintf(fmt, ...) do {    \
        printf(fmt, ##__VA_ARGS__);   \
		telnetWrite(2*MAXDATAT,fmt, ##__VA_ARGS__); \
	} while (0)
		
#define kprintfl(fmt, ...) do {    \
        printf(fmt, ##__VA_ARGS__);   \
		telnetWrite(1024,fmt, ##__VA_ARGS__); \
	} while (0)
	


#endif		
		