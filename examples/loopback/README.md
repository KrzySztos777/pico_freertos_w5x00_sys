# How to Test Loopback Example



## Step 1: Prepare software

The following serial terminal programs are required for Loopback example test, download and install from below links.

- [**Tera Term**][link-tera_term]
- [**Hercules**][link-hercules]



## Step 2: Prepare hardware

If you are using W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2, you can skip '1. Combine...'

1. Combine WIZnet Ethernet HAT with Raspberry Pi Pico.

2. Connect ethernet cable to WIZnet Ethernet HAT, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 ethernet port.

3. Connect Raspberry Pi Pico, W5100S-EVB-Pico or W5500-EVB-Pico to desktop or laptop using 5 pin micro USB cable. W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 require a USB Type-C cable.



## Step 3: Setup Loopback Example

1. Setup network configuration such as IP in 'loopback.c' which is the Loopback example in './examples/loopback/' directory.

Setup IP and other network settings to suit your network environment.

```cpp
ip4_addr_t ip = IP4(192,168,0,13); // ip address
ip4_addr_t nm = IP4(255,255,255,0);// netmask
ip4_addr_t gw = IP4(192,168,0,1);  // gateaway
w5x00_static(&ip,&nm,&gw,init_callback);
```

2. Allows users to choose between echoserver mode and echoclient mode.

- Setup loopback configuration in 'tcp_client_server.h'.

```cpp
//options for this example
#define TCP_SERVER          1//enable tcp server?
#define TCP_CLIENT          1//enable tcp client?

#define TCP_SERVER_PORT     23//server port

#define TCP_CLIENT_IP       "192.168.0.30"//client ip
#define TCP_CLIENT_PORT     4001//client port

#define TCP_TIMEOUT_SEC     5
```



## Step 4: Build

1. After completing the Loopback example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'loopback.uf2' is generated in './build/examples/loopback/' directory.

3. While pressing the BOOTSEL button of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 power on the board, the USB mass storage 'RPI-RP2' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

4. Drag and drop 'loopback.uf2' onto the USB mass storage device 'RPI-RP2'.



## Step 5: Upload and Run

**TCP Server mode**

1. Connect to the serial COM port of Raspberry Pi Pico, W5100S-EVB-Pico or W5500-EVB-Pico with Tera Term.

![][link-connect_to_serial_com_port]

2. Reset your board.

3. If the Loopback example works normally on Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 you can see the IP of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 and the loopback server is open.

![][link-see_network_information_of_raspberry_pi_pico_and_open_loopback_server]

4. Connect to the open loopback server using Hercules TCP client. When connecting to the loopback server, you need to enter is the IP that was configured in Step 3, the port is 5001 by default.

![][link-connect_to_loopback_server_using_hercules_tcp_client_1]

5. Once connected if you send data to the loopback server, you should be able to receive back the sent message.

![][link-receive_back_sent_message]

**TCP Client mode**

1. Connect to the serial COM port of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 with Tera Term.

![][link-connect_to_serial_com_port]

2. Reset your board.
3. Open Hercules and set the port for the W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 to connect to 5000.

![][link-hercules_server_open]

4. If the W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 works properly, you should see it connect to the server.

![][link-lwip_client_connect]

5. Once connected if you send data to the loopback client, you should be able to receive back the sent message.

![][link-client_echo_data]

<!--
Link
-->

[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-hercules]: https://www.hw-group.com/software/hercules-setup-utility
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/blob/main/static/images/loopback/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/blob/main/static/images/loopback/connect_to_serial_com_port.png
[link-see_network_information_of_raspberry_pi_pico_and_open_loopback_server]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/blob/main/static/images/loopback/see_network_information_of_raspberry_pi_pico_and_open_loopback_server.png
[link-connect_to_loopback_server_using_hercules_tcp_client_1]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/blob/main/static/images/loopback/connect_to_loopback_server_using_hercules_tcp_client.png
[link-receive_back_sent_message]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/blob/main/static/images/loopback/receive_back_sent_message.png
[link-hercules_server_open]:https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/blob/main/static/images/loopback/hercules_server_open.png
[link-lwip_client_connect]:https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/blob/main/static/images/loopback/lwip_client_connect.png
[link-client_echo_data]:https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/blob/main/static/images/loopback/client_echo_data.png
