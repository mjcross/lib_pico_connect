#ifndef _SNTP_H
#define _SNTP_H

#include "pico/util/datetime.h"

void start_sntp();
void stop_sntp();

int get_time_utc(struct timespec *ts_ptr);

extern volatile bool aon_timer_is_initialised;

#endif