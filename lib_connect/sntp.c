#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/async_context.h"
#include "lwip/apps/sntp.h"
#include "pico/util/datetime.h"
#include "pico/aon_timer.h"
#include "pico/mutex.h"
#include "pico/util/datetime.h"

#include "connect.h"

volatile bool aon_timer_is_initialised = false;
auto_init_mutex(aon_timer_mutex);

// callback for lwIP/SNTP to set the aon_timer to UTC (see lwipopts.h)
// this is called every time the application receives a valid NTP server response
void sntp_set_system_time_us(uint32_t sec, uint32_t us) {    
    static struct timespec ntp_ts;
    ntp_ts.tv_sec = sec;
    ntp_ts.tv_nsec = us * 1000;

    if (aon_timer_is_initialised) {
        // wait up to 10ms to obtain exclusive access to the aon_timer
        if (mutex_enter_timeout_ms (&aon_timer_mutex, 10)) {
            aon_timer_set_time(&ntp_ts);
            mutex_exit(&aon_timer_mutex);   // release the mutex as soon as possible
            puts("sntp_set_time: updated aon_timer from NTP");
        } else {
            puts("sntp_set_time: skipped aon_timer update (mutex busy)");
        }
    } else {
        // the aon_timer is uninitialised so we don't need exclusive access
        aon_timer_is_initialised = aon_timer_start(&ntp_ts);
        puts("sntp_set_time: initialised aon_timer from NTP");
    }
}

// callback for lwIP/SNTP to read system time (UTC) from the aon_timer
// when it needs to (eg) calculate the roundtrip transmission delay
void sntp_get_system_time_us(uint32_t *sec_ptr, uint32_t * us_ptr) {
    static struct timespec sys_ts;
    // we don't need exclusive access because we are on the background thread
    aon_timer_get_time(&sys_ts);
    *sec_ptr = sys_ts.tv_sec;
    *us_ptr = sys_ts.tv_nsec / 1000;
}

// function for user code to safely read the system time (UTC) asynchronously
int get_time_utc(struct timespec *ts_ptr) {
    int retval = 1;
    if (mutex_enter_timeout_ms(&aon_timer_mutex, 10)) {
        aon_timer_get_time(ts_ptr);
        mutex_exit(&aon_timer_mutex);
        retval = 0;
    }
    return retval;
}

// functions to asynchronously start and stop the client
static void start_sntp_cb(async_context_t *ctx, async_at_time_worker_t *p_worker) {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_init();    
}
void start_sntp() {
    puts("starting SNTP");
    static async_at_time_worker_t start_sntp_worker = { .do_work = start_sntp_cb };
    async_context_add_at_time_worker_in_ms(ctx, &start_sntp_worker, 0);
}

static void stop_sntp_cb(async_context_t *ctx, async_at_time_worker_t *p_worker) {
    sntp_stop();
}
void stop_sntp() {
    puts("stopping SNTP");
    static async_at_time_worker_t stop_sntp_worker = { .do_work = stop_sntp_cb };
    async_context_add_at_time_worker_in_ms(ctx, &stop_sntp_worker, 0);    
}