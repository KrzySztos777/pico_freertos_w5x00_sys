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
#define W5X00_INIT_DELAY_MS             1500//delay in ms at the beginning of the task. Just.
#define W5X00_INTERRUPT                 1//if enabled then SPI is not polled but it waits for GPIO interrupt
#define W5X00_USE_SPI_DMA               1//set DMA if SPI is used
#define W5X00_CHECK_LINK_TIMEOUT_MS     100//if no traffic for this time then check link status. if 0 then link always up
#define W5X00_SPI_SPEED                 (5000 * 1000)//spi speed

//task options
#define W5X00_POLL_SLEEP()              taskYIELD() //function during SPI polling. taskYIELD fastest. May be also vTaskDelay(1). IMPORTANT: with very high priority of this task vTaskDelay(1) is recomennded to avoid starving of another ones
#define W5X00_DRAIN_SLEEP()             NULL//function during draining packets from W5x00. NULL is fastest. May be also taskYIELD or vTaskDelay(1)
#define W5X00_DONT_SET_IRQ_CB           0//if you USES IRQ right now- not only for W5x00 then you have to call W5X00_INT_CB(gpio,events) from your IRQ handler if you want use IRQ also for W5X00. 
#define W5X00_THREAD_NAME               "w5x00_thread"//name of W5x00 thread
#define W5X00_THREAD_STACKSIZE          256//stacksize for w5x00 thread
#define W5X00_THREAD_PRIO               (configMAX_PRIORITIES-2)//priority for w5x00 thread
#define W5X00_SLEEP_MS(ms)              vTaskDelay(pdMS_TO_TICKS(ms))

//events
#define W5X00_EVENT_READY               (1 << 0)
#define W5X00_EVENT_ERROR               (1 << 1)
#define W5X00_EVENT_LINK_UP             (1 << 2)
#define W5X00_EVENT_LINK_DOWN           (1 << 3)

//helpers
#define W5X00_SET_CB()                  wizchip_gpio_interrupt_initialize(0, w5x00_int_handler);//use it with option W5X00_DONT_SET_IRQ_CB=1. Then call W5X00_INT_CB(gpio,events) macro INSIDE your interupt
#define W5X00_INT_CB(gpio,events)       wizchip_gpio_interrupt_callback(gpio,events)//if you have your own interrupt you have to call it inside your handler. call this func inside your int handler if you have set W5X00_DONT_SET_IRQ_CB=1

#define W5X00_DHCP_ON                   1//human-readable argument for w5x00_start
#define W5X00_DHCP_OFF                  0//human-readable argument for w5x00_start

//wait for events macro
#define W5X00_WAIT_DONE(TICKS)          w5x00_event_wait(W5X00_EVENT_READY | W5X00_EVENT_ERROR, TICKS);//Wait for error or ready
#define W5X00_WAIT_READY(TICKS)         w5x00_event_wait(W5X00_EVENT_READY, TICKS);//Wait successfull initialization (and read)
#define W5X00_WAIT_ERROR(TICKS)         w5x00_event_wait(W5X00_EVENT_ERROR, TICKS);//Wait for error- practically can't set to ready anymore
#define W5X00_WAIT_LINK_UP(TICKS)       w5x00_event_wait(W5X00_EVENT_LINK_UP, TICKS);//Wait for link ^UP^
#define W5X00_WAIT_LINK_DOWN(TICKS)     w5x00_event_wait(W5X00_EVENT_LINK_DOWN, TICKS);//Wait for link _DOWN_

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

//shortcut for setting only static/dhcp
#ifdef LWIP_DHCP
#define w5x00_dhcp()                w5x00_start(W5X00_DHCP_ON, &ip_addr_any, &ip_addr_any, &ip_addr_any)
#else
#define w5x00_dhcp()                YOU_HAVE_TO_ENABLE___^LWIP_DHCP^___IN_"LWIOPTS.H"_IF_YOU_WANT_USE_DHCP!
#endif
#define w5x00_static(ip,nm,gw)      w5x00_start(W5X00_DHCP_OFF, ip, nm, gw)

typedef void (*init_cb_t)(struct netif *netif_arg);

static void w5x00_task();
void w5x00_start(int dhcp, ip4_addr_t *ip, ip4_addr_t *nm, ip4_addr_t *gw, void (*init_cb)(struct netif *w5x00_netif));

BaseType_t w5x00_event_wait(EventBits_t wanted_bits, TickType_t timeout_ticks);

void w5x00_check_link_status();

void w5x00_set_mac(uint8_t mac[6]);
void w5x00_get_mac(uint8_t mac[6]);

struct netif* w5x00_get_netif(struct netif *netif_ptr);

enum w5x00_state_enum w5x00_get_state(enum w5x00_state_enum *state_ptr);

#if W5X00_INTERRUPT
static void w5x00_int_handler();
#endif

#endif