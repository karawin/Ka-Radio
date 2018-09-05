#ifndef mdns_dns_h_included
#define mdns_dns_h_included

#include <mdns/mdns.h>
#include <stdbool.h>

typedef enum _mdnsResponseCode {
    responseCodeNoError = 0,
    responseCodeFormatError = 1,
    responseCodeServerFailure = 2,
    responseCodeNameError = 3,
    responseCodeNotImplemented = 4,
    responseCodeRefused = 5,
    responseCodeYXDomain = 6, // Domain name exists but it should not
    responseCodeZXRRSet = 7, // RR exists that should not
    responseCodeNXRRSet = 8, // RR should exist but does not
    responseCodeNotAuthoritative = 9,
    responseCodeNotInZone = 10
} mdnsResponseCode;

typedef enum _mdnsOpCode {
    opCodeQuery = 0,
    opCodeInverseQuery = 1,
    opCodeStatus = 2,
    opCodeReserved0 = 3,
    opCodeNotify = 4,
    opCodeUpdate = 5
} mdnsOpCode;

typedef struct _mdnsPacketFlags {
    bool recursionWanted:1; // completely ignore this one for MDNS
    bool isTruncated:1;     // responses are never truncated
    bool authoritative:1;   // responses are always authoritative
    mdnsOpCode opCode:4;    // only opCodeQuery allowed here
    bool isResponse:1;

    bool recursionAvailable:1; // completely ignore this one for MDNS
    bool authenticData:1;      // ignored
    bool checkingDisabled:1;   // ignored
    uint8_t padding:1;         // ignored
    mdnsResponseCode responseCode:4; // only responseCodeNoError allowed here
} __attribute__((packed)) mdnsPacketFlags;

typedef enum _mdnsRecordType {
    mdnsRecordTypeA = 0x01,
    mdnsRecordTypePTR = 0x0c,
    mdnsRecordTypeTXT = 0x10,
    mdnsRecordTypeSRV = 0x21,
    mdnsRecordTypeAAAA = 0x1c,
    mdnsRecordTypeAny = 0xff // Officially this is deceprated
} mdnsRecordType;

uint16_t mdns_sizeof_PTR(char *hostname, mdnsService **services, uint8_t numServices, mdnsService *serviceOrNull);
uint16_t mdns_sizeof_SRV(char *hostname, mdnsService **services, uint8_t numServices, mdnsService *serviceOrNull);
uint16_t mdns_sizeof_TXT(char *hostname, mdnsService **services, uint8_t numServices, mdnsService *serviceOrNull);
uint16_t mdns_sizeof_A(char *hostname);
uint16_t mdns_sizeof_AAAA(char *hostname, ip6_address_t ip);

char *mdns_make_PTR(char *buffer, uint16_t ttl, char *hostname, mdnsService **services, uint8_t numServices, mdnsService *serviceOrNull);
char *mdns_make_SRV(char *buffer, uint16_t ttl, char *hostname, mdnsService **services, uint8_t numServices, mdnsService *serviceOrNull);
char *mdns_make_TXT(char *buffer, uint16_t ttl, char *hostname, mdnsService **services, uint8_t numServices, mdnsService *serviceOrNull);
char *mdns_make_A(char *buffer, uint16_t ttl, char *hostname, ip_address_t ip);
char *mdns_make_AAAA(char *buffer, uint16_t ttl, char *hostname, ip6_address_t ip);

#endif /* mdns_dns_h_included */