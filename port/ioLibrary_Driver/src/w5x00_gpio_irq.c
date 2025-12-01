/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "wizchip_conf.h"
#include "socket.h"
#include "w5x00_gpio_irq.h"

#include "pico_freertos_w5x00_sys.h"//w5x00_get_status()

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
static void (*callback_ptr)(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* GPIO */
void wizchip_gpio_interrupt_initialize(uint8_t socket, void (*callback)(void))
{
    uint16_t reg_val;
    // int ret_val;

    reg_val = (SIK_CONNECTED | SIK_DISCONNECTED | SIK_RECEIVED | SIK_TIMEOUT); // except SendOK
    /*ret_val = */ctlsocket(socket, CS_SET_INTMASK, (void *)&reg_val);

#if (_WIZCHIP_ == W5100S)
    reg_val = (1 << socket);
#elif (_WIZCHIP_ == W5500)
    reg_val = ((1 << socket) << 8);
#endif
    /*ret_val = */ctlwizchip(CW_SET_INTRMASK, (void *)&reg_val);

    //set callback
    callback_ptr = callback;

    //prepare PIN_INT
    gpio_init(PIN_INT);
    gpio_set_dir(PIN_INT, GPIO_IN);
    gpio_pull_up(PIN_INT);

    #if !W5X00_DONT_ATTACH_IRQ
    gpio_set_irq_enabled_with_callback(PIN_INT, GPIO_IRQ_EDGE_FALL, true, &wizchip_gpio_interrupt_callback);
    #endif
}

void wizchip_gpio_interrupt_callback(uint gpio, uint32_t events)
{
    #if W5X00_DONT_SET_IRQ_CB
    //check if pin belongs to W5x00 PIN_INT. Otherwise it must belonds to it- then nothing to check
    if(gpio==PIN_INT && w5x00_get_state()==W5X00_READY)
    #endif
    {
        //rearm interrupt
        uint8_t sn_ir = 0;
        ctlsocket(0, CS_GET_INTERRUPT, &sn_ir);
        if (sn_ir) {
            ctlsocket(0, CS_CLR_INTERRUPT, &sn_ir);
        }
        //below chip-level rearm is not needed
        // uint8_t cir = 0;
        // ctlwizchip(CW_GET_INTERRUPT, &cir);
        // if (cir) {
        //     ctlwizchip(CW_CLR_INTERRUPT, &cir);
        // }

        //send callback
        if (callback_ptr != NULL)
        {
            callback_ptr();
        }
    }
}