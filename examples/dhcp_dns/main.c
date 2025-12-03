#include <stdio.h>

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

#include "pico_freertos_w5x00_sys.h"

int blink=100;
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

#include "lwip/netdb.h"
#include "lwip/ip_addr.h"
#include "lwip/ip4_addr.h"

void test_task()
{   
    W5X00_WAIT_READY(portMAX_DELAY);
    printf("Starting DNS resolve...\n");

    const char *host = "youtube.com";
    struct hostent *he = NULL;

    // Retry until resolved
    while ((he = gethostbyname(host)) == NULL) {
        printf("DNS lookup failed, retrying...\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // hostent → in_addr list
    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;

    // we take the first result
    if (addr_list[0] != NULL) {

        ip_addr_t ip;
        // convert socket API in_addr → lwIP ip_addr_t
        ip.addr = addr_list[0]->s_addr;

        printf("Resolved \"%s\" IP: %s\n", host, ip_ntoa(&ip));
    }

    vTaskDelete(NULL);
}

int main(){

    stdio_init_all();

    xTaskCreate(led_task, "LED_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(test_task, "test_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    //set our mac address
    uint8_t mac[6]={0x00,0x08,0xDC,0x12,0x34,0x56};
    w5x00_set_mac(mac);

    //set IP adress
    ip4_addr_t ip = IP4(192,168,0,41); // ip address
    ip4_addr_t nm = IP4(255,255,255,0);// netmask
    ip4_addr_t gw = IP4(192,168,0,1);  // gateaway
    
    //dhcp also? If static&dhcp then address will be changed from static to passed by DHCP
    int dhcp = 1;//is dhcp need to be set?

    w5x00_start(dhcp, &ip, &nm, &gw, NULL);
    
    vTaskStartScheduler();

    while(1){};
}