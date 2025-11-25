#ifndef PICO_FREERTOS_W5X00_SYS
#define PICO_FREERTOS_W5X00_SYS

#include <stdint.h>

#define W5X00_TASK_INTERVAL_MS          1 //interval in ms for task if polling SPI

void w5x00_set_mac(uint8_t mac[6]);
void w5x00_get_mac(uint8_t mac[6]);

void w5x00_dhcp_dns_test_nosys_test();

#endif