#ifndef _TCP_CLIENT_SERVER_H
#define _TCP_CLIENT_SERVER_H

#include "pico_freertos_w5x00_sys.h"

#include <stdio.h>

#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"

//options for this example
#define TCP_SERVER          1//enable tcp server?
#define TCP_CLIENT          1//enable tcp client?

#define TCP_SERVER_PORT     23//server port

#define TCP_CLIENT_IP       "192.168.0.30"//client ip
#define TCP_CLIENT_PORT     4001//client port

#define TCP_TIMEOUT_SEC     5

//functions
int tcp_loopback(int sock);
void tcp_server_task(void *arg);
void tcp_client_task(void *arg);

#endif