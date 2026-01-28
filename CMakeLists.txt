cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0123 NEW)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/arm_toolchain.cmake")
project(HF2_Debug C CXX ASM)

set(MCU_DEVICE "HC32F460JEUA")

# 定义 CPU 和 FPU 参数
# -mfloat-abi=hard: 使用硬件浮点寄存器传参
# -mfpu=fpv4-sp-d16: 指定 Cortex-M4F 单精度 FPU
add_compile_options(
        -mcpu=cortex-m4
        -mfloat-abi=hard
        -mfpu=fpv4-sp-d16
        -fno-rtti
        -funsigned-char
        -fshort-enums
        -fshort-wchar
        -gdwarf-3
)

add_compile_options($<$<COMPILE_LANGUAGE:ASM>:${CPU_FLAGS}>)

# =============================================================
# FetchContent FreeRTOS
# =============================================================
set(FREERTOS_ROOT "${CMAKE_SOURCE_DIR}/middlewares/FreeRTOS-Kernel")
if(NOT EXISTS "${FREERTOS_ROOT}/tasks.c")
    message(FATAL_ERROR "FreeRTOS source not found! Did you run 'git submodule update --init --recursive'?")
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

set(RTOS_PORT_SOURCES
        ${FREERTOS_ROOT}/portable/GCC/ARM_CM4F/port.c
)

# 创建 FreeRTOS 静态库
add_library(freertos STATIC ${RTOS_SOURCES} ${RTOS_PORT_SOURCES})

target_include_directories(freertos PUBLIC
        ${FREERTOS_ROOT}/include
        ${FREERTOS_ROOT}/portable/GCC/ARM_CM4F
        ${CMAKE_SOURCE_DIR}/config
)

# =============================================================
# FatFs
# =============================================================
add_subdirectory(middlewares/fatfs_port)

# TjpgDec
add_subdirectory(middlewares/tjpgdec)
target_include_directories(tjpgdec PRIVATE config)

# =============================================================
# MainDriver
# =============================================================

# --- Defines ---
add_compile_definitions(__DEBUG)
add_compile_definitions(HC32F460)
add_compile_definitions(USE_DDL_DRIVER)
add_definitions(-D__CLANG_ARM)

# --- Include Paths ---
include_directories("${CMAKE_SOURCE_DIR}/./common")
include_directories("${CMAKE_SOURCE_DIR}/./source")
include_directories("${CMAKE_SOURCE_DIR}/./driver/inc")

set(SOURCES
        "${CMAKE_SOURCE_DIR}/startup_hc32f460.s"
        "${CMAKE_SOURCE_DIR}/common/system_hc32f460.c"
        "${CMAKE_SOURCE_DIR}/source/main.c"
        source/ring_buffer.c
)

# 自动获取 driver/src 目录下所有的 .c 文件
file(GLOB DRIVER_SOURCES "${CMAKE_SOURCE_DIR}/driver/src/*.c")
file(GLOB MAIN_SOURCES "${CMAKE_SOURCE_DIR}/source/*.c")
file(GLOB ALGO_SOURCES "${CMAKE_SOURCE_DIR}/source/algo/*.c")
file(GLOB TASKS_SOURCES "${CMAKE_SOURCE_DIR}/source/tasks/*.c")
list(APPEND SOURCES ${DRIVER_SOURCES} ${MAIN_SOURCES} ${ALGO_SOURCES} ${TASK_SOURCES})


add_executable(HF2_Debug ${SOURCES})

target_link_libraries(HF2_Debug PRIVATE freertos FatFs tjpgdec)

# --- Linker Options (armlink) ---
target_link_options(HF2_Debug PRIVATE
        "LINKER:--cpu=Cortex-M4.fp.sp"
        "LINKER:--map"
        "LINKER:--load_addr_map_info"
        "LINKER:--xref"
        "LINKER:--callgraph"
        "LINKER:--symbols"
        "LINKER:--info=sizes"
        "LINKER:--info=totals"
        "LINKER:--info=unused"
        "LINKER:--info=veneers"
        "LINKER:--list=HF2_Debug.map"
        "LINKER:--scatter=${CMAKE_SOURCE_DIR}/HF2.sct"
)

# --- Post Build ---
set(CMAKE_EXECUTABLE_SUFFIX .elf)
find_program(ARM_FROMELF fromelf)
add_custom_command(TARGET HF2_Debug POST_BUILD
        COMMAND ${ARM_FROMELF} --text -c "$<TARGET_FILE:HF2_Debug>" > "${CMAKE_BINARY_DIR}/HF2_Debug.asm"
)
add_custom_command(TARGET HF2_Debug POST_BUILD
        COMMAND ${ARM_FROMELF} --i32combined --output "${CMAKE_BINARY_DIR}/HF2_Debug.hex" "$<TARGET_FILE:HF2_Debug>"
)
add_custom_command(TARGET HF2_Debug POST_BUILD
        COMMAND ${ARM_FROMELF} --bin --output "${CMAKE_BINARY_DIR}/HF2_Debug.bin" "$<TARGET_FILE:HF2_Debug>"
)
