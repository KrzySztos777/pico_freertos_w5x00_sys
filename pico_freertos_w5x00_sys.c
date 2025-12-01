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

#include "w5x00_gpio_irq.h"//interrrupt
/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */

/* Network */
extern uint8_t mac[6];

/* LWIP */
struct netif g_netif;

/* link status- to avoid multi set link up */
uint8_t link_status=PHY_LINK_OFF;

/* IP addresses*/
ip4_addr_t ip = IP4(0,0,0,0); // ip address
ip4_addr_t nm = IP4(0,0,0,0); // netmask
ip4_addr_t gw = IP4(0,0,0,0); // gateaway
int dhcp = 0;//is dhcp need to be set?

/* user's callback before set netif ^UP^ */
void (*init_cb)(struct netif *netif_arg)=NULL;

/* pack for incoming frame */
uint8_t pack[ETHERNET_MTU+50];//50 is margin for ethernet, VLAN, etc.

/* state of initializing W5x00*/
enum w5x00_state_enum w5x00_state=W5X00_NOT_STARTED;//it is threaten as atomic 

/* task handle for notifications (interrupt uses it) */
xTaskHandle w5x00TaskHandle=NULL;

/* events for link/up or sth else */
EventGroupHandle_t w5x00_event_group=NULL;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

void w5x00_start(int _dhcp, ip4_addr_t *_ip, ip4_addr_t *_nm, ip4_addr_t *_gw, void (*_init_cb)(struct netif *w5x00_netif))
{
    //copy argument to local
    dhcp=_dhcp;
    memcpy(&ip,_ip,sizeof(ip4_addr_t));
    memcpy(&nm,_nm,sizeof(ip4_addr_t));
    memcpy(&gw,_gw,sizeof(ip4_addr_t));
    init_cb=_init_cb;

    //init eventgroups
    w5x00_event_group = xEventGroupCreate();

    //create task
    xTaskCreate(w5x00_task, W5X00_THREAD_NAME, W5X00_THREAD_STACKSIZE, NULL, W5X00_THREAD_PRIO, &w5x00TaskHandle);

    //w5x00 has been started
    w5x00_state=W5X00_STARTING_IN_PROGRESS;
}

