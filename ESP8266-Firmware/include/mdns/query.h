#ifndef mdns_query_h_included
#define mdns_query_h_included

#include <mdns/mdns.h>

#if MDNS_ENABLE_QUERY

typedef struct _mdnsQueryHandle {
    char *service;
    mdnsProtocol protocol;
    mdnsQueryCallback *callback;
} mdnsQueryHandle;

#endif /* MDNS_ENABLE_QUERY */

#endif /* mdns_query_h_included */