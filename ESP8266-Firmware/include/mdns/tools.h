#ifndef mdns_tools_h_included
#define mdns_tools_h_included

#include <mdns/mdns.h>
#include "platform.h"

// Build DNS-SD service name: _type._protocol.local
char *mdns_make_service_name(mdnsService *service);

// Build DNS-SD FQDN: Hostname._type._protocol.local
char *mdns_make_fqdn(char *hostname, mdnsService *service);

// Build local hostname: Hostname.local
char *mdns_make_local(char *hostname);

#endif /* mdns_tools_h_included */