#include <string.h>
#include <stdlib.h>

#include <mdns/mdns.h>

#include "server.h"
#include "debug.h"


mdnsService *mdns_create_service(char *name, mdnsProtocol protocol, uint16_t port) {
    mdnsService *service = calloc(1, sizeof(mdnsService));
    service->name = strdup(name);
    service->protocol = protocol;
    service->port = port;

    return service;
}

void mdns_service_add_txt(mdnsService *service, char *key, char *value) {
    service->txtRecords = realloc(service->txtRecords, sizeof(mdnsTxtRecord) * (service->numTxtRecords + 1));
    
    service->txtRecords[service->numTxtRecords].name = strdup(key);
    service->txtRecords[service->numTxtRecords].value = strdup(value);
    service->numTxtRecords++;
}

void mdns_service_destroy(mdnsService *service) {
	uint8_t i;
    for ( i = 0; i < service->numTxtRecords; i++) {
        free(service->txtRecords[i].name);
        free(service->txtRecords[i].value);
    }
    free(service->txtRecords);
    free(service->name);
    free(service);
}


#if MDNS_ENABLE_PUBLISH

void mdns_add_service(mdnsHandle *handle, mdnsService *service) {
    if (handle->services) {
        handle->services = realloc(handle->services, sizeof(mdnsService *) * (handle->numServices + 1));
    } else {
        handle->services = malloc(sizeof(mdnsService *) * (handle->numServices + 1));
    }
    handle->services[handle->numServices] = service;
    handle->numServices++;

    if (handle->started) {
        xQueueSend(handle->mdnsQueue, (void *)mdnsTaskActionRestart, portMAX_DELAY);
    }
}

void mdns_remove_service(mdnsHandle *handle, mdnsService *service) {
	uint8_t i,j;
    for(i = 0; i < handle->numServices; i++) {
        if (handle->services[i] == service) {
            for (j = i + 1; j < handle->numServices - 1; j++) {
                handle->services[i] = handle->services[j];
            }
        }
    }
    handle->services = realloc(handle->services, sizeof(mdnsService *) * (handle->numServices - 1));
    handle->numServices--;

    if (handle->started) {
        xQueueSend(handle->mdnsQueue, (void *)mdnsTaskActionRestart, portMAX_DELAY);    
    }
}

#endif /* MDNS_ENABLE_PUBLISH */
