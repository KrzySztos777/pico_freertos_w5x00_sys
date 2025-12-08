#include <stdio.h>

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

// #include "w5x00opts.h"//commented. let's use default options
#include "pico_freertos_w5x00_sys.h"

//get info if dhcp is on - procedure
#include "lwip/netif.h"

//loopback
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"

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

//universal tcp_loopback func
int tcp_loopback(int sock)
{
    blink=100;//blink faster- success is the master!
    
    char buf[256];

    while (1)
    {
        int r = recv(sock, buf, sizeof(buf)-1, 0);

        if (r <= 0) {
            printf("Loopback: connection closed or error\n");
            return -1;  // zakończ obsługę socketu
        }

        buf[r] = 0;  // nul-terminate for printing
        printf("Loopback received: %s", buf);

        // Odesłanie danych
        char sendIntro[]="Loopback sent: ";
        if (send(sock, sendIntro, strlen(sendIntro), 0) < 0 || send(sock, buf, r, 0) < 0) {
            printf("Loopback: send() failed\n");
            return -1;
        }
    }
}

//tcp server task
void tcp_server_task(void *arg)
{
    W5X00_WAIT_LINK_UP(portMAX_DELAY);
    
    int listen_sock, client_sock;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        printf("socket() failed, errno=%d\n", errno);
        vTaskDelete(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(23);
    server_addr.sin_addr.s_addr = IPADDR_ANY;

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("bind() failed, errno=%d\n", errno);
        closesocket(listen_sock);
        vTaskDelete(NULL);
    }

    if (listen(listen_sock, 1) < 0) {
        printf("listen() failed, errno=%d\n", errno);
        closesocket(listen_sock);
        vTaskDelete(NULL);
    }

    printf("TCP server listening on port 23...\n");

    while (1)
    {
        client_sock = accept(listen_sock, (struct sockaddr *)&server_addr, &addr_len);
        if (client_sock < 0) {
            printf("accept() failed, errno=%d\n", errno);
            continue;
        }
        
        int timeout_ms = 5000;
        setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));
        setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, &timeout_ms, sizeof(timeout_ms));

        printf("Client connected\n");

        tcp_loopback(client_sock);

        closesocket(client_sock);
        printf("Client disconnected\n");
    }
}



int main(){

    stdio_init_all();

    xTaskCreate(led_task, "LED_Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    //tcp loopback server task
    xTaskCreate(tcp_server_task, "server_task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

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