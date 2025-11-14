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

#include "port_common.h"

#include "wizchip_conf.h"
#include "socket.h"
#include "w5x00_spi.h"
#include "w5x00_lwip.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"

#include "lwip/apps/lwiperf.h"
#include "lwip/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"

//NO_SYS=0
#include "lwip/tcpip.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Socket */
#define SOCKET_MACRAW 0

/* Port */
#define PORT_LWIPERF 5001

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/* Network */
extern uint8_t mac[6];

/* LWIP */
struct netif g_netif;

/* DNS */
static uint8_t g_dns_target_domain[] = "www.eltin.com.pl";
static uint8_t g_dns_get_ip_flag = 0;
static uint32_t g_ip;
static ip_addr_t g_resolved;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */

uint8_t pack[ETHERNET_MTU+50];//50 is margin

void w5x00_dhcp_dns_test_nosys_test()
{
    /* Initialize */
    int8_t retval = 0;
    // uint8_t *pack = malloc(ETHERNET_MTU);
    uint16_t pack_len = 0;
    struct pbuf *p = NULL;

    set_clock_khz();

    // Initialize stdio after the clock change
    stdio_init_all();

    W5X00_SLEEP_MS(1000 * 3); // wait for 3 seconds

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    // Set ethernet chip MAC address
    setSHAR(mac);
    ctlwizchip(CW_RESET_PHY, 0);

    // // Initialize LWIP in NO_SYS mode
    // lwip_init();

    // netif_add(&g_netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY, NULL, netif_initialize, netif_input);
    // g_netif.name[0] = 'e';
    // g_netif.name[1] = '0';

    // // Assign callbacks for link and status
    // netif_set_link_callback(&g_netif, netif_link_callback);
    // netif_set_status_callback(&g_netif, netif_status_callback);

    // // MACRAW socket open
    // retval = socket(SOCKET_MACRAW, Sn_MR_MACRAW, PORT_LWIPERF, 0x00);

    // if (retval < 0)
    // {
    //     W5X00_PRINTF(" MACRAW socket open failed\n");
    // }

    // // Set the default interface and bring it up
    // netif_set_default(&g_netif);
    // netif_set_link_up(&g_netif);
    // netif_set_up(&g_netif);

    // Uruchamia wątek tcpip_thread()
    W5X00_PRINTF("tcpip_init goes...\n");
    vTaskDelay(pdMS_TO_TICKS(1500));

    tcpip_init(NULL, NULL);

    // Inicjalizacja interfejsu sieciowego (Twojego W5x00)
    ip4_addr_t ipaddr, netmask, gw;
    IP4_ADDR(&ipaddr, 192,168,0,40);
    IP4_ADDR(&netmask, 255,255,255,0);
    IP4_ADDR(&gw, 192,168,0,1);

    W5X00_PRINTF("Start adding netif...\n");
    vTaskDelay(pdMS_TO_TICKS(1500));

    netif_add(&g_netif, &ipaddr, &netmask, &gw, NULL, netif_initialize/*wizchip_netif_init*/, tcpip_input);
    g_netif.name[0] = 'e';
    g_netif.name[1] = '0';
    netif_set_default(&g_netif);
    netif_set_link_callback(&g_netif, netif_link_callback);
    netif_set_status_callback(&g_netif, netif_status_callback);
    netif_set_up(&g_netif);

    // Start DHCP configuration for an interface
    dhcp_start(&g_netif);

    dns_init();

    W5X00_PRINTF("W5x00 init completed!\n");
    vTaskDelay(pdMS_TO_TICKS(1500));


    /* Infinite loop */
    while (1)
    {
        // getsockopt(SOCKET_MACRAW, SO_RECVBUF, &pack_len);

        // if (pack_len > 0)
        // {
        //     pack_len = recv_lwip(SOCKET_MACRAW, (uint8_t *)pack, pack_len);

        //     if (pack_len)
        //     {
        //         p = pbuf_alloc(PBUF_RAW, pack_len, PBUF_POOL);
        //         pbuf_take(p, pack, pack_len);
        //         // free(pack);

        //         // pack = malloc(ETHERNET_MTU);
        //     }
        //     else
        //     {
        //         W5X00_PRINTF(" No packet received\n");
        //     }

        //     if (pack_len && p != NULL)
        //     {
        //         LINK_STATS_INC(link.recv);

        //         if (g_netif.input(p, &g_netif) != ERR_OK)
        //         {
        //             pbuf_free(p);
        //         }
        //     }
        // }

        getsockopt(SOCKET_MACRAW, SO_RECVBUF, &pack_len);
        if (pack_len > 0) {
            pack_len = recv_lwip(SOCKET_MACRAW, pack, pack_len);
            if (pack_len > 0) {
                p = pbuf_alloc(PBUF_RAW, pack_len, PBUF_POOL);
                if (p != NULL) {
                    pbuf_take(p, pack, pack_len);
                    if (tcpip_input(p, &g_netif) != ERR_OK) {
                        pbuf_free(p);
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1)); // minimalne odciążenie CPU

        // if ((dns_gethostbyname(g_dns_target_domain, &g_resolved, NULL, NULL) == ERR_OK) && (g_dns_get_ip_flag == 0))
        // {
        //     g_ip = g_resolved.addr;

        //     W5X00_PRINTF(" DNS success\n");
        //     W5X00_PRINTF(" Target domain : %s\n", g_dns_target_domain);
        //     W5X00_PRINTF(" IP of target domain : [%03d.%03d.%03d.%03d]\n", g_ip & 0xFF, (g_ip >> 8) & 0xFF, (g_ip >> 16) & 0xFF, (g_ip >> 24) & 0xFF);

        //     g_dns_get_ip_flag = 1;
        // }

        /* Cyclic lwIP timers check */
        // sys_check_timeouts();
    }
}

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}
