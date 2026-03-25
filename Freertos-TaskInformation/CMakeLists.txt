cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Freertos_M4Demo)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
        FreeRtos/source/croutine.c
        FreeRtos/source/event_groups.c
        FreeRtos/source/list.c
        FreeRtos/source/queue.c
        FreeRtos/source/stream_buffer.c
        FreeRtos/source/tasks.c
        FreeRtos/source/timers.c
        FreeRtos/portable/GCC/ARM_CM4F/port.c
        FreeRtos/portable/MemMang/heap_4.c
        FreeRtos_demo/Src/FreeRtos_demo.c
        Lib/key/key.c

        MyApp/Mymain.cpp

)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
        FreeRtos/include
        FreeRtos/portable/GCC/ARM_CM4F
        FreeRtos/config
        FreeRtos_demo/Inc
        Lib/key
        
        MyApp

)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# Generate HEX file in Debug File

add_custom_command(TARGET Freertos_M4Demo POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:Freertos_M4Demo> ${CMAKE_BINARY_DIR}/Freertos_M4Demo.hex
        COMMENT "生成HEX文件: ${CMAKE_BINARY_DIR}/Freertos_M4Demo.hex"
)
