/*
 * Copyright 2016 karawin (http://www.karawin.fr)
*/
#pragma once

#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__


#include "webclient.h"
#include "vs1053.h"

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "esp_common.h"
#include "esp_softap.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "flash.h"
#include "eeprom.h"
#include "interface.h"
#include "websocket.h"

void serverTask(void *pvParams);

#endif