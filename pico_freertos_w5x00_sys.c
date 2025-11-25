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
#include "lwip/ip_addr.h"
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
#include "lwip/ip4_addr.h"

/* IP addresses*/
ip4_addr_t ip = IP4(0,0,0,0); // ip address
ip4_addr_t nm = IP4(0,0,0,0); // netmask
ip4_addr_t gw = IP4(0,0,0,0); // gateaway
int dhcp = 0;//is dhcp need to be set?

/* pack for incoming frame */
uint8_t pack[ETHERNET_MTU+50];//50 is margin

/* state of initializing W5x00*/
enum w5x00_state_enum w5x00_state=W5X00_NOT_STARTED;//it is atomic 

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */


void w5x00_task()
{
    //w5x00 has been started
    w5x00_state=W5X00_STARTING_IN_PROGRESS;

    /* Initialize */
    int8_t retval = 0;
    // uint8_t *pack = malloc(ETHERNET_MTU);
    uint16_t pack_len = 0;
    struct pbuf *p = NULL;

    // W5X00_SLEEP_MS(1500); // wait for 1.5 seconds
    W5X00_PRINTF("W5x00 init starts...\n");

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    //try detect W5x00 chip.
    do {
        wizchip_reset();
        wizchip_initialize();
        if (wizchip_check() == 0) {
            w5x00_state = W5X00_CHIP_NOT_DETECTED;
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    } while (w5x00_state == W5X00_CHIP_NOT_DETECTED);

    // W5x00 chip detected. lets update state to the previous one
    w5x00_state=W5X00_STARTING_IN_PROGRESS;

    // Set ethernet chip MAC address
    w5x00_set_mac(mac);
    
    // Run tcpip_thread  
    tcpip_init(NULL, NULL);
    
    /////////////////////////////////////////
    //General start of preparing lwip netif//
    /////////////////////////////////////////

    //if there is no DHCP enabled then dhcp variable is set to 0- avoiding "dhcp_start" func not recognized
    #if !LWIP_DHCP
    dhcp=0;
    W5X00_PRINTF("You wanted DHCP, but there is no LWIP_DHCP set to 1 in lwipopts.h! Please make: #define LWIP_DHCP 1\n");
    #endif

    if(!dhcp){
        //we may prevent for setting ip,nm or gw to 0.0.0.0- if there is no dhcp
        #if PREVENT_NULL_IP
        if(ip4_addr_isany(&ip) || ip4_addr_isany(&nm) || ip4_addr_isany(&gw)){
            ip4addr_aton(DEFAULT_STATIC_IP,&ip);
            ip4addr_aton(DEFAULT_STATIC_NM,&nm);
            ip4addr_aton(DEFAULT_STATIC_GW,&gw);
        }
        #endif
    }

    //add netif and set it to default
    netif_add(&g_netif, &ip, &nm, &gw, NULL, netif_initialize, tcpip_input);
    g_netif.name[0] = 'e';
    g_netif.name[1] = '0';
    netif_set_default(&g_netif);

    //start dhcp if needed and is available
    #if LWIP_DHCP
    if(dhcp)
        dhcp_start(&g_netif);
    #endif

    //try to open MACRAW socket. It should always go
    do {
        retval = socket(0, Sn_MR_MACRAW, 0, 0x00);

        if (retval < 0)
        {
            W5X00_PRINTF(" MACRAW socket open failed\n");
            w5x00_state = W5X00_MACRAW_SOCKET_FAILED;
            vTaskDelay(pdMS_TO_TICKS(100));

        }
    } while (w5x00_state == W5X00_MACRAW_SOCKET_FAILED);
    
    // W5x00 MACRAW socket has been opened. Lets update state to the previous one
    w5x00_state=W5X00_STARTING_IN_PROGRESS;

    //lets set ^UP^ netif
    netif_set_up(&g_netif);
    netif_set_link_up(&g_netif);//TODO- check cable connection should be there?

    W5X00_PRINTF("W5x00 init completed!\n");
    
    uint8_t getmac[6]={0,0,0,0,0,0};
    w5x00_get_mac(getmac);
    W5X00_PRINTF("getmac %02X:%02X:%02X:%02X:%02X:%02X\n",getmac[0],getmac[1],getmac[2],getmac[3],getmac[4],getmac[5]);
    W5X00_PRINTF("hwaddr %02X:%02X:%02X:%02X:%02X:%02X\n",(g_netif.hwaddr)[0],(g_netif.hwaddr)[1],(g_netif.hwaddr)[2],(g_netif.hwaddr)[3],(g_netif.hwaddr)[4],(g_netif.hwaddr)[5]);

    //w5x00 setup finished!
    w5x00_state=W5X00_RUNNING;

    /* Infinite loop. Let's send packets to the lwip's mailbox! */
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

//unthreadsafe function to set mac address that can be called from any moment
void w5x00_set_mac(uint8_t setmac[6]){
    
    //if NULL or {0,0,0,0,0,0} then set default
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