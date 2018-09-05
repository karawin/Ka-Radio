#include <mdns/mdns.h>
#include <ctype.h>

#include "mdns_network.h"
#include "mdns_query.h"
#include "mdns_publish.h"
#include "server.h"
#include "debug.h"

void mdns_server_task(void *userData) {
    mdnsHandle *handle = userData;
    mdnsTaskAction action = mdnsTaskActionNone;
    int tmp;
    LOG(TRACE, "mdns: Service task started");    

    while (1) {
#if MDNS_BROADCAST_ONLY
        // wait a maximum of 30 seconds then re-announce
        if (xQueueReceive(handle->mdnsQueue, &tmp, 3000) == pdFALSE) {
            tmp = mdnsTaskActionRestart; // if nothing happens for about 30 seconds, re-announce
        }
#else
        // wait until we should do something
        xQueueReceive(handle->mdnsQueue, &tmp, portMAX_DELAY);
#endif
        action = (mdnsTaskAction)tmp;

        // destroy messages are for the caller, not for us
        if (action == mdnsTaskActionDestroy) {    
            // re-insert into queue
            xQueueSendToBack(handle->mdnsQueue, &action, portMAX_DELAY);

            // give up time slot
            taskYIELD();

            continue;
        }

        switch (action) {
            case mdnsTaskActionStart:
                // start up service
                if (!mdns_join_multicast_group()) {
                    LOG(ERROR, "mdns: Joining multicast group failed");
                }
                handle->pcb = mdns_listen(handle);
#if MDNS_ENABLE_PUBLISH
                // and announce the services on the network
                mdns_announce(handle);
#endif
                handle->started = true;
                break;

            case mdnsTaskActionStop:
                // cleanly shut down, this means sending a goodbye message
#if MDNS_ENABLE_PUBLISH
                mdns_goodbye(handle);
#endif
                // shutdown socket
                mdns_shutdown_socket(handle->pcb);
                handle->pcb = NULL;
                if (!mdns_leave_multicast_group()) {
                    LOG(ERROR, "mdns: Leaving multicast group failed");
                }
                // notify parent and destroy this task
                action = mdnsTaskActionDestroy;
                xQueueSendToBack(handle->mdnsQueue, &action, portMAX_DELAY);
                handle->started = false;
                vTaskDelete(NULL);
                break;

            case mdnsTaskActionRestart:
                // just force an announcement
#if MDNS_ENABLE_PUBLISH
                mdns_announce(handle);
#endif
                break;
            
#if MDNS_ENABLE_QUERY
            case mdnsTaskActionQuery:
                // send all registered queries
                mdns_send_queries(handle);
                break;
#endif /* MDNS_ENABLE_QUERY */

            default:
                break;
        }
    }
}

#if MDNS_ENABLE_QUERY
void mdns_add_query(mdnsHandle *handle, mdnsQueryHandle *query) {
    // TODO: mutex lock handle->queries
    handle->queries = realloc(handle->queries, sizeof(mdnsQueryHandle *) * (handle->numQueries + 1));
    handle->queries[handle->numQueries] = query;
    handle->numQueries++;

    LOG(DEBUG, "mdns: adding query: %s", query->service);
    
    mdnsTaskAction action = mdnsTaskActionQuery;
    xQueueSendToBack(handle->mdnsQueue, &action, portMAX_DELAY);
}

void mdns_remove_query(mdnsHandle *handle, mdnsQueryHandle *query) {
    // TODO: mutex lock handle->queries
    for(uint8_t i = 0; i < handle->numQueries; i++) {
        if (handle->queries[i] == query) {
            LOG(DEBUG, "mdns: removing query: %s", query->service);
            for (uint8_t j = i + 1; j < handle->numQueries - 1; j++) {
                handle->queries[i] = handle->queries[j];
            }
        }
    }
    handle->queries = realloc(handle->queries, sizeof(mdnsQueryHandle *) * (handle->numQueries - 1));
    handle->numQueries--;
}
#endif /* MDNS_ENABLE_QUERY */