static void w5x00_task()
{
    /* Initialize */
    int8_t retval = 0;
    // uint8_t *pack = malloc(ETHERNET_MTU);
    uint16_t pack_len = 0;
    struct pbuf *p = NULL;
    
    W5X00_SLEEP_MS(pdMS_TO_TICKS(W5X00_INIT_DELAY_MS)); // wait some time. may be usefull
    W5X00_PRINTF("W5x00 init starts...\n");

    wizchip_spi_initialize();
    wizchip_cris_initialize();
    
    //try detect W5x00 chip.
    do {
        wizchip_reset();

        if (wizchip_initialize() == 0) {
            w5x00_state = W5X00_CHIP_INIT_FAILED;
            xEventGroupSetBits(w5x00_event_group, W5X00_EVENT_ERROR);
            W5X00_PRINTF("W5x00 initialized fail!\n");
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        else if (wizchip_check() == 0) {
            w5x00_state = W5X00_CHIP_NOT_DETECTED;
            xEventGroupSetBits(w5x00_event_group, W5X00_EVENT_ERROR);
            W5X00_PRINTF("ACCESS ERR : VERSION of W5x00 chip doesn't match!\n");
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        else{
            w5x00_state = W5X00_STARTING_IN_PROGRESS;
            xEventGroupClearBits(w5x00_event_group, W5X00_EVENT_ERROR);
        }
    } while (w5x00_state == W5X00_CHIP_NOT_DETECTED || w5x00_state == W5X00_CHIP_INIT_FAILED);

    #if !W5X00_DONT_SET_IRQ_CB && W5X00_INTERRUPT
    W5X00_SET_CB();
    #endif

    // Set ethernet chip MAC address
    w5x00_set_mac(mac);
    W5X00_PRINTF("W5X00 MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    
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

    //last chance to do something before the whole machine start
    if(init_cb!=NULL)
        init_cb(&g_netif);

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

    //PRINT MAC INFORMATION FROM EVERY PLACE- TO BE SURE
    // uint8_t getmac[6]={0,0,0,0,0,0};
    // w5x00_get_mac(getmac);
    // W5X00_PRINTF("getmac %02X:%02X:%02X:%02X:%02X:%02X\n",getmac[0],getmac[1],getmac[2],getmac[3],getmac[4],getmac[5]);
    // W5X00_PRINTF("hwaddr %02X:%02X:%02X:%02X:%02X:%02X\n",(g_netif.hwaddr)[0],(g_netif.hwaddr)[1],(g_netif.hwaddr)[2],(g_netif.hwaddr)[3],(g_netif.hwaddr)[4],(g_netif.hwaddr)[5]);
    // getmac[0]=0xFFU;getmac[0]=0U;getmac[0]=0U;getmac[0]=0U;getmac[0]=0U;getmac[0]=0U;
    // getSHAR(getmac);
    // W5X00_PRINTF("getmac %02X:%02X:%02X:%02X:%02X:%02X\n",getmac[0],getmac[1],getmac[2],getmac[3],getmac[4],getmac[5]);
    //w5x00 setup finished!

    w5x00_state=W5X00_RUNNING;
    xEventGroupSetBits(w5x00_event_group, W5X00_EVENT_READY);
    W5X00_PRINTF("W5x00 init completed!\n");

    //if no interrupt and link checking is disabled then just set link up and forgot about it
    #if !W5X00_CHECK_LINK_TIMEOUT_MS
    netif_set_link_up(&g_netif);
    xEventGroupSetBits(w5x00_event_group, W5X00_EVENT_LINK_UP);
    link_status=PHY_LINK_ON;
    #endif

    //first, initial read frame from w5x00
    getsockopt(0, SO_RECVBUF, &pack_len);

    /* Infinite loop. Let's send packets to the lwip's mailbox! */
    while (1)
    {
        #if W5X00_INTERRUPT && W5X00_CHECK_LINK_TIMEOUT_MS
        static uint32_t notify=0;//set to 0 to start link at least once
        if(!notify)
            w5x00_check_link_status();
        //if interrupts not enabled and checking link status is on
        #elif !W5X00_INTERRUPT && W5X00_CHECK_LINK_TIMEOUT_MS
        //last frame read or link check in ticks       
        static TickType_t last_read_time=(TickType_t)0;
        if(xTaskGetTickCount() - last_read_time >= pdMS_TO_TICKS(W5X00_CHECK_LINK_TIMEOUT_MS)){//if no activity for time defined in W5X00_CHECK_LINK_TIMEOUT_MS
            w5x00_check_link_status();
            last_read_time=xTaskGetTickCount();
        }
        #endif
        
        //make tcpip_input while buffer is empty
        while (pack_len > 0)
        {
            pack_len = recv_lwip(0, (uint8_t *)pack, pack_len);

            if (pack_len)
            {
                p = pbuf_alloc(PBUF_RAW, pack_len, PBUF_POOL);
                pbuf_take(p, pack, pack_len);
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

            //if too much packets you may want to give a breath
            W5X00_DRAIN_SLEEP();

            //save last read for checking link status
            #if !W5X00_INTERRUPT && W5X00_CHECK_LINK_TIMEOUT_MS
            last_read_time=xTaskGetTickCount();
            #endif

            //read until buffer is empty
            getsockopt(0, SO_RECVBUF, &pack_len);
        }

        #if !W5X00_INTERRUPT
        //let's give a breathe for this task
        W5X00_POLL_SLEEP();
        #else
            #if W5X00_CHECK_LINK_TIMEOUT_MS
            //we want to check cyclink link status
            notify=ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(W5X00_CHECK_LINK_TIMEOUT_MS));
            #else
            //we dont want to check link status. wait infinity
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            #endif
        #endif

        //check if something in buffer
        getsockopt(0, SO_RECVBUF, &pack_len);
    }
}

//wait for event. Mainly used from macro W5X00_WAIT_...
BaseType_t w5x00_event_wait(EventBits_t wanted_bits, TickType_t timeout_ticks){

    //checking if w5x00 created. If not- w5x00_start hasn't been called
    configASSERT(w5x00_event_group != NULL);

    EventBits_t bits = xEventGroupWaitBits(
        w5x00_event_group,
        wanted_bits,
        pdFALSE,        // don't clear flags
        pdTRUE,         // wait for all bits
        timeout_ticks
    );

    // Sprawdzenie, czy wszystkie wymagane bity zostały ustawione
    if ((bits & wanted_bits) == wanted_bits)
        return pdPASS;
    else
        return pdFAIL;
}

#if W5X00_INTERRUPT
//function registered as callback inside W5X00_SET_CB(). It wakes up w5x00 task
static void w5x00_int_handler(){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(w5x00TaskHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
#endif

//check link status
void w5x00_check_link_status(){
    //temporary var
    uint8_t current_link_status=PHY_LINK_OFF;

    //checing link status and setting link up/down
    if(ctlwizchip(CW_GET_PHYLINK, (void *)&current_link_status) != -1){//if link status is known
        if(current_link_status==PHY_LINK_ON && current_link_status!=link_status){//and link is up and changed
            netif_set_link_up(&g_netif);//then set link ^UP^
            xEventGroupSetBits(w5x00_event_group, W5X00_EVENT_LINK_UP);
            xEventGroupClearBits(w5x00_event_group, W5X00_EVENT_LINK_DOWN);
        }
        else if(current_link_status==PHY_LINK_OFF && current_link_status!=link_status){//otherwise if link is down and has been changed
            netif_set_link_down(&g_netif);//set link _DOWN_
            xEventGroupSetBits(w5x00_event_group, W5X00_EVENT_LINK_DOWN);
            xEventGroupClearBits(w5x00_event_group, W5X00_EVENT_LINK_UP);
        }
    //save current link status
    link_status=current_link_status;
    }
}

//unthreadsafe function to set mac address. Best option is call it only once at startup
void w5x00_set_mac(uint8_t setmac[6]){
    
    //if NULL or {0,0,0,0,0,0} then set default
    if(setmac==NULL || !memcmp(setmac,(uint8_t[6]){0,0,0,0,0,0},6)){
        //begins with {0x00, 0x08, 0xDC}
        mac[0]=(uint8_t)0x00;
        mac[1]=(uint8_t)0x08;
        mac[2]=(uint8_t)0xDC;

        //ends with last 3 bytes of pico_unique_board_id_t
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

//copy mac address to desired pointer
void w5x00_get_mac(uint8_t getmac[6]){
    memcpy(getmac,mac,6);
}

struct netif* w5x00_get_netif(struct netif *netif_ptr){
    if(netif_ptr!=NULL)
        netif_ptr=&g_netif;
    return &g_netif;
}

//get initialization state of W5X00
enum w5x00_state_enum w5x00_get_state(enum w5x00_state_enum *state_ptr)
{
    if(state_ptr!=NULL)
        *state_ptr=w5x00_state;
    return w5x00_state;
}