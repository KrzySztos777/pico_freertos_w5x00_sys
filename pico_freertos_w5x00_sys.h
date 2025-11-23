#ifndef PICO_FREERTOS_W5X00_SYS
#define PICO_FREERTOS_W5X00_SYS

#define W5X00_TASK_INTERVAL_MS          1 //interval in ms for task if polling SPI

//these two dont touch! They are ancient relics from modified code for this lib 
#define SOCKET_MACRAW                   0 //must be zero. w5x00_lwip.c is hard coded for this
#define PORT_LWIPERF                    5001 //don't care. it remains as it was

void w5x00_dhcp_dns_test_nosys_test();

#endif