#include <stdlib.h>
#include <strings.h>

#include <mdns/mdns.h>
#include "mdns_publish.h"

#include "mdns_network.h"
#include "server.h"
#include "tools.h" // deceprated
#include "dns.h"

#include "debug.h"

//
// PUBLISH
//

#if MDNS_ENABLE_PUBLISH

static uint16_t mdns_calculate_size(mdnsHandle *handle, mdnsRecordType query, mdnsService *serviceOrNull) {
    uint8_t hostnameLen = strlen(handle->hostname);
    uint16_t size = 12; // header

    switch (query) {
        case mdnsRecordTypePTR:
            size += mdns_sizeof_PTR(handle->hostname, handle->services, handle->numServices, serviceOrNull);
        case mdnsRecordTypeSRV:
            size += mdns_sizeof_SRV(handle->hostname, handle->services, handle->numServices, serviceOrNull);
        case mdnsRecordTypeTXT:
            size += mdns_sizeof_TXT(handle->hostname, handle->services, handle->numServices, serviceOrNull);
        case mdnsRecordTypeA:
            size += mdns_sizeof_A(handle->hostname);
        case mdnsRecordTypeAAAA:
            size += mdns_sizeof_AAAA(handle->hostname, handle->ip6);
            break;
    }

    // LOG(TRACE, "mdns: Calculated packet size: %d", size);
    return size;
}

static char *mdns_prepare_response(mdnsHandle *handle, mdnsRecordType query, uint16_t ttl, uint16_t transactionID, uint16_t *len, mdnsService *serviceOrNull) {
    uint8_t hostnameLen = strlen(handle->hostname);
    uint16_t size = mdns_calculate_size(handle, query, serviceOrNull);
    char *buffer = calloc(size, 1);
    char *ptr = buffer;
    *len = size;

    // transaction ID
    *ptr++ = transactionID >> 8;
    *ptr++ = transactionID & 0xff;

    // flags
    mdnsPacketFlags flags;
    memset(&flags, 0, 2);
    flags.isResponse = 1;
    flags.opCode = opCodeQuery;
    flags.authoritative = 1;
    flags.responseCode = responseCodeNoError;
    memcpy(ptr, &flags, 2);
    ptr += 2;

    // num questions (zero)
    *ptr++ = 0;  *ptr++ = 0;

    // num answers (one), we have at least the A record
    *ptr++ = 0;  *ptr++ = 1;

    // num authority RRs (zero)
    *ptr++ = 0;  *ptr++ = 0;

    // num Additional RRs
    // FIXME: update number of records afterwards, not here
    uint8_t numRRs = 0;
			uint8_t i;
    switch (query) {
        case mdnsRecordTypePTR:
            numRRs += handle->numServices;
        case mdnsRecordTypeSRV:
            numRRs += handle->numServices;
        case mdnsRecordTypeTXT:
            for (i = 0; i < handle->numServices; i++) {
                if (handle->services[i]->numTxtRecords > 0) {
                    numRRs ++; // one per service
                }
            }
        case mdnsRecordTypeA:
            numRRs++;
        case mdnsRecordTypeAAAA:
            numRRs++;
            break;
    }
    *ptr++ = 0;
    *ptr++ = numRRs - 1; // One is already in the answer, the others are additional RRs

    char *fqdn = NULL;

    // records
    switch (query) {
        case mdnsRecordTypePTR:
            ptr = mdns_make_PTR(ptr, ttl, handle->hostname, handle->services, handle->numServices, serviceOrNull);
        case mdnsRecordTypeSRV:
            ptr = mdns_make_SRV(ptr, ttl, handle->hostname, handle->services, handle->numServices, serviceOrNull);
        case mdnsRecordTypeTXT:
            ptr = mdns_make_TXT(ptr, ttl, handle->hostname, handle->services, handle->numServices, serviceOrNull);
        case mdnsRecordTypeA:
            ptr = mdns_make_A(ptr, ttl, handle->hostname, handle->ip);
        case mdnsRecordTypeAAAA:
            ptr = mdns_make_AAAA(ptr, ttl, handle->hostname, handle->ip6);
            break;
    }

    return buffer;
}

static void send_mdns_response_packet(mdnsHandle *handle, uint16_t ttl, uint16_t transactionID, mdnsService *serviceOrNull) {
    uint16_t responseLen = 0;
    char *response;

    response = mdns_prepare_response(handle, mdnsRecordTypePTR, ttl, transactionID, &responseLen, serviceOrNull);
    mdns_send_udp_packet(handle, response, responseLen);
}

//
// API
//

