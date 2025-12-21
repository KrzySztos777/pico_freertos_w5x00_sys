#ifndef PICO_FREERTOS_W5X00_SYS_H
#define PICO_FREERTOS_W5X00_SYS_H

#include "pico_freertos_w5x00_sys_options.h"

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "lwip/ip_addr.h"//IP4 macro

/* ----------------------------------------------------------------------------------------------------
 *  HELPFULLY MACROS
 * ---------------------------------------------------------------------------------------------------- */

//returns ipv4 struct from 4 numbers
#define IP4(a, b, c, d) ((ip4_addr_t)IPADDR4_INIT_BYTES(a, b, c, d))

//simplest static ip start- just call "w5x00_static(ip,nm,gw);" with arguments a pointer to ip addresses
#define w5x00_static(ip,nm,gw,cb)      w5x00_start(W5X00_DHCP_OFF, ip, nm, gw, cb)

//simplest start with dhcp- just call "w5x00_dhcp();"
#ifdef LWIP_DHCP
#define w5x00_dhcp(cb)                w5x00_start(W5X00_DHCP_ON, &ip_addr_any, &ip_addr_any, &ip_addr_any, cb)
#else
#define w5x00_dhcp(cb)                YOU_HAVE_TO_ENABLE___^LWIP_DHCP^___IN_"LWIOPTS.H"_IF_YOU_WANT_USE_DHCP!
#endif

//dhcp on/off for better clarity
#define W5X00_DHCP_ON                   1//human-readable argument for w5x00_start
#define W5X00_DHCP_OFF                  0//human-readable argument for w5x00_start

//wait for events macros
#define W5X00_WAIT_DONE(TICKS)          w5x00_event_wait(W5X00_EVENT_DONE, TICKS)//Wait for error or ready
#define W5X00_WAIT_READY(TICKS)         w5x00_event_wait(W5X00_EVENT_READY, TICKS)//Wait for successfull initialization (and ready)
#define W5X00_WAIT_ERROR(TICKS)         w5x00_event_wait(W5X00_EVENT_ERROR, TICKS)//Wait for error- practically can't set to ready after this anymore
#define W5X00_WAIT_LINK_UP(TICKS)       w5x00_event_wait(W5X00_EVENT_LINK_UP, TICKS)//Wait for link ^UP^
#define W5X00_WAIT_LINK_DOWN(TICKS)     w5x00_event_wait(W5X00_EVENT_LINK_DOWN, TICKS)//Wait for link _DOWN_
#define W5X00_WAIT_LINK_CHANGED(TICKS)  w5x00_event_wait(W5X00_EVENT_LINK_UP | W5X00_EVENT_LINK_DOWN, TICKS)//Wait for link ^UP^

//if you want to use IRQ- but on your own rules. Usefull only with W5X00_DONT_SET_IRQ_CB=1 option
#define W5X00_SET_CB()                  wizchip_gpio_interrupt_initialize(0, w5x00_int_handler)//use it with option W5X00_DONT_SET_IRQ_CB=1. Then call W5X00_INT_CB(gpio,events) macro INSIDE your interupt
#define W5X00_INT_CB(gpio,events)       wizchip_gpio_interrupt_callback(gpio,events)//if you have your own interrupt you have to call it inside your handler. Call this macro INSIDE your int handler if you have set W5X00_DONT_SET_IRQ_CB=1

//state shortcuts
#define W5X00_IS_DONE()                 (W5X00_IS_READY() || W5X00_IS_ERROR())
#define W5X00_IS_READY()                (w5x00_get_state(NULL)==W5X00_READY)
#define W5X00_IS_ERROR()                (w5x00_get_state(NULL)==W5X00_CHIP_INIT_FAILED || w5x00_get_state(NULL)==W5X00_CHIP_NOT_DETECTED || w5x00_get_state(NULL)==W5X00_MACRAW_SOCKET_FAILED)
#define W5X00_IS_STARTED()              (w5x00_get_state(NULL)!=W5X00_NOT_STARTED)
#define W5X00_IS_STARTING()             (w5x00_get_state(NULL)==W5X00_STARTING_IN_PROGRESS)

