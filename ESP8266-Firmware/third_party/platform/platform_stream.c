#include "platform.h"
#include "freertos/FreeRTOS.h"
#include "stream.h"
#include "debug.h"

//
// private
//

static bool mdns_advance_buffer(mdnsStreamBuf *buffer) {
    struct pbuf *next = buffer->bufList->next;
    if (next == NULL) {
        return false;
    }
    pbuf_ref(next);
    pbuf_free(buffer->bufList);
    buffer->bufList = next;
    buffer->currentPosition = 0;

    return true;
}

//
// API
//

// create stream reader
mdnsStreamBuf *mdns_stream_new(mdnsNetworkBuffer *buffer) {
    mdnsStreamBuf *buf = malloc(sizeof(mdnsStreamBuf));
    pbuf_ref(buffer);

    buf->bufList = buffer;
    buf->currentPosition = 0;
}

// read byte from stream
uint8_t mdns_stream_read8(mdnsStreamBuf *buffer) {
    if (buffer->currentPosition >= buffer->bufList->len - 1) {
        mdns_advance_buffer(buffer);
    }
    char *payload = buffer->bufList->payload;
    return payload[buffer->currentPosition++];
}

// destroy stream reader
void mdns_stream_destroy(mdnsStreamBuf *buffer) {
    pbuf_free(buffer->bufList);
    free(buffer);
}
