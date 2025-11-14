# CMake minimum required version
cmake_minimum_required(VERSION 3.12)
 
if(${BOARD_NAME} STREQUAL WIZnet_Ethernet_HAT)
    #set(PICO_PLATFORM rp2350) #If you are using WIZnet_ethernet_HAT with Pico2, uncomment it.
    set(WIZNET_CHIP W5100S)
elseif(${BOARD_NAME} STREQUAL W5100S_EVB_PICO)
    set(WIZNET_CHIP W5100S)
elseif(${BOARD_NAME} STREQUAL W5500_EVB_PICO)
    set(WIZNET_CHIP W5500)
 
elseif(${BOARD_NAME} STREQUAL W55RP20_EVB_PICO)
    set(WIZNET_CHIP W5500)
 
elseif(${BOARD_NAME} STREQUAL W5100S_EVB_PICO2)
    set(PICO_PLATFORM rp2350)
    set(WIZNET_CHIP W5100S)
elseif(${BOARD_NAME} STREQUAL W5500_EVB_PICO2)
    set(PICO_PLATFORM rp2350)
    set(WIZNET_CHIP W5500)
else() 
    message(FATAL_ERROR "BOARD_NAME is wrong = ${BOARD_NAME}")
endif()

# Pull in SDK (must be before project)
if(NOT DEFINED PICO_SDK_PATH)
    if(DEFINED ENV{PICO_SDK_PATH})
        set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
        message(STATUS "Using PICO_SDK_PATH from environment: ${PICO_SDK_PATH}")
    else()
        message(FATAL_ERROR "PICO_SDK_PATH not set! Please set it in environment or pass -DPICO_SDK_PATH=<path> to cmake.")
    endif()
endif()

include(pico_sdk_import.cmake)
include(wiznet_pico_lwip_c_sdk_version.cmake)

# Set project name and informations
project(pico_freertos_w5x00_sys C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add it as a library
add_library(${PROJECT_NAME} INTERFACE)

# Set the project root directory if it's not already defined, as may happen if
# the tests folder is included directly by a parent project, without including
# the top level CMakeLists.txt.
if(NOT DEFINED WIZNET_DIR)
    set(WIZNET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/ioLibrary_Driver)
    message(STATUS "WIZNET_DIR = ${WIZNET_DIR}")
endif()

if(NOT DEFINED PORT_DIR)
    set(PORT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/port)
    message(STATUS "PORT_DIR = ${PORT_DIR}")
endif()

# Hardware-specific examples in subdirectories:
if(PROJECT_IS_TOP_LEVEL)
    # add_subdirectory(examples)
endif()

# Add libraries in subdirectories
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)
add_subdirectory(${PORT_DIR})

# Dodaj pliki źródłowe
target_sources(pico_freertos_w5x00_sys PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/pico_freertos_w5x00_sys.c
)

# Określ publiczne nagłówki
target_include_directories(pico_freertos_w5x00_sys INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(pico_freertos_w5x00_sys INTERFACE
    FreeRTOS-Kernel-Heap4
    IOLIBRARY_FILES 
    LWIP_FILES
    TIMER_FILES
    )

# Set compile options
add_compile_options(
        -Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )
