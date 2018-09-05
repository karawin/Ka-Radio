#include "platform.h"

#include "mdns_network.h"
#include "stream.h"
#include "debug.h"
#include "server.h"

#include <lwip/igmp.h>
#include <esp_common.h>

//
// private
//

#if !MDNS_BROADCAST_ONLY
void mdns_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *buf, ip_addr_t *ip, uint16_t port) {
    mdnsHandle *handle = (mdnsHandle *)arg;

    LOG(TRACE, "mdns: received %d bytes of data", buf->len);

    // make a stream buffer
    mdnsStreamBuf *buffer = mdns_stream_new(buf);

    // call parser
    mdns_parse_packet(handle, buffer, ip, port);

    // clean up our stream reader
    mdns_stream_destroy(buffer);
}
#endif /* MDNS_BROADCAST_ONLY */

//
// API
//

bool mdns_join_multicast_group(void) {
    ip_addr_t multicast_addr;
    multicast_addr.addr = (uint32_t) MDNS_MULTICAST_ADDR;
    
    LOG(TRACE, "mdns: joining multicast group");
    if (igmp_joingroup(IP_ADDR_ANY, &multicast_addr)!= ERR_OK) {
        return false;
    }

    return true;
}

bool mdns_leave_multicast_group(void) {
    ip_addr_t multicast_addr;
    multicast_addr.addr = (uint32_t) MDNS_MULTICAST_ADDR;

    LOG(TRACE, "mdns: leaving multicast group");
    if (igmp_leavegroup(IP_ADDR_ANY, &multicast_addr)!= ERR_OK) {
        return false;
    }

    return true;    
}

mdnsUDPHandle *mdns_listen(mdnsHandle *handle) {
    ip_addr_t multicast_addr;
    multicast_addr.addr = (uint32_t) MDNS_MULTICAST_ADDR;

    LOG(TRACE, "mdns: listening on MDNS port on %d.%d.%d.%d",
        (multicast_addr.addr & 0xff),
        ((multicast_addr.addr >> 8) & 0xff),
        ((multicast_addr.addr >> 16) & 0xff),
        ((multicast_addr.addr >> 24) & 0xff)
    );

    struct udp_pcb *pcb = udp_new();
    pcb->ttl = MDNS_MULTICAST_TTL;

    err_t err = udp_bind(pcb, IP_ADDR_ANY, MDNS_PORT);
    if (err != ERR_OK) {
        LOG(ERROR, "Could not listen to UDP port");
        return NULL;
    }

#if !MDNS_BROADCAST_ONLY
    LOG(TRACE, "mdns: setting up receive callback");
    udp_recv(pcb, mdns_recv_callback, (void *)handle);
#endif /* MDNS_BROADCAST_ONLY */

    LOG(TRACE, "mdns: connecting");
    udp_connect(pcb, &multicast_addr, MDNS_PORT);
    return pcb;

}

uint16_t mdns_send_udp_packet(mdnsHandle *handle, char *data, uint16_t len) {
    struct pbuf * buf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if (pbuf_take(buf, data, len) != ERR_OK) {
        LOG(ERROR, "mdns: pbuf not big enough");
    }

    // HEXDUMP(DEBUG, "mdns: UDP Packet", data, len);
    // LOG(TRACE, "mdns: sending packet (%d bytes)", len);

    // actually send it
    udp_send(handle->pcb, buf);
    
    pbuf_free(buf);
    free(data);
    return len;
}

void mdns_shutdown_socket(mdnsUDPHandle *pcb) {
    LOG(TRACE, "mdns: shutting down socket");
    udp_disconnect(pcb);
    udp_remove(pcb);
}
