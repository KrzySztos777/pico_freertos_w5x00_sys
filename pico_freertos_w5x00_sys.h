#ifndef PICO_FREERTOS_W5X00_SYS
#define PICO_FREERTOS_W5X00_SYS

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "lwip/ip_addr.h"//IP4 macro

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

//performance options
#define W5X00_PRINTF                    printf//for debugging- may be set to NULL if you don't want to se it
#define W5X00_INTERRUPT                 0//if enabled then SPI is not polled but it waits for GPIO interrupt
#define W5X00_USE_SPI_DMA               1//set DMA if SPI is used
#define W5X00_CHECK_LINK_TIMEOUT_MS     100//if no traffic for this time then check link status. if 0 then link always up

//task options
#define W5X00_POLL_SLEEP()              taskYIELD() //function during SPI polling. taskYIELD fastest. May be also vTaskDelay(1). IMPORTANT: with very high priority of this task vTaskDelay(1) is recomennded to avoid starving of another ones
#define W5X00_DRAIN_SLEEP()             NULL//function during draining packets from W5x00. NULL is fastest. May be also taskYIELD or vTaskDelay(1)
#define W5X00_THREAD_NAME               "w5x00_thread"//name of W5x00 thread
#define W5X00_THREAD_STACKSIZE          1024//stacksize for w5x00 thread
#define W5X00_THREAD_PRIO               1//(configMAX_PRIORITIES-1)//priority for w5x00 thread
#define W5X00_SLEEP_MS(ms)              vTaskDelay(pdMS_TO_TICKS(ms))

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

#define W5X00_DHCP_ON       1
#define W5X00_DHCP_OFF      0

#define w5x00_dhcp()                w5x00_start(W5X00_DHCP_ON, &ip_addr_any, &ip_addr_any, &ip_addr_any)
#define w5x00_static(ip,nm,gw)      w5x00_start(W5X00_DHCP_OFF, ip, nm, gw)

static void w5x00_task();
void w5x00_start(int _dhcp, ip4_addr_t *_ip, ip4_addr_t *_nm, ip4_addr_t *_gw);

void w5x00_check_link_status();

void w5x00_set_mac(uint8_t mac[6]);
void w5x00_get_mac(uint8_t mac[6]);

#endif