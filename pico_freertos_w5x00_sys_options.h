#ifndef PICO_FREERTOS_W5X00_SYS_OPTIONS_H
#define PICO_FREERTOS_W5X00_SYS_OPTIONS_H

//if you have this file then it will be included as a first
#if __has_include("w5x00opts.h")
#include "w5x00opts.h"
#endif

//below are all default options copy&paste friendly- put it in "w5x00opts.h" file
//OR: you can set your defaults options right below!

// //performance options
// #define W5X00_PRINTF                    NULL //for debugging- may be set to e.g. printf if you want to see logs
// #define W5X00_INIT_DELAY_MS             0 //delay in ms at the beginning of the task (e.g. 1500). Just. First logs may not appear if set to 0
// #define W5X00_INTERRUPT                 0 //if enabled then SPI is not polled but it waits for GPIO interrupt //LWIPER FAILS WITH THIS OPTION
// #define W5X00_USE_SPI_DMA               0 //set DMA if SPI is used. Set1 to enable or 0 to disable (disabled is default)
// #define W5X00_CHECK_LINK_TIMEOUT_MS     100 //if no traffic for this time in ms then check link status. if 0 then link always up
// #define W5X00_SPI_SPEED                 (40 * 1000 * 1000) //spi speed
#define W5X00_MAX_ETH_FRAME             1536 // max incoming ethernet frame size

//custom pinout options. DO NOT use them if you have evaluation board
// #define W5X00_SPI_PORT                  //spi0 //spi0 or spi1
// #define W5X00_PINOUT                    //18,19,16,17,20,21 //pins in order: SCK,MOSI,MISO,CS,RST,IRQ. It overrides BOARD_NAME from CMakeLists

// //task options
// #define W5X00_POLL_SLEEP()              vTaskDelay(1) //function between SPI polling. Setting vTaskDelay(1) is wise-golden-option. IMPORTANT: You can set to taskYIELD() but every task with lower priority will be starved
// #define W5X00_DRAIN_SLEEP()             NULL//function between draining packets from W5x00. NULL is fastest (wise-golden-option). May be also set to taskYIELD() or vTaskDelay(1)
// #define W5X00_DONT_SET_IRQ_CB           0//if you USES IRQ right now- not only for W5x00 then you have to call W5X00_INT_CB(gpio,events) from your IRQ handler if you want use IRQ also for W5X00. 
// #define W5X00_THREAD_NAME               "w5x00_thread"//name of W5x00 thread
// #define W5X00_THREAD_STACKSIZE          512//stacksize for w5x00 thread
// #define W5X00_THREAD_PRIO               (configMAX_PRIORITIES-2)//priority for w5x00 thread
// #define W5X00_SLEEP_MS(ms)              vTaskDelay(pdMS_TO_TICKS(ms))

// //prevent null ip without dhcp
// #define PREVENT_NULL_IP 1
// #define W5X00_DEFAULT_IP "192.168.0.13"
// #define W5X00_DEFAULT_NM "255.255.255.0"
// #define W5X00_DEFAULT_GW "192.168.0.1"

/* ----------------------------------------------------------------------------------------------------
 *  PERFORMANCE OPTIONS
 * ---------------------------------------------------------------------------------------------------- */

/**
 * @brief Logging function used by the W5X00 driver.
 *
 * Set to @c NULL to disable all debug logs Set to e.g. printf if you want to see logs.
 */
#ifndef W5X00_PRINTF
#define W5X00_PRINTF(...) do { } while(0)
#endif

/**
 * @brief Initial delay (in milliseconds) executed at the beginning of the W5X00 task. Just.
 * First logs may not appear if set to 0 or very low variable.
 *
 * Useful for hardware stabilization before starting network activity.
 * 
 */
#ifndef W5X00_INIT_DELAY_MS
#define W5X00_INIT_DELAY_MS 0
#endif

/**
 * @brief Enables GPIO interrupt-driven mode instead of SPI polling.
 *
 * - @c 1 – enable interrupt mode  
 * - @c 0 – use continuous SPI polling
 */
#ifndef W5X00_INTERRUPT
#define W5X00_INTERRUPT 0
#endif

/**
 * @brief Enables DMA for SPI transfers.
 *
 * - @c 1 – DMA enabled  
 * - @c 0 – DMA disabled
 */
#ifndef W5X00_USE_SPI_DMA
#define W5X00_USE_SPI_DMA 0
#endif

