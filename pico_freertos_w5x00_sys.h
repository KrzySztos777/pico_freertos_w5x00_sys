#ifndef PICO_FREERTOS_W5X00_SYS
#define PICO_FREERTOS_W5X00_SYS

#include <stdint.h>
#include "FreeRTOS.h"
#include "event_groups.h"

#define IP4(a, b, c, d) ((ip4_addr_t)IPADDR4_INIT_BYTES(a, b, c, d))

enum w5x00_state_enum {
    W5X00_NOT_STARTED = 0,
    W5X00_STARTING_IN_PROGRESS,
    W5X00_CHIP_INIT_FAILED,//it is critical. W5x00 initialization failed
    W5X00_CHIP_NOT_DETECTED,//it is critical. No W5x00 chip detected!
    W5X00_MACRAW_SOCKET_FAILED,//it is critical. Can't open socket on W5x00 for lwip communication
    W5X00_RUNNING,
    W5X00_OK = W5X00_RUNNING
};

#define W5X00_PRINTF                    printf//for debugging- may be set to NULL if you don't want to se it
#define W5X00_TASK_INTERVAL_MS          1 //interval in ms for task if polling SPI
#define W5X00_USE_SPI_DMA               1 //set DMA if SPI is used

//we may prevent for setting ip,nm or gw to 0.0.0.0- if there is no dhcp
#ifndef PREVENT_NULL_IP
#define PREVENT_NULL_IP     1
#endif

#ifndef DEFAULT_STATIC_IP
#define DEFAULT_STATIC_IP   "192.168.0.13"
#endif
#ifndef DEFAULT_STATIC_NM
#define DEFAULT_STATIC_NM   "255.255.255.0"
#endif
#ifndef DEFAULT_STATIC_GW
#define DEFAULT_STATIC_GW   "192.168.0.1"
#endif

void w5x00_task();

void w5x00_set_mac(uint8_t mac[6]);
void w5x00_get_mac(uint8_t mac[6]);

#endif