//link status shortcuts
#define W5X00_GET_LINK_STATUS()         w5x00_get_link_status(NULL)
#define W5X00_IS_LINK_UP()              (W5X00_GET_LINK_STATUS()==PHY_LINK_ON)
#define W5X00_IS_LINK_DOWN()            (W5X00_GET_LINK_STATUS()==PHY_LINK_OFF)

//enum of all possible states
enum w5x00_state_enum {
    W5X00_NOT_STARTED = 0,//Init function w5x00_start has not been called.
    W5X00_STARTING_IN_PROGRESS,//Initializing is started and in progress...
    W5X00_CHIP_INIT_FAILED,//Critical error. W5x00 initialization failed!
    W5X00_CHIP_NOT_DETECTED,//Critical error. W5x00 chip has not been detected!
    W5X00_MACRAW_SOCKET_FAILED,//Critical error. Can't open hardware socket on W5x00 for lwip communication!
    W5X00_READY,//Succesfully initialized and ready!
    W5X00_OK = W5X00_READY//Succesfully initialized and ready!
};

//events
#define W5X00_EVENT_DONE                (W5X00_EVENT_READY | W5X00_EVENT_ERROR)
#define W5X00_EVENT_READY               (1 << 0)
#define W5X00_EVENT_ERROR               (1 << 1)
#define W5X00_EVENT_LINK_UP             (1 << 2)
#define W5X00_EVENT_LINK_DOWN           (1 << 3)

//init callback definition.
typedef void (*w5x00_init_cb_t)(struct netif *netif_ptr);

/* ----------------------------------------------------------------------------------------------------
 *  USABLE FUNCTIONS
 * ---------------------------------------------------------------------------------------------------- */

 /**
 * @brief Starts the W5X00 network driver and its background task.
 *
 * This function initializes the W5X00 hardware, configures the network
 * interface (either static or DHCP), and launches the internal FreeRTOS
 * thread responsible for handling the W5X00 device.  
 * 
 * The function itself does not block. Once initialization is complete and
 * the driver becomes operational, the provided callback (if any) will
 * be invoked.
 *
 * @param dhcp
 *        If non-zero, DHCP is enabled and static IP parameters are
 *        applied and is set until DHCP will change them.
 *
 * @param ip
 *        Pointer to the static IPv4 address.
 *        When DHCP is disabled and @c==NULL or 0.0.0.0
 *        then W5X00_DEFAULT_IP is set (if PREVENT_NULL_IP option is set)
 *
 * @param nm
 *        Pointer to the static netmask.
 *        When DHCP is disabled and @c==NULL or 0.0.0.0
 *        then W5X00_DEFAULT_NM is set (if PREVENT_NULL_IP option is set)
 *
 * @param gw
 *        Pointer to the static gateway address.
 *        When DHCP is disabled and @c==NULL or 0.0.0.0
 *        then W5X00_DEFAULT_GW is set (if PREVENT_NULL_IP option is set)
 *
 * @param init_cb
 *        Optional initialization callback invoked once 
 *        Between netif_set_up and netif_set_link_up.
 *        Last chance to do something lwip machine will start.
 *        Some of them may be unthreadsafe- like set callbacks.
 *        @c struct netif. Pass NULL to disable the callback.
 */
void w5x00_start(int dhcp, const ip4_addr_t *ip, const ip4_addr_t *nm, const ip4_addr_t *gw, const w5x00_init_cb_t init_cb);



/**
 * @brief Sets the hardware MAC address of the W5X00 device.
 *
 * This function writes a 6-byte MAC address into the W5X00 hardware.
 * The driver does not perform validation, so the caller must ensure
 * that the provided address is valid and unique within the network.
 * Must be set before w5x00_start to not collide with lwip.
 *
 * @param mac
 *        Pointer to an array containing the 6-byte MAC address
 *        in standard order (e.g., {0x00, 0x08, 0xDC, ...}).
 *        If set to NULL or {0,0,0,0,0,0} then default is set.
 */
void w5x00_set_mac(const uint8_t mac[6]);



