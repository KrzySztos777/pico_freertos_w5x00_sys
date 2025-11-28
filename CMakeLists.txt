# CMake minimum required version
cmake_minimum_required(VERSION 3.12)

#CMake's imports
include(pico_sdk_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

# Set project name and informations
project(pico_freertos_w5x00_sys C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add it as a library
add_library(${PROJECT_NAME} INTERFACE)

# ioLibrary_Driver uses WIZNET_DIR.
if(NOT DEFINED WIZNET_DIR)
    set(WIZNET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/ioLibrary_Driver)
    message(STATUS "WIZNET_DIR = ${WIZNET_DIR}")
endif()

# pass defines and CMake variables to ioLibrary_Driver
if(${BOARD_NAME} STREQUAL WIZnet_Ethernet_HAT)
    #set(PICO_PLATFORM rp2350) #If you are using WIZnet_ethernet_HAT with Pico2, uncomment it.
    set(WIZNET_CHIP W5100S)
    target_compile_definitions(${PROJECT_NAME} INTERFACE _WIZCHIP_=W5100S)
    target_compile_definitions(${PROJECT_NAME} INTERFACE DEVICE_BOARD_NAME=WIZnet_Ethernet_HAT)
elseif(${BOARD_NAME} STREQUAL W5100S_EVB_PICO)
    set(WIZNET_CHIP W5100S)
    target_compile_definitions(${PROJECT_NAME} INTERFACE _WIZCHIP_=W5100S)
    target_compile_definitions(${PROJECT_NAME} INTERFACE DEVICE_BOARD_NAME=W5100S_EVB_PICO)
elseif(${BOARD_NAME} STREQUAL W5500_EVB_PICO)
    set(WIZNET_CHIP W5500)
    target_compile_definitions(${PROJECT_NAME} INTERFACE _WIZCHIP_=W5500)
    target_compile_definitions(${PROJECT_NAME} INTERFACE DEVICE_BOARD_NAME=W5500_EVB_PICO)
elseif(${BOARD_NAME} STREQUAL W55RP20_EVB_PICO)
    set(WIZNET_CHIP W5500)
    target_compile_definitions(${PROJECT_NAME} INTERFACE _WIZCHIP_=W5500)
    target_compile_definitions(${PROJECT_NAME} INTERFACE DEVICE_BOARD_NAME=W55RP20_EVB_PICO)
 
elseif(${BOARD_NAME} STREQUAL W5100S_EVB_PICO2)
    set(PICO_PLATFORM rp2350)
    set(WIZNET_CHIP W5100S)
    target_compile_definitions(${PROJECT_NAME} INTERFACE _WIZCHIP_=W5100S)
    target_compile_definitions(${PROJECT_NAME} INTERFACE DEVICE_BOARD_NAME=W5100S_EVB_PICO2)
elseif(${BOARD_NAME} STREQUAL W5500_EVB_PICO2)
    set(PICO_PLATFORM rp2350)
    set(WIZNET_CHIP W5500)
    target_compile_definitions(${PROJECT_NAME} INTERFACE _WIZCHIP_=W5500)
    target_compile_definitions(${PROJECT_NAME} INTERFACE DEVICE_BOARD_NAME=W5500_EVB_PICO2)
else() 
    message(FATAL_ERROR "BOARD_NAME is wrong = ${BOARD_NAME}")
endif()

# Add libraries in subdirectories
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib) #ioLibrary_Driver

# Hardware-specific examples in subdirectories:
if(PROJECT_IS_TOP_LEVEL)
    # add_subdirectory(examples)
endif()

# Add source files
target_sources(${PROJECT_NAME} PUBLIC
    #current project
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.c

    #port
    ${CMAKE_CURRENT_SOURCE_DIR}/port/ioLibrary_Driver/src/w5x00_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/port/ioLibrary_Driver/src/w5x00_gpio_irq.c
    ${CMAKE_CURRENT_SOURCE_DIR}/port/lwip/w5x00_lwip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/port/timer/timer.c
)

# Conditionally generate pio
if(${BOARD_NAME} STREQUAL W55RP20_EVB_PICO)
    pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/port/ioLibrary_Driver/src/wiznet_spi_pio.pio)

    target_sources(${PROJECT_NAME} PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/port/ioLibrary_Driver/src/wiznet_spi_pio.c
    )
endif()

# Define public header files
target_include_directories(${PROJECT_NAME} INTERFACE
    # current project
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    
    # ioLibrary_Driver
    ${WIZNET_DIR}/Ethernet

    # port
    ${CMAKE_CURRENT_SOURCE_DIR}/port/ioLibrary_Driver/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/port/timer
    ${CMAKE_CURRENT_SOURCE_DIR}/port/lwip
    ${CMAKE_CURRENT_SOURCE_DIR}/port
)

# Link libraries
target_link_libraries(${PROJECT_NAME} INTERFACE
    # ioLibrary_Driver
    ETHERNET_FILES

    # port
    pico_stdlib
    hardware_pio
    hardware_spi
    hardware_dma
    hardware_clocks

    #freertos & lwip
    FreeRTOS-Kernel
    pico_lwip_freertos

    #last 3 bytes copied to the end of default mac address
    pico_unique_id 
    )

# Set compile options
add_compile_options(
    -Wall
    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function # we have some for the docs that aren't called
    -Wno-maybe-uninitialized
    )
