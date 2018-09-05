#include <mdns/mdns.h>
#include "mdns_query.h"
#include "mdns_network.h"

//
// QUERY
//

#if MDNS_ENABLE_QUERY
void mdns_parse_answers(mdnsStreamBuf *buffer, uint16_t numAnswers) {
    LOG(TRACE, "mdns: Parsing %d answers", numAnswers);

    char *serviceName[4];

    while (numAnswers--) {
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

#if DEBUG_LEVEL <= TRACE
        for (uint8_t i = 0; i < stringsRead; i++) {
            LOG(TRACE, "mdns: serviceName[%d] = %s", i, serviceName[i]);
        }
#endif

        mdnsRecordType answerType = mdns_stream_read16(buffer);
        uint16_t answerClass = mdns_stream_read16(buffer) & 0x7f; // mask out top bit: cache buster flag
        uint32_t answerTtl = mdns_stream_read32(buffer);
        uint16_t dataLength = mdns_stream_read16(buffer);

        switch(answerType) {
            case mdnsRecordTypeA: { // IPv4 Address
                uint32_t answerIP = mdns_stream_read32(buffer);
                LOG(TRACE, "mdns: Answer -> A: %d.%d.%d.%d", (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff );
                // TODO: do something with IP
            }
            case mdnsRecordTypePTR: { // Reverse lookup aka. Hostname
                char *hostname = mdns_stream_read_string(buffer, dataLength);
                if (hostname[0] == 0xc0) {
                    free(hostname); // Compressed pointer (not supported)
                }
                // TODO: do something with hostname
                LOG(TRACE, "mdns: Answer -> PTR: %s", hostname);
                free(hostname);
            }
            case mdnsRecordTypeTXT: { // Text records
                char **txt = NULL;
                uint8_t numRecords = 0;

                for (uint16_t i; i < dataLength; i++) {
                    uint8_t len = mdns_stream_read8(buffer);
                    if (len == 0) {
                        break; // no record
                    }
                    char *record = mdns_stream_read_string(buffer, len);
                    if (record[0] == '=') { // oer RFC 6763 Section 6.4
                        free(record);
                        continue;
                    }
                    i += len;

                    txt = realloc(txt, sizeof(char *) * (numRecords + 1));
                    txt[numRecords++] = record;
                }

                // TODO: do something with txt records
#if DEBUG_LEVEL <= TRACE
                LOG(TRACE, "mdns: Answer -> TXT (%d records):", numRecords);
                for(uint8_t i = 0; i < numRecords; i++) {
                    LOG(TRACE, "mdns: - %s", txt[i]);
                }
#endif

                if (numRecords > 0) {
                    for (uint8_t i; i < numRecords; i++) {
                        free(txt[i]);
                    }
                    free(txt);
                }
            }
            case mdnsRecordTypeSRV: { // Service records
                uint16_t prio = mdns_stream_read16(buffer);
                uint16_t weight = mdns_stream_read16(buffer);
                uint16_t port = mdns_stream_read16(buffer);

                uint16_t hostnameLen = mdns_stream_read16(buffer);
                char *hostname = mdns_stream_read_string(buffer, hostnameLen);

                // TODO: do something with service data
                LOG(TRACE, "mdns: Answer -> SRV: %s", hostname);

                free(hostname);               
            }

            case mdnsRecordTypeAAAA:
                LOG(TRACE, "mdns: Answer -> AAAA: skipping");
                // fallthrough

            default:
                // Ignore these, just skip over the buffer
                for (uint16_t i = 0; i < dataLength; i++) {
                    (void)mdns_stream_read8(buffer);
                }
                break;
        }
    }
    return;    
}

//
// API
//

void mdns_send_queries(mdnsHandle *handle) {
    LOG(DEBUG, "mdns: Sending queries NOT IMPLEMENTED");

    // TODO: send outstanding queries
    
    // send query, setting most significant bit in QClass to zero to get multicast responses

    // A DNS-SD query is just a PTR query to _<service>._<protocol>.local
    // -> Usually the server will send additional RRs that contain SRV, TXT and at least one A or AAAA
    // -> If that does not happen we have to send another query for those with the data from the PTR
    // A MDNS query is just a A query to <hostname>.local
}

#endif /* MDNS_ENABLE_QUERY */