/**
 * @brief Retrieves the currently configured hardware MAC address.
 *
 * This function reads the 6-byte MAC address from the W5X00 device
 * and stores it in the provided buffer.
 *
 * @param mac
 *        Pointer to a 6-byte buffer where the MAC address will be stored.
 *        Must not be NULL.
 */
void w5x00_get_mac(uint8_t mac[6]);


/**
 * @brief Reads the current PHY link status from the W5X00 device.
 *
 * This function retrieves the current link state (UP or DOWN) from the
 * W5X00 hardware. If the caller provides a non-NULL pointer, the link status
 * will be written into that memory location. If @p link_status is NULL,
 * the function will simply return the status without storing it.
 *
 * @param link_status
 *        Optional pointer to a variable where the link status will be stored.
 *        - If not NULL: the function writes the current link state into it.
 *        - If NULL: the function performs no write-back.
 *
 * @return 
 *        The current link status as a numeric value:
 *        - @c 1 – link is UP  
 *        - @c 0 – link is DOWN  
 *
 *        Returned value is always valid regardless of whether
 *        @p link_status is NULL.
 */
uint8_t w5x00_get_link_status(uint8_t *link_status);


/**
 * @brief Returns the internal W5X00 network interface object.
 *
 * This function provides access to the underlying @c struct netif used by
 * the W5X00 driver. If the caller supplies a non-NULL pointer, the function
 * copies the current interface state into that structure. If @p netif_ptr
 * is NULL, the function simply returns a direct pointer to the internal
 * @c struct netif instance.
 *
 * @param netif_ptr
 *        Optional pointer to a user-provided @c struct netif.
 *        - If not NULL: the function copies the internal netif into it and 
 *          also returns this pointer.
 *        - If NULL: the function returns a pointer to the internal netif instance.
 *
 * @return 
 *        Pointer to the @c struct netif representing the active W5X00 interface.
 *        This may be either @p netif_ptr (when provided) or the internal driver instance.
 */
struct netif* w5x00_get_netif(struct netif *netif_ptr);


/**
 * @brief Retrieves the current internal W5X00 driver state.
 *
 * This function returns the current state of the W5X00 driver. If the caller
 * provides a non-NULL pointer, the state is also written into that variable.
 * If @p state_ptr is NULL, no write-back occurs and only the return value
 * should be used.
 *
 * @param state_ptr
 *        Optional pointer to a variable where the current state will be stored.
 *        - If not NULL: the internal state is copied into this variable.
 *        - If NULL: the function does not write anything.
 *
 * @return 
 *        The current driver state as a value of @c enum w5x00_state_enum.
 *
 *        This return value is always valid regardless of whether 
 *        @p state_ptr is NULL.
 */
enum w5x00_state_enum w5x00_get_state(enum w5x00_state_enum *state_ptr);


/**
 * @brief Waits for one or more W5X00 event-group bits to become set.
 *
 * This function blocks until the specified event bits appear in the global
 * W5X00 event group, or until the timeout expires. No event bits are cleared
 * by this function; clearing must be handled elsewhere if required.
 *
 * On success (all requested bits set), the function returns @c pdPASS.
 * On timeout (one or more bits not set before the timeout), the function
 * returns @c pdFAIL.
 *
 * @param wanted_bits
 *        Bitmask of event-group flags to wait for. All specified bits must be
 *        present simultaneously for the function to succeed.
 *
 * @param timeout_ticks
 *        Maximum time to wait, expressed in FreeRTOS ticks. Use 
 *        @c portMAX_DELAY to wait indefinitely.
 *
 * @return
 *        - @c pdPASS – all @p wanted_bits became set before the timeout  
 *        - @c pdFAIL – timeout occurred before all bits were set
 */
BaseType_t w5x00_event_wait(EventBits_t wanted_bits, TickType_t timeout_ticks);



//these below are private- using outside pico_freertos_w5x00_sys.c is prohibited
static void w5x00_task();
static void w5x00_check_link_status();
#if W5X00_INTERRUPT
static void w5x00_int_handler();
#endif

#endif