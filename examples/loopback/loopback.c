#include <stdio.h>

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

// #include "w5x00opts.h"//commented. let's use default options
#include "pico_freertos_w5x00_sys.h"

//get info if dhcp is on - procedure
#include "lwip/netif.h"

//tcp client/server
#include "tcp_client_server.h"

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

void init_callback(struct netif *netif){
    netif_set_status_callback(netif, status_callback);
    netif_set_link_callback(netif, link_callback);
}

int main(){

    stdio_init_all();

    xTaskCreate(led_task, "LED_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    //tcp loopback server and client task
    #if TCP_SERVER
    xTaskCreate(tcp_server_task, "server_task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    #endif
    #if TCP_CLIENT
    xTaskCreate(tcp_client_task, "client_task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    #endif

    //set our mac address
    uint8_t mac[6]={0x00,0x08,0xDC,0xFF,0x44,0xCC};
    w5x00_set_mac(mac);

    //SECOND OPTION- only STATIC (may be also w5x00_static(&ip,&nm,&gw,NULL);)
    ip4_addr_t ip = IP4(192,168,0,13); // ip address
    ip4_addr_t nm = IP4(255,255,255,0);// netmask
    ip4_addr_t gw = IP4(192,168,0,1);  // gateaway
    w5x00_static(&ip,&nm,&gw,init_callback);

    vTaskStartScheduler();

    while(1){};
}