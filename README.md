# PREAMBLE

This library provides a port of the W5x00 family (W5500 and W5100S) for Raspberry Pi Pico / Pico 2, enabling operation with **LwIP in NO_SYS = 0 mode on FreeRTOS**. It is a fork of <a href="https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C">WIZnet-PICO-LWIP-C</a>. The operation and usage of the examples are identical. The code and README were kept as close to the original as possible. Bug reports, reviews, and suggestions are always welcome.

# Getting Started with LwIP Examples

These sections will guide you through a series of steps from configuring development environment to running LwIP examples using the **WIZnet's ethernet products**.

- [**Development environment configuration**](#development_environment_configuration)
- [**Hardware requirements**](#hardware_requirements)
- [**LwIP example structure**](#lwip_example_structure)
- [**LwIP example testing**](#lwip_example_testing)
- [**How to use this library**](#how_to_use_this_library)



<a name="development_environment_configuration"></a>
## Development environment configuration

Pico SDK and FreeFRTOS has to be installed already. It is recommended to set **FREERTOS_KERNEL_PATH** and **PICO_SDK_PATH** as environment varialbes. It is guarded by default _pico_sdk_import.cmake_ and _FreeRTOS_Kernel_import.cmake_ files.

To test the LwIP examples, the development environment must be configured to use Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2.

These examples were tested after configuring the development environment on **Windows**. Please refer to '**Chapter 3: Installing the Raspberry Pi Pico VS Code Extension**' in the document below and configure accordingly.

- [**Getting started with Raspberry Pi Pico**][link-getting_started_with_raspberry_pi_pico]

**Visual Studio Code** was used during development and testing of LwIP examples, the guide document in each directory was prepared also base on development with Visual Studio Code. Please refer to corresponding document.



<a name="hardware_requirements"></a>
## Hardware requirements

The LwIP examples use **Raspberry Pi Pico** and **WIZnet Ethernet HAT** - ethernet I/O module built on WIZnet's [**W5100S**][link-w5100s] ethernet chip, **W5100S-EVB-Pico** - ethernet I/O module built on [**RP2040**][link-rp2040] and WIZnet's [**W5100S**][link-w5100s] ethernet chip,  **W5500-EVB-Pico** and **W55RP20-EVB-Pico** - ethernet I/O module built on [**RP2040**][link-rp2040] and WIZnet's [**W5500**][link-w5500] ethernet chip, **W5100S-EVB-Pico2** - ethernet I/O module built on [**RP2350**][link-rp2350] and WIZnet's [**W5100S**][link-w5100s] ethernet chip or **W5100S-EVB-Pico2** - ethernet I/O module built on [**RP2350**][link-rp2350] and WIZnet's [**W5100S**][link-w5100s] ethernet chip.

- [**Raspberry Pi Pico**][link-raspberry_pi_pico] or [**Raspberry Pi Pico2**][link-raspberry_pi_pico2]& [**WIZnet Ethernet HAT**][link-wiznet_ethernet_hat]
- [**W5100S-EVB-Pico**][link-w5100s-evb-pico]
- [**W5500-EVB-Pico**][link-w5500-evb-pico]
- [**W55RP20-EVB-Pico**][link-w55rp20-evb-pico]
- [**W5100S-EVB-Pico2**][link-w5100s-evb-pico2]
- [**W5500-EVB-Pico2**][link-w5500-evb-pico2]


<a name="lwip_example_structure"></a>
## LwIP example structure

Examples are available at '**pico_freertos_w5x00_sys/examples/**' directory. As of now, following examples are provided.

- [**DHCP & DNS**][link-dhcp_dns]
- [**Loopback**][link-loopback]
- [**iPerf**][link-iperf]

Note that **ioLibrary_Driver**, **pico-sdk**, **pico-extras** are needed to run LwIP examples.

- **ioLibrary_Driver** library is applicable to WIZnet's W5x00 ethernet chip.

Libraries are located in the '**pico_freertos_w5x00_sys/lib/**' directory.

- [**ioLibrary_Driver**][link-iolibrary_driver]

If you want to modify the code that MCU-dependent and use a MCU other than **RP2040**, you can modify it in the '**pico_freertos_w5x00_sys/port/**' directory.

port is located in the '**pico_freertos_w5x00_sys/port/**' directory.

- [**ioLibrary_Driver**][link-port_iolibrary_driver]
- [**lwip**][link-port_lwip]
- [**timer**][link-port_timer]



<a name="lwip_example_testing"></a>
## LwIP example testing

1. Download

If the LwIP examples are cloned, the library set as a submodule is an empty directory. Therefore, if you want to download the library set as a submodule together, clone the LwIP examples with the following Git command.

```cpp
/* Change directory */
// change to the directory to clone
cd [user path]

// e.g.
cd D:/pico_freertos_w5x00_sys

/* Clone */
git clone --recurse-submodules https://github.com/KrzySztos777/pico_freertos_w5x00_sys.git
```

With Visual Studio Code, the library set as a submodule is automatically downloaded, so it doesn't matter whether the library set as a submodule is an empty directory or not, so refer to it.

2. Setup board

Setup the board in '**CMakeLists.txt**' in '**pico_freertos_w5x00_sys/**' directory according to the evaluation board to be used referring to the following.

- WIZnet Ethernet HAT
- W5100S-EVB-Pico
- W5500-EVB-Pico
- W55RP20-EVB-Pico
- W5100S-EVB-Pico2
- W5500-EVB-Pico2

For example, when using WIZnet Ethernet HAT or W5100S-EVB-Pico :

```cpp
# Set board
set(BOARD_NAME WIZnet_Ethernet_HAT)
```

When using W5500-EVB-Pico :

```cpp
# Set board
set(BOARD_NAME W5500_EVB_PICO)
```

When using W55RP20-EVB-Pico :

```cpp
# Set board
set(BOARD_NAME W55RP20_EVB_PICO)
```

When using W5100S_EVB_PICO2 :
```cpp
# Set board
set(BOARD_NAME W5100S_EVB_PICO2)
```

When using W5500_EVB_PICO2 :
```cpp
# Set board
set(BOARD_NAME W5500_EVB_PICO2)
```

3. Test

Please refer to 'README.md' in each example directory to find detail guide for testing LwIP examples.

> ※ If the board pauses when rebooting using W55RP20-EVB-Pico, patch it as follows.
>
> ```cpp
> // Patch
> git apply ./patches/0001_pico_sdk_clocks.patch
> ```

<a name="how_to_use_this_library"></a>
## How to use port directory

Library is tried to be as-clear-as-possible. Please check examples how powerfull it may be. Below is ***absolutelly minimal example*** of using this lib:

```
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
    w5x00_start(dhcp, &ip, &nm, &gw, NULL);//or w5x00_dhcp(NULL)- but then we may don't know ip address immediatelly
    
    //That's all. now you can try to ping this ip
    
    vTaskStartScheduler();

    while(1){};
}
```
Options that you may want to change are described in **pico_freertos_w5x00_sys_options.h** file. You may do it by creating your own **w5x00opts.h** inside your project or just set this options in any header file before **pico_freertos_w5x00_sys** file is included. These are fast-described and default (best):

```
//performance options
#define W5X00_PRINTF                    NULL //for debugging- may be set to e.g. printf if you want to see logs
#define W5X00_INIT_DELAY_MS             0 //delay in ms at the beginning of the task (e.g. 1500). Just. First logs may not appear if set to 0
#define W5X00_INTERRUPT                 0 //if enabled then SPI is not polled but it waits for GPIO interrupt //LWIPER FAILS WITH THIS OPTION
#define W5X00_USE_SPI_DMA               0 //set DMA if SPI is used. Set1 to enable or 0 to disable (disabled is default)
#define W5X00_CHECK_LINK_TIMEOUT_MS     100 //if no traffic for this time in ms then check link status. if 0 then link always up
#define W5X00_SPI_SPEED                 (40 * 1000 * 1000) //spi speed

//task options
#define W5X00_POLL_SLEEP()              vTaskDelay(1) //function between SPI polling. Setting vTaskDelay(1) is wise-golden-option. IMPORTANT: You can set to taskYIELD() but every task with lower priority will be starved
#define W5X00_DRAIN_SLEEP()             NULL//function between draining packets from W5x00. NULL is fastest (wise-golden-option). May be also set to taskYIELD() or vTaskDelay(1)
#define W5X00_DONT_SET_IRQ_CB           0//if you USES IRQ right now- not only for W5x00 then you have to call W5X00_INT_CB(gpio,events) from your IRQ handler if you want use IRQ also for W5X00. 
#define W5X00_THREAD_NAME               "w5x00_thread"//name of W5x00 thread
#define W5X00_THREAD_STACKSIZE          512//stacksize for w5x00 thread
#define W5X00_THREAD_PRIO               (configMAX_PRIORITIES-2)//priority for w5x00 thread
#define W5X00_SLEEP_MS(ms)              vTaskDelay(pdMS_TO_TICKS(ms))

//prevent null ip without dhcp
#define PREVENT_NULL_IP 1
#define W5X00_DEFAULT_IP "192.168.0.13"
#define W5X00_DEFAULT_NM "255.255.255.0"
#define W5X00_DEFAULT_GW "192.168.0.1"
```
## Known bugs
If W5X00_INTERRUPT option is enabled then iperf fails. Library with default options may be threaten as production code.
<!--
Link
-->

[link-getting_started_with_raspberry_pi_pico]: https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf
[link-rp2040]: https://www.raspberrypi.org/products/rp2040/
[link-rp2350]: https://www.raspberrypi.com/products/rp2350/
[link-w5100s]: https://docs.wiznet.io/Product/iEthernet/W5100S/overview
[link-w5500]: https://docs.wiznet.io/Product/iEthernet/W5500/overview
[link-raspberry_pi_pico]: https://www.raspberrypi.org/products/raspberry-pi-pico/
[link-raspberry_pi_pico2]: https://www.raspberrypi.com/products/raspberry-pi-pico-2/
[link-wiznet_ethernet_hat]: https://docs.wiznet.io/Product/Open-Source-Hardware/wiznet_ethernet_hat
[link-w5100s-evb-pico]: https://docs.wiznet.io/Product/iEthernet/W5100S/w5100s-evb-pico
[link-w5500-evb-pico]: https://docs.wiznet.io/Product/iEthernet/W5500/w5500-evb-pico
[link-w5100s-evb-pico2]:https://docs.wiznet.io/Product/iEthernet/W5100S/w5100s-evb-pico2
[link-w5500-evb-pico2]:https://docs.wiznet.io/Product/iEthernet/W5500/w5500-evb-pico2
[link-w5500-evb-pico]: https://docs.wiznet.io/Product/iEthernet/W5500/w5500-evb-pico
[link-w55rp20-evb-pico]: https://docs.wiznet.io/Product/ioNIC/W55RP20/w55rp20-evb-pico
[link-dhcp_dns]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/tree/main/examples/dhcp_dns
[link-loopback]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/tree/main/examples/loopback
[link-iperf]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/tree/main/examples/lwiperf
[link-iolibrary_driver]: https://github.com/Wiznet/ioLibrary_Driver
[link-pico_sdk]: https://github.com/raspberrypi/pico-sdk
[link-pico_extras]: https://github.com/raspberrypi/pico-extras
[link-port_iolibrary_driver]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/tree/main/port/ioLibrary_Driver
[link-port_lwip]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/tree/main/port/lwip
[link-port_timer]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-LWIP-C/tree/main/port/timer
