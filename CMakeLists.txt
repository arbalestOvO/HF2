cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0123 NEW)

# 指向新的 GCC 工具链文件
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/arm_toolchain.cmake")

project(HF2_Debug C CXX ASM)

set(MCU_DEVICE "HC32F460JEUA")

# GCC 编译选项
add_compile_options(
        -mcpu=cortex-m4
        -mthumb                 # Cortex-M 必须
        -mfloat-abi=hard
        -mfpu=fpv4-sp-d16
        -funsigned-char
        -fshort-enums
        -gdwarf-3
        -g                      # 开启调试信息
)

# =============================================================
# FetchContent FreeRTOS
# =============================================================
set(FREERTOS_ROOT "${CMAKE_SOURCE_DIR}/middlewares/FreeRTOS-Kernel")
if(NOT EXISTS "${FREERTOS_ROOT}/tasks.c")
    message(FATAL_ERROR "FreeRTOS source not found!")
endif()

set(RTOS_SOURCES
        ${FREERTOS_ROOT}/tasks.c
        ${FREERTOS_ROOT}/queue.c
        ${FREERTOS_ROOT}/list.c
        ${FREERTOS_ROOT}/timers.c
        ${FREERTOS_ROOT}/event_groups.c
        ${FREERTOS_ROOT}/stream_buffer.c
        ${FREERTOS_ROOT}/portable/MemMang/heap_4.c
)

# 注意：FreeRTOS GCC 移植层文件与 armclang 兼容，但请确保使用的是 GCC 目录
set(RTOS_PORT_SOURCES
        ${FREERTOS_ROOT}/portable/GCC/ARM_CM4F/port.c
)

add_library(freertos STATIC ${RTOS_SOURCES} ${RTOS_PORT_SOURCES})

target_include_directories(freertos PUBLIC
        ${FREERTOS_ROOT}/include
        ${FREERTOS_ROOT}/portable/GCC/ARM_CM4F
        ${CMAKE_SOURCE_DIR}/config
)

# =============================================================
# Subdirectories
# =============================================================
add_subdirectory(middlewares/fatfs_port)
add_subdirectory(middlewares/tjpgdec)
target_include_directories(tjpgdec PRIVATE config)

# =============================================================
# MainDriver
# =============================================================

# --- Defines ---
add_compile_definitions(__DEBUG)
add_compile_definitions(HC32F460)
add_compile_definitions(USE_DDL_DRIVER)


# --- Include Paths ---
include_directories("${CMAKE_SOURCE_DIR}/common")
include_directories("${CMAKE_SOURCE_DIR}/source")
include_directories("${CMAKE_SOURCE_DIR}/driver/inc")


set(STARTUP_FILE "${CMAKE_SOURCE_DIR}/startup_hc32f460.S")

set(SOURCES
        ${STARTUP_FILE}
        "${CMAKE_SOURCE_DIR}/common/system_hc32f460.c"
)

file(GLOB DRIVER_SOURCES "${CMAKE_SOURCE_DIR}/driver/src/*.c")
file(GLOB MAIN_SOURCES "${CMAKE_SOURCE_DIR}/source/*.c")
file(GLOB ALGO_SOURCES "${CMAKE_SOURCE_DIR}/source/algo/*.c")
file(GLOB TASKS_SOURCES "${CMAKE_SOURCE_DIR}/source/tasks/*.c")
list(APPEND SOURCES ${DRIVER_SOURCES} ${MAIN_SOURCES} ${ALGO_SOURCES} ${TASK_SOURCES})

add_executable(HF2_Debug ${SOURCES})

target_link_libraries(HF2_Debug PRIVATE freertos FatFs tjpgdec)

# --- Linker Options (GCC ld) ---
# 需要将 HF2.sct (Scatter file) 转换为 HF2.ld (Linker Script)
target_link_options(HF2_Debug PRIVATE
        -T${CMAKE_SOURCE_DIR}/HF2.ld
        -Wl,-Map=${CMAKE_BINARY_DIR}/HF2_Debug.map
        -Wl,--print-memory-usage
        -Wl,--start-group
        -lc
        -lm
        -Wl,--end-group
        -Wl,--no-warn-rwx-segments
        -nostartfiles
)

# --- Post Build (Converted from fromelf to objcopy/size) ---
add_custom_command(TARGET HF2_Debug POST_BUILD
        COMMAND ${CMAKE_SIZE} "$<TARGET_FILE:HF2_Debug>"
)

add_custom_command(TARGET HF2_Debug POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex "$<TARGET_FILE:HF2_Debug>" "${CMAKE_BINARY_DIR}/HF2_Debug.hex"
)

add_custom_command(TARGET HF2_Debug POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary "$<TARGET_FILE:HF2_Debug>" "${CMAKE_BINARY_DIR}/HF2_Debug.bin"
)

# 可选：生成反汇编
add_custom_command(TARGET HF2_Debug POST_BUILD
        COMMAND ${ARM_TOOLCHAIN_PATH}/arm-none-eabi-objdump -d -S "$<TARGET_FILE:HF2_Debug>" > "${CMAKE_BINARY_DIR}/HF2_Debug.asm"
)