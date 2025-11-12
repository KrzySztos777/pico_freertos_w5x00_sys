# for include outside's lwipopts
include_directories(${CMAKE_SOURCE_DIR})

# ioLibrary_Driver
add_library(IOLIBRARY_FILES STATIC)

if(${BOARD_NAME} STREQUAL WIZnet_Ethernet_HAT)
    #set(PICO_PLATFORM rp2350) #If you are using WIZnet_ethernet_HAT with Pico2, uncomment it.
    target_compile_definitions(IOLIBRARY_FILES PUBLIC _=W5100S)
    target_compile_definitions(IOLIBRARY_FILES PUBLIC DEVICE_BOARD_NAME=WIZnet_Ethernet_HAT)
elseif(${BOARD_NAME} STREQUAL W5100S_EVB_PICO)
    target_compile_definitions(IOLIBRARY_FILES PUBLIC _=W5100S)
    target_compile_definitions(IOLIBRARY_FILES PUBLIC DEVICE_BOARD_NAME=W5100S_EVB_PICO)
elseif(${BOARD_NAME} STREQUAL W5500_EVB_PICO)
    target_compile_definitions(IOLIBRARY_FILES PUBLIC _WIZCHIP_=W5500)
    target_compile_definitions(IOLIBRARY_FILES PUBLIC DEVICE_BOARD_NAME=W5500_EVB_PICO)
 
elseif(${BOARD_NAME} STREQUAL W55RP20_EVB_PICO)
    target_compile_definitions(IOLIBRARY_FILES PUBLIC _=W5500)
    target_compile_definitions(IOLIBRARY_FILES PUBLIC DEVICE_BOARD_NAME=W55RP20_EVB_PICO)
 
elseif(${BOARD_NAME} STREQUAL W5100S_EVB_PICO2)
    target_compile_definitions(IOLIBRARY_FILES PUBLIC _=W5100S)
    target_compile_definitions(IOLIBRARY_FILES PUBLIC DEVICE_BOARD_NAME=W5100S_EVB_PICO2)
elseif(${BOARD_NAME} STREQUAL W5500_EVB_PICO2)
    target_compile_definitions(IOLIBRARY_FILES PUBLIC _=W5500)
    target_compile_definitions(IOLIBRARY_FILES PUBLIC DEVICE_BOARD_NAME=W5500_EVB_PICO2)
else() 
    message(FATAL_ERROR "BOARD_NAME is wrong = ${BOARD_NAME}")
endif()

target_sources(IOLIBRARY_FILES PUBLIC
        ${PORT_DIR}/ioLibrary_Driver/src/w5x00_spi.c
        ${PORT_DIR}/ioLibrary_Driver/src/w5x00_gpio_irq.c
        )

if(${BOARD_NAME} STREQUAL W55RP20_EVB_PICO)
pico_generate_pio_header(IOLIBRARY_FILES ${PORT_DIR}/ioLibrary_Driver/src/wiznet_spi_pio.pio)

target_sources(IOLIBRARY_FILES PUBLIC 
        ${PORT_DIR}/ioLibrary_Driver/src/wiznet_spi_pio.c
        )
endif()
        
target_include_directories(IOLIBRARY_FILES PUBLIC
        ${WIZNET_DIR}/Ethernet
        ${PORT_DIR}/ioLibrary_Driver/inc
        ${PORT_DIR}
        )

target_link_libraries(IOLIBRARY_FILES PUBLIC
        pico_stdlib
        hardware_pio
        hardware_spi
        hardware_dma
        hardware_clocks
        )

# lwip
add_library(LWIP_FILES STATIC)

target_sources(LWIP_FILES PUBLIC
        ${PORT_DIR}/lwip/w5x00_lwip.c
        )

target_include_directories(LWIP_FILES PUBLIC
        ${WIZNET_DIR}/Ethernet
        ${PORT_DIR}
        ${PORT_DIR}/lwip
        )

target_link_libraries(LWIP_FILES PUBLIC
        ETHERNET_FILES
        pico_stdlib
        pico_lwip
        pico_lwip_nosys
        )

# timer
add_library(TIMER_FILES STATIC)

target_sources(TIMER_FILES PUBLIC
        ${PORT_DIR}/timer/timer.c
        )

target_include_directories(TIMER_FILES PUBLIC
        ${PORT_DIR}/timer
        )

target_link_libraries(TIMER_FILES PRIVATE
        pico_stdlib      
        )
