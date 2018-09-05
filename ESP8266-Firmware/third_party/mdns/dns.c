#include "dns.h"
#include "tools.h"
#include "server.h"
#include "mdns_network.h"

#include "debug.h"

static inline uint16_t sizeof_record_header(char *fqdn) {
    uint16_t size = 0;

    uint8_t fqdnLen = strlen(fqdn);

    size += fqdnLen + 1;
    size++; // terminator
    size += 2; // type
    size += 2; // class
    size += 4; // ttl
    size += 2; // data length

    return size;
}

uint16_t mdns_sizeof_PTR(char *hostname, mdnsService **services, uint8_t numServices, mdnsService *serviceOrNull) {
    uint16_t size = 0;
	uint8_t i;
    for (i = 0; i < numServices; i++) {
        mdnsService *service = services[i];
        if (serviceOrNull) {
            service = serviceOrNull; // service override
        }

        char *fqdn = mdns_make_service_name(service); // _type._protocol.local
        size += sizeof_record_header(fqdn);
        free(fqdn);

        // packet data
        fqdn = mdns_make_fqdn(hostname, service);
        size += strlen(fqdn) + 2 /* length header + zero byte */;
        free(fqdn);

        if (serviceOrNull) {
            break; // short circuit if we only should send one service
        }
    }

    return size;
}

uint16_t mdns_sizeof_SRV(char *hostname, mdnsService **services, uint8_t numServices, mdnsService *serviceOrNull) {
    uint16_t size = 0;
	uint8_t i;
    for (i = 0; i < numServices; i++) {
        mdnsService *service = services[i];
        if (serviceOrNull) {
            service = serviceOrNull; // service override
        }
        uint8_t serviceNameLen = strlen(service->name);

        char *fqdn = mdns_make_fqdn(hostname, service); // Hostname._service._protocol.local
        size += sizeof_record_header(fqdn);
        free(fqdn);
        
        size += 2; // prio
        size += 2; // weight
        size += 2; // port

        // target
        fqdn = mdns_make_local(hostname);
        size += strlen(fqdn) + 2 /* length header + zero byte */;
        free(fqdn);

        if (serviceOrNull) {
            break; // short circuit if we only should send one service
        }
    }

    return size;
}

uint16_t mdns_sizeof_TXT(char *hostname, mdnsService **services, uint8_t numServices, mdnsService *serviceOrNull) {
    uint16_t size = 0;
	uint8_t i;
    // Hostname._servicetype._protocol.local
    for (i = 0; i < numServices; i++) {
        mdnsService *service = services[i];

        if (serviceOrNull) {
            service = serviceOrNull; // service override
        }

        if (service->numTxtRecords > 0) {
            char *fqdn = mdns_make_fqdn(hostname, service); // Servicename._type._protocol.local
            size += sizeof_record_header(fqdn);
            free(fqdn);

            uint8_t txtLen = 0;
			uint8_t j;
            for(j = 0; j < service->numTxtRecords; j++) {
                txtLen += 1 /* len of txt record k/v pair */;
                txtLen += strlen(service->txtRecords[j].name) + 1 /* = */;
                txtLen += strlen(service->txtRecords[j].value);
            }
            if (txtLen == 0) {
                txtLen = 1; // NULL byte sentinel at least
            }
            size += txtLen;
        }

        if (serviceOrNull) {
            break; // short circuit if we only should send one service
        }
    }

    return size;
}

uint16_t mdns_sizeof_A(char *hostname) {
    uint16_t size = 0;

    // fqdn
    char *fqdn = mdns_make_local(hostname);
    size += sizeof_record_header(fqdn);
    free(fqdn);

    // ip address
    size += 4;

    return size;
}

uint16_t mdns_sizeof_AAAA(char *hostname, ip6_address_t ip) {
    uint16_t size = 0;
    ip6_addr_t zero = { 0 };
    if (memcmp(&zero, &ip, sizeof(ip6_addr_t)) == 0) {
        return 0;
    }

    // fqdn
    char *fqdn = mdns_make_local(hostname);
    size += sizeof_record_header(fqdn);
    free(fqdn);

    // ip address
    size += 16;

    return size;
}

static char *append_tokenized(char *buffer, char *domain) {
    uint8_t len = strlen(domain);
    
    // calculate lengts
    uint8_t parts[5];
    uint8_t partIndex = 0;
    uint8_t lastLen = 0;
	uint8_t i;
    for (i = 0; i < len; i++) {
        if (domain[i] == '.') {
            parts[partIndex++] = i - lastLen;
            lastLen = i + 1;
        }
    }
    parts[partIndex] = len - lastLen;

    // write part entries
    partIndex = 0;
    for (i = 0; i < len; i++) {
        if ((i == 0) || (domain[i] == '.')) {
            *buffer++ = parts[partIndex++];
        }
        if (domain[i] != '.') {
            *buffer++ = domain[i];
        }
    }
    *buffer++ = 0; // terminator

    return buffer;
}

static inline char *record_header(char *buffer, char *fqdn, mdnsRecordType type, uint16_t ttl, uint16_t len) {
    buffer = append_tokenized(buffer, fqdn);

    // type
    *buffer++ = 0;
    *buffer++ = type;
    // class
    *buffer++ = 0x80; // cache buster flag
    *buffer++ = 0x01; // class: internet
    // ttl
    *buffer++ = ttl >> 24;
    *buffer++ = ttl >> 16;
    *buffer++ = ttl >> 8;
    *buffer++ = ttl & 0xff;
    // data length
    *buffer++ = len >> 8;
    *buffer++ = len & 0xff;

    return buffer;
}

