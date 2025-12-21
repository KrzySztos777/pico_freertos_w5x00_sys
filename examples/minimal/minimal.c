#include <stdio.h>

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

#include "pico_freertos_w5x00_sys.h"

int main(){

    stdio_init_all();

    ip4_addr_t ip = IP4(192,168,0,13); // ip address
    ip4_addr_t nm = IP4(255,255,255,0);// netmask
    ip4_addr_t gw = IP4(192,168,0,1);  // gateaway
    int dhcp = 0;//is dhcp need to be set?
    w5x00_start(dhcp, &ip, &nm, &gw, NULL);//or w5x00_dhcp(NULL)- but then we may don't know ip address
    
    //That's all. now you can try to ping this ip
    
    vTaskStartScheduler();

    while(1){};
}