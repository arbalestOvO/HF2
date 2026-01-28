set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m4)

# 1. 路径处理
if(NOT DEFINED ENV{ARM_TOOLCHAIN_PATH})
    message(STATUS "ARM_TOOLCHAIN_PATH env not set, searching in PATH")
else()
    file(TO_CMAKE_PATH "$ENV{ARM_TOOLCHAIN_PATH}" TOOLCHAIN_PATH)
    list(APPEND CMAKE_PROGRAM_PATH "${TOOLCHAIN_PATH}")
endif()

# 2. 强制指定各阶段工具
find_program(CMAKE_C_COMPILER NAMES armclang)
find_program(CMAKE_CXX_COMPILER NAMES armclang)
find_program(CMAKE_ASM_COMPILER NAMES armclang)
find_program(CMAKE_LINKER NAMES armlink)
find_program(CMAKE_OBJCOPY NAMES fromelf)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 3. 编译选项 (基础架构选项)
set(CMAKE_C_FLAGS_INIT "--target=arm-arm-none-eabi -xc -std=c99")
set(CMAKE_CXX_FLAGS_INIT "--target=arm-arm-none-eabi -xc++ -std=c++11")
set(CMAKE_ASM_FLAGS_INIT "--target=arm-arm-none-eabi -x assembler-with-cpp -masm=armasm")

# 4. 链接选项 (armlink)
set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "linker flags" FORCE)

# 5. 屏蔽默认标志
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")