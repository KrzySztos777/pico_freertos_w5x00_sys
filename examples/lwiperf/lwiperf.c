#include <stdio.h>

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

#include "w5x00opts.h"//let's use this- our options must be in ANY header file- not in *.c file
#include "pico_freertos_w5x00_sys.h"


//dns resolve headers
#include "lwip/netdb.h"
#include "lwip/ip_addr.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include <stdio.h>

//lwipperf
#include "lwip/apps/lwiperf.h"
#include "hardware/clocks.h"

void link_callback(struct netif *netif)
{
    printf("LINK CALLBACK:\n");
    if (netif_is_link_up(netif))
        printf("  Physical link is ^UP^\n");
    else 
        printf("  Physical link is _DOWN_\n");
}

void status_callback(struct netif *netif)
{
    printf("STATUS CALLBACK:\n");

    char ip[16], mask[16], gw[16];

    ip4addr_ntoa_r(netif_ip4_addr(netif), ip, sizeof(ip));
    ip4addr_ntoa_r(netif_ip4_netmask(netif), mask, sizeof(mask));
    ip4addr_ntoa_r(netif_ip4_gw(netif), gw, sizeof(gw));

    printf("  IP Address: %s\n", ip);
    printf("  Netmask:    %s\n", mask);
    printf("  Gateway:    %s\n", gw);

    //get info if dhp is on
    struct dhcp *dhcp = (struct dhcp *)netif_get_client_data(
        netif,
        LWIP_NETIF_CLIENT_DATA_INDEX_DHCP
    );

    printf("  DHCP state: %s\n", dhcp?"On":"Off");
}

int blink=500;
void led_task()
{   
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    while (true) {
        gpio_put(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(blink));
        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(blink));
    }
}

//lwiperf callback
void lwiperf_callback(void *arg,
                    enum lwiperf_report_type report_type,
                    const ip_addr_t *local_addr, u16_t local_port,
                    const ip_addr_t *remote_addr, u16_t remote_port,
                    u32_t bytes_transferred, u32_t ms_duration,
                    u32_t bandwidth_kbps)
    {
    (void)arg;//unused

    printf("LWIPERF REPORT CALLBACK:\n");

    char ip_local[16], ip_remote[16];

    ipaddr_ntoa_r(local_addr,  ip_local,  sizeof(ip_local));
    ipaddr_ntoa_r(remote_addr, ip_remote, sizeof(ip_remote));

    printf("  Local address:  %s:%u\n",  ip_local,  local_port);
    printf("  Remote address: %s:%u\n",  ip_remote, remote_port);

    printf("  Transfer:          %.2f MBytes\n", bytes_transferred / (1024.0 * 1024.0));//iperf cmd format
    printf("  Duration:          %.4f sec\n", ms_duration / 1000.0);//iperf cmd format (time may be different because uC is less accurate)
    printf("  Bandwidth:         %.2f Mbits/sec\n", bandwidth_kbps / 1000.0);//iperf cmd format

    blink=100;//blink faster- success is the master!
}

void init_callback(struct netif *netif){
    netif_set_status_callback(netif, status_callback);
    netif_set_link_callback(netif, link_callback);

    //THIS IS THE HEART OF THIS EXAMPLE (ofc lwiperf_callback may be set to NULL):
    lwiperf_start_tcp_server_default(lwiperf_callback, NULL);
}

int main(){

    stdio_init_all();
    // set_sys_clock_khz(300000, true);  // 300 MHz    

    xTaskCreate(led_task, "LED_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    //set our mac address
    uint8_t mac[6]={0x00,0x08,0xDC,0xFF,0x44,0xCC};
    w5x00_set_mac(mac);

    //THIRD OPTION- static&dhcp (use static ip until dhcp changes it)
    ip4_addr_t ip = IP4(192,168,0,13); // ip address
    ip4_addr_t nm = IP4(255,255,255,0);// netmask
    ip4_addr_t gw = IP4(192,168,0,1);  // gateaway
    int dhcp = 1;//is dhcp need to be set?
    w5x00_start(dhcp, &ip, &nm, &gw, init_callback);

    //the third one is my favourite, coz we can connect 1-to-1 uC to a computer by swapping the static ip&gw addresses
    //@up- in this example static&dhcp is *usefull*

    vTaskStartScheduler();

    while(1){};
}

//cmd:
//iperf -c 192.168.0.13 //or other ip if dhcp changed it