char *mdns_make_PTR(char *buffer, uint16_t ttl, char *hostname, mdnsService **services, uint8_t numServices, mdnsService *serviceOrNull) {
    char *ptr = buffer;
	uint8_t i;
    for (i = 0; i < numServices; i++) {
        mdnsService *service = services[i];
        if (serviceOrNull) {
            service = serviceOrNull; // service override
        }

        char *target = mdns_make_fqdn(hostname, service);
        uint8_t targetLen = strlen(target);
        char *fqdn = mdns_make_service_name(service); // _type._protocol.local
        ptr = record_header(ptr, fqdn, mdnsRecordTypePTR, ttl, targetLen + 2 /* length header + zero byte */);
        free(fqdn);

        // packet data
        ptr = append_tokenized(ptr, target);
        free(target);

        if (serviceOrNull) {
            break; // short circuit if we only should send one service
        }
    }

    return ptr;
}

char *mdns_make_SRV(char *buffer, uint16_t ttl, char *hostname, mdnsService **services, uint8_t numServices, mdnsService *serviceOrNull) {
    char *ptr = buffer;
	uint8_t i;
    for (i = 0; i < numServices; i++) {
        mdnsService *service = services[i];
        if (serviceOrNull) {
            service = serviceOrNull; // service override
        }
        uint8_t serviceNameLen = strlen(service->name);

        char *target = mdns_make_local(hostname);
        uint8_t targetLen = strlen(target);
        char *fqdn = mdns_make_fqdn(hostname, service); // Hostname._service._protocol.local
        ptr = record_header(ptr, fqdn, mdnsRecordTypeSRV, ttl, targetLen + 6 + 2 /* prio, weight, port + length header + zero byte */);
        free(fqdn);
        
        // prio
        *ptr++ = 0;
        *ptr++ = 0;

        // weight
        *ptr++ = 0;
        *ptr++ = 0;

        // port
        *ptr++ = service->port >> 8;
        *ptr++ = service->port & 0xff; 

        // target
        ptr = append_tokenized(ptr, target);
        free(target);

        if (serviceOrNull) {
            break; // short circuit if we only should send one service
        }
    }

    return ptr;
}

char *mdns_make_TXT(char *buffer, uint16_t ttl, char *hostname, mdnsService **services, uint8_t numServices, mdnsService *serviceOrNull) {
    char *ptr = buffer;

    // Hostname._servicetype._protocol.local
	uint8_t i,j;
    for (i = 0; i < numServices; i++) {
        mdnsService *service = services[i];

        if (serviceOrNull) {
            service = serviceOrNull; // service override
        }

        if (service->numTxtRecords > 0) {
            uint8_t txtLen = 0;
            for(j = 0; j < service->numTxtRecords; j++) {
                txtLen += 1 /* len of txt record k/v pair */;
                txtLen += strlen(service->txtRecords[j].name) + 1 /* = */;
                txtLen += strlen(service->txtRecords[j].value);
            }
            if (txtLen == 0) {
                txtLen = 1; // NULL byte sentinel at least
            }

            char *fqdn = mdns_make_fqdn(hostname, service); // Servicename._type._protocol.local
            ptr = record_header(ptr, fqdn, mdnsRecordTypeTXT, ttl, txtLen);
            free(fqdn);

            for(j = 0; j < service->numTxtRecords; j++) {
                uint8_t namLen = strlen(service->txtRecords[j].name);
                uint8_t valLen = strlen(service->txtRecords[j].value);
                *ptr++ = namLen + 1 + valLen;
                memcpy(ptr, service->txtRecords[j].name, namLen);
                ptr += namLen;
                *ptr++ = '=';
                memcpy(ptr, service->txtRecords[j].value, valLen);
                ptr += valLen;                    
            }

            if (txtLen == 0) {
                *ptr++ = 0; // empty txt record
            }
        }

        if (serviceOrNull) {
            break; // short circuit if we only should send one service
        }
    }

    return ptr;
}

char *mdns_make_A(char *buffer, uint16_t ttl, char *hostname, ip_address_t ip) {
    char *ptr = buffer;

    // fqdn
    char *fqdn = mdns_make_local(hostname);
    ptr = record_header(ptr, fqdn, mdnsRecordTypeA, ttl, 4);
    free(fqdn);

    // ip address
    memcpy(ptr, &ip, 4);
    ptr += 4;

    return ptr;
}

char *mdns_make_AAAA(char *buffer, uint16_t ttl, char *hostname, ip6_address_t ip) {
    char *ptr = buffer;

    // make sure we actually have an IPv6 address
    ip6_addr_t zero = { 0 };
    if (memcmp(&zero, &ip, sizeof(ip6_addr_t)) == 0) {
        return ptr;
    }

    // fqdn
    char *fqdn = mdns_make_local(hostname);
    ptr = record_header(ptr, fqdn, mdnsRecordTypeAAAA, ttl, 16);
    free(fqdn);

    // ip address
    memcpy(ptr, &ip, 16);
    ptr += 16;

    return ptr;
}
