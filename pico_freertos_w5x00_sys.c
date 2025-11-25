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
#include "pico_freertos_w5x00_sys.h"
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

#include <string.h>//memcpy,memcmp-(mac)
#include "pico/unique_id.h"//for mac only
/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */

/* Network */
extern uint8_t mac[6];

/* LWIP */
struct netif g_netif;

/* pack for incoming frame */
uint8_t pack[ETHERNET_MTU+50];//50 is margin

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

 //unthreadsafe function to set mac address that can be called from any moment
void w5x00_set_mac(uint8_t setmac[6]){
    
    //if NULL or 0,0,0,0,0,0 then set default
    if(setmac==NULL || !memcmp(setmac,(uint8_t[6]){0,0,0,0,0,0},6)){
        //it begins with {0x00, 0x08, 0xDC}
        mac[0]=(uint8_t)0x00;
        mac[1]=(uint8_t)0x08;
        mac[2]=(uint8_t)0xDC;

        //it ends with last 3 bytes of pico_unique_board_id_t
        pico_unique_board_id_t uid;
        pico_get_unique_board_id(&uid);
        memcpy(mac+3,uid.id+5,3);
    }
    else//copy to local variable
        memcpy(mac,setmac,6);

    //set to W5500
    setSHAR(mac);
    ctlwizchip(CW_RESET_PHY, 0);
    
    //copy to netif
    memcpy(g_netif.hwaddr, mac, 6);
    g_netif.hwaddr_len = 6;
}

void w5x00_get_mac(uint8_t getmac[6]){
    memcpy(getmac,mac,6);
}

void w5x00_dhcp_dns_test_nosys_test()
{
    /* Initialize */
    int8_t retval = 0;
    // uint8_t *pack = malloc(ETHERNET_MTU);
    uint16_t pack_len = 0;
    struct pbuf *p = NULL;
    
    stdio_init_all();

    W5X00_SLEEP_MS(1500); // wait for 1.5 seconds

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    // Set ethernet chip MAC address
    w5x00_set_mac(mac);
    
    // Uruchamia wątek tcpip_thread()    
    tcpip_init(NULL, NULL);
    
    // Inicjalizacja interfejsu sieciowego (Twojego W5x00)
    ip4_addr_t ipaddr, netmask, gw;
    IP4_ADDR(&ipaddr, 192,168,0,41);
    IP4_ADDR(&netmask, 255,255,255,0);
    IP4_ADDR(&gw, 192,168,0,1);

    W5X00_PRINTF("Start adding netif...\n");
    vTaskDelay(pdMS_TO_TICKS(1500));

    netif_add(&g_netif, &ipaddr, &netmask, &gw, NULL, netif_initialize, tcpip_input);
    g_netif.name[0] = 'e';
    g_netif.name[1] = '0';
    
    netif_set_default(&g_netif);
    // MACRAW socket open
    retval = socket(0, Sn_MR_MACRAW, 0, 0x00);

    if (retval < 0)
    {
        W5X00_PRINTF(" MACRAW socket open failed\n");
    }
    
    netif_set_up(&g_netif);
    netif_set_link_up(&g_netif);

    W5X00_PRINTF("W5x00 init completed!\n");
    
    uint8_t getmac[6]={0,0,0,0,0,0};
    w5x00_get_mac(getmac);
    W5X00_PRINTF("getmac %02X:%02X:%02X:%02X:%02X:%02X\n",getmac[0],getmac[1],getmac[2],getmac[3],getmac[4],getmac[5]);
    W5X00_PRINTF("hwaddr %02X:%02X:%02X:%02X:%02X:%02X\n",(g_netif.hwaddr)[0],(g_netif.hwaddr)[1],(g_netif.hwaddr)[2],(g_netif.hwaddr)[3],(g_netif.hwaddr)[4],(g_netif.hwaddr)[5]);

    /* Infinite loop */
    while (1)
    {
        getsockopt(0, SO_RECVBUF, &pack_len);

        if (pack_len > 0)
        {
            pack_len = recv_lwip(0, (uint8_t *)pack, pack_len);

            if (pack_len)
            {
                p = pbuf_alloc(PBUF_RAW, pack_len, PBUF_POOL);
                pbuf_take(p, pack, pack_len);
                // free(pack);

                // pack = malloc(ETHERNET_MTU);
            }
            else
            {
                W5X00_PRINTF(" No packet received\n");
            }

            if (pack_len && p != NULL)
            {
                
                LINK_STATS_INC(link.recv);

                // if (g_netif.input(p, &g_netif) != ERR_OK)
                if (tcpip_input(p, &g_netif) != ERR_OK)
                {
                    pbuf_free(p);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(W5X00_TASK_INTERVAL_MS)); // minimalne odciążenie CPU
    }
}