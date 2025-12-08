#include <stdio.h>

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

// #include "w5x00opts.h"//commented let's use default options
#include "pico_freertos_w5x00_sys.h"

//dns resolve headers
#include "lwip/netdb.h"
#include "lwip/ip_addr.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include <stdio.h>

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

void init_callback(struct netif *netif){
    //this callback is the last chance to set something before lwip starts for good
    
    // Setting mac address may be also here, but we do it in main func
    //uint8_t mac[6]={0x00,0x08,0xDC,0xFF,0x44,0xCC};
    //w5x00_set_mac(mac);
    
    netif_set_status_callback(netif, status_callback);
    netif_set_link_callback(netif, link_callback);

    //get info if dhp is on
    struct dhcp *dhcp = (struct dhcp *)netif_get_client_data(
        netif,
        LWIP_NETIF_CLIENT_DATA_INDEX_DHCP
    );

    if(!dhcp){//if no dhcp but static- then for this example we set DNS static
        ip_addr_t dnsserver;
        ipaddr_aton("1.1.1.1", &dnsserver);
        dns_setserver(0, &dnsserver);
        ipaddr_aton("8.8.8.8", &dnsserver);
        dns_setserver(1, &dnsserver);
    }
}


int blink=500;
void led_task()
{   
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    while (true) {
        gpio_put(LED_PIN, 1);
        vTaskDelay(blink / portTICK_PERIOD_MS);
        gpio_put(LED_PIN, 0);
        vTaskDelay(blink / portTICK_PERIOD_MS);
    }
}

void test_task()
{   
    W5X00_WAIT_LINK_UP(portMAX_DELAY);
    printf("Starting DNS resolve...\n");

    const char *host = "eltin.com.pl";
    struct hostent *he = NULL;

    //retry until resolved
    while ((he = gethostbyname(host)) == NULL) {
        printf("DNS lookup failed, retrying...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    //hostent to addr list
    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;

    //we take the first result
    if (addr_list[0] != NULL) {

        ip_addr_t ip;
        ip.addr = addr_list[0]->s_addr;

        printf("Resolved \"%s\" IP: %s\n", host, ip_ntoa(&ip));

        blink=100;//blink faster- success is the master!
    }

    vTaskDelete(NULL);
}

int main(){

    stdio_init_all();

    xTaskCreate(led_task, "LED_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(test_task, "test_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    //set our mac address
    uint8_t mac[6]={0x00,0x08,0xDC,0xFF,0x44,0xCC};
    w5x00_set_mac(mac);

    //FIRST OPTION- only DHCP (simpler is only w5x00_dhcp(NULL);)
    // w5x00_dhcp(init_callback); //ip,nm and gw is 0.0.0.0 until dhcp changes them to desired ones

    //SECOND OPTION- only STATIC (may be also w5x00_static(&ip,&nm,&gw,NULL);)
    // ip4_addr_t ip = IP4(192,168,0,58); // ip address
    // ip4_addr_t nm = IP4(255,255,255,0);// netmask
    // ip4_addr_t gw = IP4(192,168,0,1);  // gateaway
    // w5x00_static(&ip,&nm,&gw,init_callback);
    
    //THIRD OPTION- static&dhcp (use static ip until dhcp changes it)
    ip4_addr_t ip = IP4(192,168,0,13); // ip address
    ip4_addr_t nm = IP4(255,255,255,0);// netmask
    ip4_addr_t gw = IP4(192,168,0,1);  // gateaway
    int dhcp = 1;//is dhcp need to be set?
    w5x00_start(dhcp, &ip, &nm, &gw, init_callback);

    //the third one is my favourite, coz we can connect 1-to-1 uC to a computer by swapping the static ip&gw addresses
    
    vTaskStartScheduler();

    while(1){};
}