#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

// Enable mode with operating system
#define NO_SYS                         0
#define SYS_LIGHTWEIGHT_PROT           1

// Basic memory settings
#define MEM_ALIGNMENT                  4
#define MEM_SIZE                       8192

// TCP configuration
#define LWIP_TCP                       1
#define TCP_MSS                        (1500 /*mtu*/ - 20 /*iphdr*/ - 20 /*tcphhr*/)
#define TCP_SND_BUF                    (2 * TCP_MSS)
#define TCP_WND                        (2 * TCP_MSS)

// TCP/IP thread configuration
#define LWIP_TCPIP_CORE_LOCKING        0
#define TCPIP_THREAD_NAME              "tcpip_thread"
#define TCPIP_THREAD_STACKSIZE         1024
#define TCPIP_THREAD_PRIO              (configMAX_PRIORITIES-2)
#define TCPIP_MBOX_SIZE                32

// Queue memory configuration
#define DEFAULT_TCP_RECVMBOX_SIZE      32
#define DEFAULT_UDP_RECVMBOX_SIZE      32
#define DEFAULT_ACCEPTMBOX_SIZE        32
#define DEFAULT_RAW_RECVMBOX_SIZE      32

// Platform assert/logging handling
#define LWIP_DEBUG                     1
#define LWIP_PLATFORM_DIAG(x)          do { printf x; } while(0)
#define LWIP_PLATFORM_ASSERT(x)        do { printf("Assert failed: %s\n", x); while(1); } while(0)

// Ethernet frame padding
#define ETH_PAD_SIZE                   0 // MUST BE ZERO

// PBUF_POOL buffers (default values, you may change later)
#define PBUF_POOL_SIZE                 16
#define PBUF_POOL_BUFSIZE              512

// Other protocol options – disabled by default, you may enable later
#define LWIP_NETCONN                   1
#define LWIP_SOCKET                    1
#define LWIP_ARP                       1
#define LWIP_ICMP                      1
#define LWIP_DNS                       1
#define LWIP_DHCP                      1
#define LWIP_RAW                       1

// Disable ACD to avoid build errors
// http://lwip.100.n7.nabble.com/Build-issue-if-LWIP-DHCP-is-set-to-0-td33280.html
#define LWIP_DHCP_DOES_ACD_CHECK 0

// Disable redefinition error for 'struct timeval'
// .../pico-sdk/lib/lwip/src/include/lwip/sockets.h:531:8: error: redefinition of 'struct timeval'
#define LWIP_TIMEVAL_PRIVATE 0

// POSIX error codes are needed
#define LWIP_ERRNO_INCLUDE <errno.h>
#define LWIP_PROVIDE_ERRNO 1

// Enable callbacks
#define LWIP_NETIF_LINK_CALLBACK 1
#define LWIP_NETIF_STATUS_CALLBACK  1

#endif // __LWIPOPTS_H__