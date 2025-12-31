# 指定 cmake 最低版本需求
cmake_minimum_required(VERSION 3.20)
# 指定项目名称
project(template)

# 指定语言环境
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

ENABLE_LANGUAGE(ASM)

# 指定芯片架构
set(MCU cortex-m4)
set(FPU_FLAGS "-mfpu=fpv4-sp-d16 -mfloat-abi=hard")
add_compile_definitions(STM32F429xx)

# 指定各模块路径

# mcu组
set(MCU_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../mcu)
set(COR_DIR ${MCU_DIR}/core)
set(LIB_DIR ${MCU_DIR}/Libraries)
set(DRI_DIR ${MCU_DIR}/drivers)
set(BOA_DIR ${MCU_DIR}/board)
set(DEV_DIR ${MCU_DIR}/devices)
set(SER_DIR ${MCU_DIR}/services)
set(APP_DIR ${MCU_DIR}/app)

# crm组
set(CRM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../crm)
# freeRTOS模块
set(FREERTOS_DIR ${CRM_DIR}/freeRTOS)
set(FRTOS_INC_DIR ${FREERTOS_DIR}/include)
set(FRTOS_PORT_DIR ${FREERTOS_DIR}/portable)
set(FRTOS_SRC_DIR ${FREERTOS_DIR}/src)
set(MEMMANG_DIR ${FRTOS_PORT_DIR}/MemMang)
set(ARM_CM4F_DIR ${FRTOS_PORT_DIR}/GCC/ARM_CM4F)


# 基本编译选项
set(CMAKE_C_FLAGS "-mcpu=${MCU} -mthumb ${FPU_FLAGS} -O2 -ffunction-sections -fdata-sections -Wall -g")
set(CMAKE_EXE_LINKER_FLAGS "-T${CMAKE_CURRENT_SOURCE_DIR}/STM32F429IGTX_FLASH.ld -Wl,--gc-sections -static")

# 指定启动文件
set(STARTUP_FILE ${LIB_DIR}/CMSIS/startup_stm32f429xx.s)


# 包含目录
include_directories(
    # mcu组
    ${COR_DIR}
    ${LIB_DIR}/CMSIS
    ${LIB_DIR}/HAL_Driver
    ${LIB_DIR}/HAL_Driver/Legacy
    ${DRI_DIR}
    ${BOA_DIR}
    ${DEV_DIR}
    ${SER_DIR}
    ${APP_DIR}

    # crm组
    ${FRTOS_INC_DIR}
    ${ARM_CM4F_DIR}

)


# 源文件
file(GLOB SRC_FILES
    # mcu组
    ${COR_DIR}/*.c
    ${LIB_DIR}/CMSIS/*.c
    ${LIB_DIR}/HAL_Driver/*.c
    ${LIB_DIR}/HAL_Driver/Legacy/*.c
    ${DRI_DIR}/*.c
    ${BOA_DIR}/*.c
    ${DEV_DIR}/*.c
    ${SER_DIR}/*.c
    ${APP_DIR}/*.c

    # crm组
    ${ARM_CM4F_DIR}/*.c
    ${MEMMANG_DIR}/*.c
    ${FRTOS_SRC_DIR}/*.c
)

# 移除其他时钟基准文件
list(REMOVE_ITEM SRC_FILES
    ${LIB_DIR}/HAL_Driver/stm32f4xx_hal_timebase_rtc_alarm_template.c
    ${LIB_DIR}/HAL_Driver/stm32f4xx_hal_timebase_rtc_wakeup_template.c
)


# 生成可执行文件（ELF）
add_executable(${PROJECT_NAME}.elf ${SRC_FILES} ${STARTUP_FILE})

# 生成 HEX 与 BIN
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
)