/**
 * @brief Link-check timeout (in milliseconds).
 *
 * If there is no network activity for this duration, the driver checks 
 * the PHY link status.  
 *
 * If set to @c 0, the link is always treated as UP.
 */
#ifndef W5X00_CHECK_LINK_TIMEOUT_MS
#define W5X00_CHECK_LINK_TIMEOUT_MS 100
#endif

/**
 * @brief SPI bus clock speed used for the W5X00.
 */
#ifndef W5X00_SPI_SPEED
#define W5X00_SPI_SPEED (40 * 1000 * 1000)
#endif

/**
 * @brief Maximum size of incoming ethernet frame.
 */
#ifndef W5X00_MAX_ETH_FRAME
#define W5X00_MAX_ETH_FRAME 1536
#endif

/* ----------------------------------------------------------------------------------------------------
 *  TASK OPTIONS
 * ---------------------------------------------------------------------------------------------------- */

/**
 * @brief Function executed during SPI polling loops (when interrupts are disabled).
 *
 * Default: @c vTaskDelay(1) to avoid starving others tasks with lower priority.
 * You may use @c vTaskYIELD() but other tasks with lower priority will starving (VERY NOT RECOMMENDED)
 */
#ifndef W5X00_POLL_SLEEP
#define W5X00_POLL_SLEEP() vTaskDelay(1)
#endif

/**
 * @brief Function executed while draining packets from the W5X00.
 *
 * - @c NULL is fastest. It is wise-golden-option.
 * - May be set to @c taskYIELD() or @c vTaskDelay(1) to improve task fairness (but it doesn't)
 */
#ifndef W5X00_DRAIN_SLEEP
#define W5X00_DRAIN_SLEEP() NULL
#endif

/**
 * @brief Prevents the driver from installing its own IRQ callback.
 *
 * Set to @c 1 if you already use a GPIO interrupt handler.  
 * In that case (set to 1 ), you MUST manually call W5X00_INT_CB(gpio, events) from your ISR. Otherwise not.
 */
#ifndef W5X00_DONT_SET_IRQ_CB
#define W5X00_DONT_SET_IRQ_CB 0
#endif

/**
 * @brief Name of the FreeRTOS task responsible for running the W5X00 logic.
 */
#ifndef W5X00_THREAD_NAME
#define W5X00_THREAD_NAME "w5x00_thread"
#endif

/**
 * @brief Stack size (in words) for the W5X00 task.
 */
#ifndef W5X00_THREAD_STACKSIZE
#define W5X00_THREAD_STACKSIZE 512
#endif

/**
 * @brief Priority of the W5X00 task.
 */
#ifndef W5X00_THREAD_PRIO
#define W5X00_THREAD_PRIO (configMAX_PRIORITIES - 2)
#endif

/**
 * @brief Sleep helper macro for the W5X00 code (delay in milliseconds).
 */
#ifndef W5X00_SLEEP_MS
#define W5X00_SLEEP_MS(ms) vTaskDelay(pdMS_TO_TICKS(ms))
#endif

/* =====================================================================================
 *  PREVENT NULL IP OPTIONS
 * ===================================================================================== */

/**
 * @brief Prevents using 0.0.0.0 as IP, netmask or gateway when DHCP is disabled.
 *
 * If set to @c 1, and DHCP is OFF, the driver will replace null values (0.0.0.0)  
 * with the default static IP, netmask and gateway defined below (PREVENT_DEFAULT_...)
 *
 * - @c 1 – enabled (recommended)  
 * - @c 0 – allow 0.0.0.0 values
 */
#ifndef PREVENT_NULL_IP
#define PREVENT_NULL_IP 1
#endif

/**
 * @brief Default static IP address used when PREVENT_NULL_IP is enabled.
 */
#ifndef W5X00_DEFAULT_IP
#define W5X00_DEFAULT_IP "192.168.0.13"
#endif

/**
 * @brief Default static subnet mask used when PREVENT_NULL_IP is enabled.
 */
#ifndef W5X00_DEFAULT_NM
#define W5X00_DEFAULT_NM "255.255.255.0"
#endif

/**
 * @brief Default static gateway address used when PREVENT_NULL_IP is enabled.
 */
#ifndef W5X00_DEFAULT_GW
#define W5X00_DEFAULT_GW "192.168.0.1"
#endif

#endif