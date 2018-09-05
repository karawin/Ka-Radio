#ifndef mdns_mdns_impl_h_included
#include <mdns/mdns.h>

#include "platform.h"
#include "stream.h"
#include "dns.h"

#define MDNS_MULTICAST_ADDR 0xfb0000e0
#define MDNS_MULTICAST_TTL 60 /* seconds */
#define MDNS_PORT 5353

//
// Network related (this is implemented in libplatform)
//

// join multicast group
bool mdns_join_multicast_group(void);

// leave multicast group
bool mdns_leave_multicast_group(void);

// listen to multicast messages
mdnsUDPHandle *mdns_listen(mdnsHandle *handle);

// stop listening
void mdns_shutdown_socket(mdnsUDPHandle *pcb);

// parse and dispatch a packet (implemented here)
#if !MDNS_BROADCAST_ONLY
void mdns_parse_packet(mdnsHandle *handle, mdnsStreamBuf *buffer, ip_addr_t *ip, uint16_t port);
#endif /* !MDNS_BROADCAST_ONLY */

#endif /* mdns_mdns_impl_h_included */