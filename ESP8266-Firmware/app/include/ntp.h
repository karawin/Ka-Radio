//////////////////////////////////////////////////
// Simple NTP client for ESP8266.
// Copyright 2016 jp cocatrix (KaraWin)
// jp@karawin.fr
// See license.txt for license terms.
//////////////////////////////////////////////////

#ifndef __NTP_H__
#define __NTP_H__
#include <time.h>

typedef struct {
	uint8 options;
	uint8 stratum;
	uint8 poll;
	uint8 precision;
	uint32 root_delay;
	uint32 root_disp;
	uint32 ref_id;
	uint8 ref_time[8];
	uint8 orig_time[8];
	uint8 recv_time[8];
	uint8 trans_time[8];
} ntp_t;

//void ntpTask(void *pvParams);

// print locale date time in ISO-8601 local time
bool ntp_get_time(struct tm **dt);
void ntp_print_time();
#endif