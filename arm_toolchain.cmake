set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m4)

# 1. 设置工具链路径 (根据你的报错日志修改)
set(ARM_TOOLCHAIN_PATH "D:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.2 rel1/bin")

# 2. 查找 GCC 工具 (使用 NO_DEFAULT_PATH 防止找到其他版本的 GCC)
find_program(CMAKE_C_COMPILER NAMES arm-none-eabi-gcc PATHS ${ARM_TOOLCHAIN_PATH} NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES arm-none-eabi-g++ PATHS ${ARM_TOOLCHAIN_PATH} NO_DEFAULT_PATH)
find_program(CMAKE_ASM_COMPILER NAMES arm-none-eabi-gcc PATHS ${ARM_TOOLCHAIN_PATH} NO_DEFAULT_PATH)
find_program(CMAKE_OBJCOPY NAMES arm-none-eabi-objcopy PATHS ${ARM_TOOLCHAIN_PATH} NO_DEFAULT_PATH)
find_program(CMAKE_SIZE NAMES arm-none-eabi-size PATHS ${ARM_TOOLCHAIN_PATH} NO_DEFAULT_PATH)

# 跳过编译器完整性检查 (嵌入式交叉编译通常需要)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 3. 编译选项 (关键修改：移除 --target，改用 GCC 标准参数)
set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")

set(CMAKE_C_FLAGS_INIT "${MCU_FLAGS} -std=gnu11 -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS_INIT "${MCU_FLAGS} -std=c++14 -fdata-sections -ffunction-sections")
set(CMAKE_ASM_FLAGS_INIT "${MCU_FLAGS} -x assembler-with-cpp")

# 4. 链接选项 (使用 nano.specs 减小体积)
set(CMAKE_EXE_LINKER_FLAGS_INIT "--specs=nano.specs -Wl,--gc-sections")