#ifndef mdns_platform_h_included
#define mdns_platform_h_included

#include <lwip/opt.h>
#include <lwip/udp.h>
#include <lwip/inet.h>
#include <lwip/ip_addr.h>
#include <lwip/igmp.h>
#include <lwip/mem.h>

#include <mdns/mdns.h>

typedef struct udp_pcb mdnsUDPHandle;
typedef struct pbuf mdnsNetworkBuffer;

struct _mdnsStreamBuf {
    struct pbuf *bufList;
    uint16_t currentPosition;
};

#endif /* mdns_platform_h_included */
