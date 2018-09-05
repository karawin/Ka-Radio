#include "query.h"

#include <mdns/mdns.h>
#include "server.h"

#if MDNS_ENABLE_QUERY

mdnsQueryHandle *mdns_query(mdnsHandle *handle, char *service, mdnsProtocol protocol, mdnsQueryCallback *callback) {
    LOG(TRACE, "mdns: Creating query %s", service);

    mdnsQueryHandle *qHandle = malloc(sizeof(mdnsQueryHandle));
    
    // copy over service name
    uint8_t serviceLen = strlen(service);
    qHandle->service = malloc(serviceLen + 1);
    memcpy(qHandle->service, service, serviceLen + 1);

    qHandle->protocol = protocol;
    qHandle->callback = callback;

    mdns_add_query(handle, qHandle);

    return qHandle;
}

void mdns_query_destroy(mdnsHandle *handle, mdnsQueryHandle *query) {
    LOG(TRACE, "mdns: Destroying query %s", query->service);

    mdns_remove_query(handle, query);

    free(query->service);
    free(query);
}

#endif /* MDNS_ENABLE_QUERY */
