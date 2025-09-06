# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

# Set Pico board type
set(PICO_BOARD pico CACHE STRING "Board type")

# Set C/C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Completely disable picotool and other tools download to avoid network issues
set(PICO_SKIP_TOOLS_PACKAGE_SEARCH 1)
set(SKIP_TOOL_RISCV_OPENOCD 1)
set(SKIP_TOOL_PICOTOOL 1)
set(PICO_SDK_FETCH_FROM_GIT 0)
set(PICOTOOL_FETCH_FROM_GIT 0)
set(PICO_SKIP_TOOL_CHECKS 1)
set(PICO_NO_HARDWARE 0)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================


# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pio_st7789_lcd C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# CUSTOM
# Add ST7789 library
add_library(st7789_lib
    src/st7789.cpp
    src/st7789_hal.cpp
    src/st7789_gfx.cpp
    src/st7789_font.cpp
)

# CUSTOM
# Set ST7789 library include directories
target_include_directories(st7789_lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# CUSTOM
# Set ST7789 library link libraries
target_link_libraries(st7789_lib
    pico_stdlib
    hardware_spi
    hardware_gpio
    hardware_dma
    hardware_pio
)

# Add executable. Default name is the project name, version 0.1
add_executable(pio_st7789_lcd st7789_lcd.cpp)

pico_generate_pio_header(pio_st7789_lcd ${CMAKE_CURRENT_LIST_DIR}/st7789_lcd.pio)

# CUSTOM
# LCD demo program include directories
target_include_directories(pio_st7789_lcd PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# CUSTOM
# Link libraries required by LCD demo program
target_link_libraries(pio_st7789_lcd
    st7789_lib
    pico_stdlib
    hardware_spi
    hardware_gpio
    hardware_pio
)

# Configure standard input/output for LCD demo program
pico_enable_stdio_usb(pio_st7789_lcd 1)
pico_enable_stdio_uart(pio_st7789_lcd 0)

#target_sources(pio_st7789_lcd PRIVATE st7789_lcd.c)

#target_link_libraries(pio_st7789_lcd PRIVATE pico_stdlib hardware_pio hardware_interp)



pico_add_extra_outputs(pio_st7789_lcd)

# add url via pico_set_program_url
