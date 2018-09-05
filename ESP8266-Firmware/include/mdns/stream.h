#ifndef mdns_stream_h_included
#define mdns_stream_h_included

#include "platform.h"

typedef struct _mdnsStreamBuf mdnsStreamBuf;

// create stream reader (this is implemented in libplatform)
mdnsStreamBuf *mdns_stream_new(mdnsNetworkBuffer *buffer);

// read byte from stream (this is implemented in libplatform)
uint8_t mdns_stream_read8(mdnsStreamBuf *buffer);

// read 16 bit int from stream
uint16_t mdns_stream_read16(mdnsStreamBuf *buffer);

// read 32 bit int from stream
uint32_t mdns_stream_read32(mdnsStreamBuf *buffer);

// caller has to free response
char *mdns_stream_read_string(mdnsStreamBuf *buffer, uint16_t len);

// destroy stream reader (this is implemented in libplatform)
void mdns_stream_destroy(mdnsStreamBuf *buffer);

#endif /* mdns_stream_h_included */