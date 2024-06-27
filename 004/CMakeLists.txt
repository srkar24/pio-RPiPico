# Set minimum required version of CMake
# This line sets the minimum version of CMake required to build the project. 
# If a lower version of CMake is used, an error will be raised.
cmake_minimum_required(VERSION 3.12)

# Include build function from Pico SDK
# This line includes the build functions from the Pico SDK. 
# The location of the Pico SDK is specified in the environment 
# variable `$ENV{PICO_SDK_PATH}`.
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)

# Set name of project in PROJECT_NAME variable
# This line sets the name of the project to `pioasm` and specifies 
# the programming languages used in the project (C, C++, and ASM).
project(pwm_3leds_pio C CXX ASM)

# Set C language standard to C11
# This line sets the C language standard used in the project to C11.
set(CMAKE_C_STANDARD 11)

# Set C++ language standard to C++17
# This line sets the C++ language standard used in the project to C++17.
set(CMAKE_CXX_STANDARD 17)

# Export compile commands
# This line sets the flag to export the compile commands used during
# the build process. This can be useful for various development tools.
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Creates a pico-sdk subdir in our proj for libs
# This line creates a `pico-sdk` subdirectory in the project for the
# libraries specified by the Pico SDK.
pico_sdk_init()

# Add executable target with specified source files
# This line creates an executable target named `pioasm` using the 
# specified source files.
add_executable(${PROJECT_NAME}
    main.cpp
)

# Create C header file with the name <pio program>.pio.h
pico_generate_pio_header(${PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/pwm.pio
)

# Create map, bin, extra, uf2 files
# This line creates the specified output files (`map`, `bin`, `extra`, `uf2`)
# for the `pioasm` target.
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib and pico_multicore libraries
# This line links the project target to the `pico_stdlib` and `pico_multicore` 
# and additional i2c hardware support dependencies
# libraries.
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_pio
)

# Enable USB output and disable UART output
# This line enables USB output for the `pioasm` target and disables UART output.
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)