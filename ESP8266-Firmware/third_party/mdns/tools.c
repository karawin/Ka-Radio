#include "tools.h"
#include "server.h"

// Build DNS-SD service name: _type._protocol.local
char *mdns_make_service_name(mdnsService *service) {
    const uint8_t size = strlen(service->name) + 1 + 4 + 1 + 5 + 1;
    char *buffer = malloc(size);
    sprintf(buffer, "%s._%s.local", service->name, service->protocol == mdnsProtocolTCP ? "tcp" : "udp");
    return buffer;
}

// Build DNS-SD FQDN: Hostname._type._protocol.local
char *mdns_make_fqdn(char *hostname, mdnsService *service) {
    const uint8_t size = strlen(hostname) + 1 + strlen(service->name) + 1 + 4 + 1 + 5 + 1;
    char *buffer = malloc(size);
    sprintf(buffer, "%s.%s._%s.local", hostname, service->name, service->protocol == mdnsProtocolTCP ? "tcp" : "udp");
    return buffer;
}

// Build local hostname: Hostname.local
char *mdns_make_local(char *hostname) {
    const uint8_t size = strlen(hostname) + 1 + 5 + 1;
    char *buffer = malloc(size);
    sprintf(buffer, "%s.local", hostname);
    return buffer;
}
