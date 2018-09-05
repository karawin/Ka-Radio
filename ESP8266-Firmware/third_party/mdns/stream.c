#include "stream.h"
#include <stdlib.h>

#include "platform.h"

// read 16 bit int from stream
uint16_t mdns_stream_read16(mdnsStreamBuf *buffer) {
    return (mdns_stream_read8(buffer) << 8) \
            + mdns_stream_read8(buffer);
}

// read 32 bit int from stream
uint32_t mdns_stream_read32(mdnsStreamBuf *buffer) {
        return (mdns_stream_read8(buffer) << 24) \
            + (mdns_stream_read8(buffer) << 16) \
            + (mdns_stream_read8(buffer) << 8) \
            + mdns_stream_read8(buffer);
}

// caller has to free response
char *mdns_stream_read_string(mdnsStreamBuf *buffer, uint16_t len) {
    char *result = malloc(len + 1);
	uint8_t i;	
    for (i = 0; i < len; i++) {
        result[i] = mdns_stream_read8(buffer);
    }
    result[len] = '\0';
}
