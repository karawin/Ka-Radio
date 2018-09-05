#ifndef mdns_mdns_h_included
#define mdns_mdns_h_included

//
// Settings, override to configure
//

// Enable query API (Disabled by default)
#ifndef MDNS_ENABLE_QUERY
#define MDNS_ENABLE_QUERY 0
#endif

// Enable publishing API
#ifndef MDNS_ENABLE_PUBLISH
#define MDNS_ENABLE_PUBLISH 1
#endif

// Broadcast only API ?
#ifndef MDNS_BROADCAST_ONLY
#define MDNS_BROADCAST_ONLY 0
#endif

#if MDNS_BROADCAST_ONLY
#undef MDNS_ENABLE_QUERY
#define MDNS_ENABLE_QUERY 0
#endif /* MDNS_BROADCAST_ONLY */

#include <stdint.h>

typedef union ip_address {
    uint32_t addr;
    uint8_t addr8[4];
} ip_address_t;

typedef struct ip6_address {
  uint32_t addr[4];
} ip6_address_t;

//
// MDNS service
//

// MDNS Server handle
typedef struct _mdnsHandle mdnsHandle;

// Create a MDNS server for specified hostname
mdnsHandle *mdns_create(char *hostname);

// Start broadcasting MDNS records
void mdns_start(mdnsHandle *handle);

// Stop broadcasting MDNS records
void mdns_stop(mdnsHandle *handle);

// Restart MDNS service (call on IP/Network change)
void mdns_restart(mdnsHandle *handle);

// Destroy MDNS handle
void mdns_destroy(mdnsHandle *handle);

// Set IP address of station, call this in the DHCP callback to update IP
void mdns_update_ip(mdnsHandle *handle, const ip_address_t ip, const ip6_address_t ip6);


//
// MDNS records
//

// MDNS protocol type
typedef enum _mdnsProtocol {
    mdnsProtocolTCP = 0,
    mdnsProtocolUDP
} mdnsProtocol;

// MDNS TXT Record handle
typedef struct _mdnsTxtRecord {
    char *name;
    char *value;
} mdnsTxtRecord;

// MDNS Service handle
typedef struct _mdnsService {
    // name of the service (for example "http")
    char *name;

    // protocol type (TCP or UDP)
    mdnsProtocol protocol;

    // port of the service
    uint16_t port;

    // TXT records
    mdnsTxtRecord *txtRecords;
    // number of TXT records
    uint8_t numTxtRecords;

#if defined(MDNS_ENABLE_QUERY) && MDNS_ENABLE_QUERY
    // IP address of the service
    // only used when this is a query response
    struct ip_addr ip;
#endif
} mdnsService;

#if defined(MDNS_ENABLE_PUBLISH) && MDNS_ENABLE_PUBLISH

// Create a new service record
mdnsService *mdns_create_service(char *name, mdnsProtocol protocol, uint16_t port);

// Add TXT record to service record
void mdns_service_add_txt(mdnsService *service, char *key, char *value);

// Add service to MDNS broadcaster
void mdns_add_service(mdnsHandle *handle, mdnsService *service);

// Remove service from MDNS broadcaster
void mdns_remove_service(mdnsHandle *handle, mdnsService *service);

// Destroy a service handle
void mdns_service_destroy(mdnsService *service);

#endif /* MDNS_ENABLE_PUBLISH */


//
// MDNS Querying
//

#if defined(MDNS_ENABLE_QUERY) && MDNS_ENABLE_QUERY
// MDNS Query handle
typedef struct _mdnsQueryHandle mdnsQueryHandle;

// Callback for found services
typedef void *(mdnsQueryCallback)(mdnsService *service);

// start a MDNS query, calls callback if something is found
mdnsQueryHandle *mdns_query(mdnsHandle *handle, char *service, mdnsProtocol protocol, mdnsQueryCallback *callback);

// cancel MDNS query
void mdns_query_destroy(mdnsHandle *handle, mdnsQueryHandle *query);
#endif /* MDNS_ENABLE_QUERY */

#endif /* mdns_mdns_h_included */