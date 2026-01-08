#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// allow override in some examples
#ifndef NO_SYS
#define NO_SYS                      1
#endif
// allow override in some examples
#ifndef LWIP_SOCKET
#define LWIP_SOCKET                 0
#endif
#if PICO_CYW43_ARCH_POLL
#define MEM_LIBC_MALLOC             1
#else
// MEM_LIBC_MALLOC is incompatible with non polling versions
#define MEM_LIBC_MALLOC             0
#endif
#define MEM_ALIGNMENT               4
#ifndef MEM_SIZE
#define MEM_SIZE                    4000
#endif
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_ARP_QUEUE          10
#define PBUF_POOL_SIZE              24
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_MSS                     1460
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define LWIP_NETIF_STATUS_CALLBACK  0
#define LWIP_NETIF_LINK_CALLBACK    0
#define LWIP_NETIF_HOSTNAME         0
#define LWIP_NETCONN                0
#define MEM_STATS                   0
#define SYS_STATS                   0
#define MEMP_STATS                  0
#define LINK_STATS                  0
// #define ETH_PAD_SIZE                2
#define LWIP_CHKSUM_ALGORITHM       3
#define LWIP_DHCP                   1
#define LWIP_IPV4                   1
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_DNS                    1
#define LWIP_TCP_KEEPALIVE          1
#define LWIP_NETIF_TX_SINGLE_PBUF   1
#define DHCP_DOES_ARP_CHECK         0
#define LWIP_DHCP_DOES_ACD_CHECK    0

#ifndef NDEBUG
#define LWIP_DEBUG                  1
#define LWIP_STATS                  1
#define LWIP_STATS_DISPLAY          1
#endif

#define ETHARP_DEBUG                LWIP_DBG_OFF
#define NETIF_DEBUG                 LWIP_DBG_OFF
#define PBUF_DEBUG                  LWIP_DBG_OFF
#define API_LIB_DEBUG               LWIP_DBG_OFF
#define API_MSG_DEBUG               LWIP_DBG_OFF
#define SOCKETS_DEBUG               LWIP_DBG_OFF
#define ICMP_DEBUG                  LWIP_DBG_OFF
#define INET_DEBUG                  LWIP_DBG_OFF
#define IP_DEBUG                    LWIP_DBG_OFF
#define IP_REASS_DEBUG              LWIP_DBG_OFF
#define RAW_DEBUG                   LWIP_DBG_OFF
#define MEM_DEBUG                   LWIP_DBG_OFF
#define MEMP_DEBUG                  LWIP_DBG_OFF
#define SYS_DEBUG                   LWIP_DBG_OFF
#define TCP_DEBUG                   LWIP_DBG_OFF
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF
#define TCP_RTO_DEBUG               LWIP_DBG_OFF
#define TCP_CWND_DEBUG              LWIP_DBG_OFF
#define TCP_WND_DEBUG               LWIP_DBG_OFF
#define TCP_FR_DEBUG                LWIP_DBG_OFF
#define TCP_QLEN_DEBUG              LWIP_DBG_OFF
#define TCP_RST_DEBUG               LWIP_DBG_OFF
#define UDP_DEBUG                   LWIP_DBG_OFF
#define TCPIP_DEBUG                 LWIP_DBG_OFF
#define PPP_DEBUG                   LWIP_DBG_OFF
#define SLIP_DEBUG                  LWIP_DBG_OFF
#define DHCP_DEBUG                  LWIP_DBG_OFF

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
//* SNTP_COMP_ROUNDTRIP requires SNTP_GET_SYSTEM_TIME()
#define SNTP_COMP_ROUNDTRIP         1
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

#endif /* __LWIPOPTS_H__ */