#if !MDNS_BROADCAST_ONLY
void mdns_parse_query(mdnsHandle *handle, mdnsStreamBuf *buffer, uint16_t numQueries, uint16_t transactionID) {
    // we have to react to:
    // - domain name queries
    // - service discovery queries to one of our registered service types
    // - service discovery queries with our service name and type
    // - browsing queries: _services._dns-sd._udp

    LOG(TRACE, "mdns: parsing %d queries", numQueries);

    char *serviceName[4];

    while (numQueries--) {
        // Read FQDN
        uint8_t stringsRead = 0;
        do {
            uint8_t len = mdns_stream_read8(buffer);
            if (len & 0xC0) { // Compressed pointer (not supported)
                (void)mdns_stream_read8(buffer);
                break;
            }
            if (len == 0x00) { // End of name
                break;
            }
            if (stringsRead > 4) {
                return;
            }
            serviceName[stringsRead] = mdns_stream_read_string(buffer, len);
            stringsRead++;
        } while (true);


        mdnsRecordType queryType = mdns_stream_read16(buffer);
        uint16_t queryClass = mdns_stream_read16(buffer);

        if (queryClass & 0x80) {
            // should be sent via unicast, not supported
            return;
        }

        switch(queryType) {
            case mdnsRecordTypePTR: {
                // PTR records are for searching for services
				uint8_t i;
                for (i = 0; i < handle->numServices; i++) {
                    mdnsService *service = handle->services[i];
                    char *proto = (service->protocol == mdnsProtocolTCP) ? "_tcp" : "_udp";
                    if ((strcasecmp(serviceName[0], service->name) == 0) && (strcasecmp(serviceName[1], proto) == 0)) {
                        LOG(TRACE, "mdns: responding to PTR query");
                        uint16_t responseLen = 0;
                        char *response = mdns_prepare_response(handle, mdnsRecordTypePTR, MDNS_MULTICAST_TTL, transactionID, &responseLen, service);
                        mdns_send_udp_packet(handle, response, responseLen);
                        break;
                    }
                }
                break;
            }

            case mdnsRecordTypeA: {
                // A records want to find an IP address for a hostname
                if (strcasecmp(serviceName[0], handle->hostname) == 0) {
                    LOG(TRACE, "mdns: responding to A query");
                    uint16_t responseLen = 0;
                    char *response = mdns_prepare_response(handle, mdnsRecordTypeA, MDNS_MULTICAST_TTL, transactionID, &responseLen, NULL);
                    mdns_send_udp_packet(handle, response, responseLen);
                    break;                    
                }
            }

            case mdnsRecordTypeSRV:
            case mdnsRecordTypeTXT: {
                // TXT record, only answer if the complete service name is correct
                if (strcasecmp(serviceName[0], handle->hostname) == 0) {
						uint8_t i;
                    for (i = 0; i < handle->numServices; i++) {
                        mdnsService *service = handle->services[i];
                        char *proto = (service->protocol == mdnsProtocolTCP) ? "_tcp" : "_udp";
                        if ((strcasecmp(serviceName[1], service->name) == 0) && (strcasecmp(serviceName[2], proto) == 0)) {
                            LOG(TRACE, "mdns: responding to SRV or TXT query");
                            uint16_t responseLen = 0;
                            char *response = mdns_prepare_response(handle, mdnsRecordTypeTXT, MDNS_MULTICAST_TTL, transactionID, &responseLen, service);
                            mdns_send_udp_packet(handle, response, responseLen);
                            break;
                        }
                    }
                }
                break;
            }

            case mdnsRecordTypeAny: {
                // This requests just everything about a host, officially deceprated but I can see it on the network
                if (strcasecmp(serviceName[0], handle->hostname) == 0) {
                    LOG(TRACE, "mdns: responding to ANY query");
    
                    // this is a cascade, we will send multiple packets to avoid
                    // overloading the mtu
						uint8_t i;
                    for (i = 0; i < handle->numServices; i++) {
                        mdnsService *service = handle->services[i];
                        uint16_t responseLen = 0;
                        char *response = mdns_prepare_response(handle, mdnsRecordTypePTR, MDNS_MULTICAST_TTL, transactionID, &responseLen, service);
                        mdns_send_udp_packet(handle, response, responseLen);                        
                    }
                    break;                    
                }
            }

            case mdnsRecordTypeAAAA:
            default:
                break;
        }

    }
}
#endif /* !MDNS_BROADCAST_ONLY */

void mdns_announce(mdnsHandle *handle) {
    LOG(DEBUG, "mdns: Announcing");
    // respond with our data, setting most significant bit in RRClass to update caches
    send_mdns_response_packet(handle, MDNS_MULTICAST_TTL, 0, NULL);
}

void mdns_goodbye(mdnsHandle *handle) {
    LOG(DEBUG, "mdns: Goodbye");
    // send announce packet with TTL of zero
    send_mdns_response_packet(handle, 0, 0, NULL);
}

#endif /* MDNS_ENABLE_PUBLISH */