//
// API
//

mdnsHandle *mdns_create(char *hostname) {
    LOG(DEBUG, "mdns: creating MDNS service for %s", hostname);
    
    mdnsHandle *handle = calloc(1, sizeof(mdnsHandle));

    // duplicate hostname and convert to lowercase
    uint8_t hostnameLen = strlen(hostname);
    handle->hostname = malloc(hostnameLen + 1);
	uint8_t i;
    for (i = 0; i < hostnameLen; i++) {
        handle->hostname[i] = tolower(hostname[i]);
    }
    handle->hostname[hostnameLen] = '\0';
    
    handle->started = false;
    
    handle->mdnsQueue = xQueueCreate(1, sizeof(int));
    return handle;
}

// Start broadcasting MDNS records
void mdns_start(mdnsHandle *handle) {
    LOG(DEBUG, "mdns: Starting service");

    if (xTaskCreate(mdns_server_task, "mdns", 250, handle, 3, &handle->mdnsTask) != pdPASS) {
        LOG(ERROR, "mdns: Could not create service, terminating");
        mdns_destroy(handle);
    }
    mdnsTaskAction action = mdnsTaskActionStart;
    xQueueSendToBack(handle->mdnsQueue, &action, portMAX_DELAY);
    LOG(TRACE, "mdns: Service started");    
}

// Stop broadcasting MDNS records
void mdns_stop(mdnsHandle *handle) {
    LOG(DEBUG, "mdns: Stopping service");

    mdnsTaskAction action = mdnsTaskActionStop;
    xQueueSendToBack(handle->mdnsQueue, &action, portMAX_DELAY);

    // wait for mdns service to stop
    action = mdnsTaskActionNone;
    mdnsTaskAction *ptr = &action;
    while (action != mdnsTaskActionDestroy) {
        taskYIELD();
        xQueuePeek(handle->mdnsQueue, ptr, portMAX_DELAY);
    }
    // FIXME: clear queue
    LOG(TRACE, "mdns: Service stopped");    
}

// Restart MDNS service (call on IP/Network change)
void mdns_restart(mdnsHandle *handle) {
    LOG(DEBUG, "mdns: Restarting service");

    mdnsTaskAction action = mdnsTaskActionRestart;
    xQueueSendToBack(handle->mdnsQueue, &action, portMAX_DELAY);

    LOG(TRACE, "mdns: Service restarted");
}

// Update IP
void mdns_update_ip(mdnsHandle *handle, const ip_address_t ip, const ip6_address_t ip6) {
    LOG(DEBUG, "mdns: Updating IPv4 to %d.%d.%d.%d", ip.addr8[0], ip.addr8[1], ip.addr8[2], ip.addr8[3]);
    LOG(DEBUG, "mdns: Updating IPv6 to %x:%x:%x:%x", ip6.addr[0], ip6.addr[1], ip6.addr[2], ip6.addr[3]);


    if (memcmp(&handle->ip, &ip, sizeof(ip_address_t) != 0) ||
        memcmp(&handle->ip6, &ip6, sizeof(ip6_address_t)) != 0) {
        
        bool restart = handle->started;
        if (restart) {
            mdns_stop(handle);
        }
        memcpy(&handle->ip, &ip, sizeof(ip_address_t));
        memcpy(&handle->ip6, &ip6, sizeof(ip6_address_t));
        if (restart) {
            mdns_start(handle);
        }
    }
}

// Destroy MDNS handle
void mdns_destroy(mdnsHandle *handle) {
    LOG(DEBUG, "mdns: Destroying service handle");

    // shut down task
    if (handle->mdnsTask) {
        mdns_stop(handle);
    }

    // destroy all services
    uint8_t numServices = handle->numServices;
    handle->numServices = 0;
	uint8_t i;	
    for(i = 0; i < numServices; i++) {
        mdns_service_destroy(handle->services[i]);
    }
    free(handle->services);

    // free hostname
    free(handle->hostname);

    // free complete handle
    free(handle);
}