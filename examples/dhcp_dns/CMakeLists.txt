cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Set project name
project(dhcp_dns C CXX ASM)

# It is from Freertos-example-pico-sdk-2
add_compile_options(-Wall
    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function # we have some for the docs that aren't called
    )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

# Define the main executable
add_executable(${PROJECT_NAME}
    dhcp_dns.c
)

# Linking libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    pico_stdlib
    pico_freertos_w5x00_sys
    FreeRTOS-Kernel-Heap4
    )

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    .
    )

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})