#define NODEBUG // suppress debug "noise"

// --- allow the 'extended' network status callback
#define LWIP_NETIF_EXT_STATUS_CALLBACK     1

// --- two extra system timeouts (MQTT + SNTP) ---
#define MEMP_NUM_SYS_TIMEOUT        (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 2)

// --- lwIP MQTT settings ---
#define MQTT_REQ_MAX_IN_FLIGHT      5
#define MQTT_OUTPUT_RINGBUF_SIZE    256     // transmit buffer size: at least (max_topic + max_data)
#define MQTT_VAR_HEADER_BUFFER_LEN  128     // receive buffer size: to avoid frags at least (max_topic + max_data + 8)

// --- lwIP SNTP settings ---
#define SNTP_MAX_SERVERS            LWIP_DHCP_MAX_NTP_SERVERS
#define SNTP_GET_SERVERS_FROM_DHCP  LWIP_DHCP_GET_NTP_SRV
#define SNTP_SERVER_DNS             1
#define SNTP_SERVER_ADDRESS         "pool.ntp.org"
#define SNTP_DEBUG                  LWIP_DBG_OFF
#define SNTP_PORT                   LWIP_IANA_PORT_SNTP
//* SNTP_CHECK_RESPONSE >= 2 requires SNTP_GET_SYSTEM_TIME()
#define SNTP_CHECK_RESPONSE         2

// do NOT use SNTP roundtrip compensation with the rp2040 aon_timer
// because it has a resolution of only 1 second (see CMakeLists.txt)
#ifdef DISABLE_SNTP_COMP_ROUNDTRIP
#define   SNTP_COMP_ROUNDTRIP       0
#else
#define   SNTP_COMP_ROUNDTRIP       1
#endif

#define SNTP_STARTUP_DELAY          1
#define SNTP_STARTUP_DELAY_FUNC     (LWIP_RAND() % 5000)
#define SNTP_RECV_TIMEOUT           15000
//* time between queries (millisec)
#define SNTP_UPDATE_DELAY           3600000
#define SNTP_RETRY_TIMEOUT          SNTP_RECV_TIMEOUT
#define SNTP_RETRY_TIMEOUT_MAX      (SNTP_RETRY_TIMEOUT * 10)
#define	SNTP_RETRY_TIMEOUT_EXP      1
#define SNTP_MONITOR_SERVER_REACHABILITY    1

//* configure SNTP to use our callback functions for reading and setting the system time
#define SNTP_GET_SYSTEM_TIME(sec, us)  sntp_get_system_time_us(&(sec), &(us))
#define SNTP_SET_SYSTEM_TIME_US(sec, us)   sntp_set_system_time_us(sec, us)

//* declare our callback functions (the implementations are in the .c file)
#include "stdint.h"
void sntp_set_system_time_us(uint32_t sec, uint32_t us);
void sntp_get_system_time_us(uint32_t *sec_ptr, uint32_t *us_ptr);