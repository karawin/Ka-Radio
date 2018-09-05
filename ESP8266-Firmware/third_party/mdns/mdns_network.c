#include "debug.h"
#include "mdns_network.h"
#include "mdns_query.h"
#include "mdns_publish.h"

#include "stream.h"
#include "server.h"

#if !MDNS_BROADCAST_ONLY
void mdns_parse_packet(mdnsHandle *handle, mdnsStreamBuf *buffer, ip_addr_t *ip, uint16_t port) {
    uint16_t transactionID = mdns_stream_read16(buffer);
    uint16_t flagsTmp = mdns_stream_read16(buffer);
    mdnsPacketFlags flags;
    memcpy(&flags, &flagsTmp, 2);

    // MDNS only supports opCode 0 -> query, and non-error response codes
    if ((flags.opCode != opCodeQuery) || (flags.responseCode != responseCodeNoError)) {
        mdns_stream_destroy(buffer);
        return;
    }

    uint16_t numQuestions = mdns_stream_read16(buffer);
    uint16_t numAnswers = mdns_stream_read16(buffer);
    uint16_t numAuthorityRR = mdns_stream_read16(buffer);
    uint16_t numAdditionalRR = mdns_stream_read16(buffer);

    // MDNS Answer flag set -> read answers
    if (flags.isResponse) {
#if defined(MDNS_ENABLE_QUERY) && MDNS_ENABLE_QUERY
        // Read answers, additional sideloaded records will be appended after the answer
        // so we can parse them with one parser
        if (handle->numQueries > 0) {
            // only waste the processing power if we have outstanding queries actually
            mdns_parse_answers(buffer, numAnswers + numAdditionalRR);
        }
#endif /* MDNS_ENABLE_QUERY */
    } else {
#if defined(MDNS_ENABLE_PUBLISH) && MDNS_ENABLE_PUBLISH
        // we have to listen to queries all the time as a host may have missed our
        // announce packet.
        mdns_parse_query(handle, buffer, numQuestions, transactionID);
#endif /* MDNS_ENABLE_PUBLISH */
    }
}
#endif /* !MDNS_BROADCAST_ONLY */
