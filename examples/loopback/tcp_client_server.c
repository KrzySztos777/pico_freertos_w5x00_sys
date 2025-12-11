#include "tcp_client_server.h"

extern int blink;

//universal tcp_loopback func
int tcp_loopback(int sock)
{
    blink=100;//blink faster- success is the master!

    char buf[256];

    char hello[]="Hello to W5x00 loopback example!\n";
    if (send(sock, hello, strlen(hello), 0) < 0) {
        printf("Loopback: send() failed\n");
        return -1;
    }
        
    while (1)
    {
        int r = recv(sock, buf, sizeof(buf)-1, 0);

        if (r <= 0) {
            printf("Loopback: connection closed or error\n");
            return -1;//end socket handling
        }

        buf[r] = 0;//null-terminate for printing
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

    //create socket
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        printf("socket() failed, errno=%d\n", errno);
        vTaskDelete(NULL);
    }

    //prepare sockaddr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_SERVER_PORT);
    server_addr.sin_addr.s_addr = IPADDR_ANY;

    //binding
    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("bind() failed, errno=%d\n", errno);
        closesocket(listen_sock);
        vTaskDelete(NULL);
    }

    //try to listen
    if (listen(listen_sock, 1) < 0) {
        printf("listen() failed, errno=%d\n", errno);
        closesocket(listen_sock);
        vTaskDelete(NULL);
    }

    //socket is listening
    printf("TCP server listening on port %d...\n",TCP_SERVER_PORT);

    //incoming connections handling- this example handles only one connection at a time
    while (1)
    {
        client_sock = accept(listen_sock, (struct sockaddr *)&server_addr, &addr_len);
        if (client_sock < 0) {
            printf("accept() failed, errno=%d\n", errno);
            continue;
        }

        //timeouts for receive and send- both the same
        struct timeval tv;
        tv.tv_sec = TCP_TIMEOUT_SEC;//seconds
        tv.tv_usec = 0;//milis
        setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        //client connected
        printf("Server accepted connection from %s:%d\n",ipaddr_ntoa((const ip_addr_t*)&server_addr.sin_addr),htons(server_addr.sin_port));

        //let's handle it with our universal handler
        tcp_loopback(client_sock);

        //closee client's socket
        closesocket(client_sock);
        printf("Server disconnected with client %s:%d\n",ipaddr_ntoa((const ip_addr_t*)&server_addr.sin_addr),htons(server_addr.sin_port));
    }
}

//tcp client task with reconnect loop
void tcp_client_task(void *arg)
{
    W5X00_WAIT_LINK_UP(portMAX_DELAY);
    
    while (1)
    {
        int sock;
        struct sockaddr_in server_addr;
        socklen_t addr_len = sizeof(server_addr);

        //create socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            printf("socket() failed, errno=%d\n", errno);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue; //try again
        }

        //prepare sockaddr
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(TCP_CLIENT_PORT);//port
        server_addr.sin_addr.s_addr = inet_addr(TCP_CLIENT_IP);//address

        printf("Connecting to %s:%d server...\n",ipaddr_ntoa((const ip_addr_t*)&server_addr.sin_addr),TCP_CLIENT_PORT);

        //try to connect
        if (connect(sock, (struct sockaddr *)&server_addr, addr_len) < 0) {
            printf("connect() failed, errno=%d. Retrying...\n", errno);
            closesocket(sock);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue; //connection failed → retry
        }

        //set receive/send timeouts= both the same
        struct timeval tv;
        tv.tv_sec = TCP_TIMEOUT_SEC;//seconds
        tv.tv_usec = 0;//millis
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        //connected
        printf("Connected to %s:%d server!\n",ipaddr_ntoa((const ip_addr_t*)&server_addr.sin_addr),TCP_CLIENT_PORT);

        //let's handle it with our universal handler
        tcp_loopback(sock);

        //close socket and retry connection
        closesocket(sock);
        printf("Disconnected. Reconnecting...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}