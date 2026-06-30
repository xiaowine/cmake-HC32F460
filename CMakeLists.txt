cmake_minimum_required(VERSION 3.22)

# C 语言标准设置：HC32 DDL 源码按 C11/gnu11 构建
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# 默认使用本工程内的 Arm GNU Toolchain 工具链文件
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE
        "${CMAKE_CURRENT_LIST_DIR}/cmake/gcc-arm-none-eabi.cmake"
        CACHE FILEPATH "ARM GCC toolchain file")
endif()

# Ninja 是单配置生成器，编译类型由 CMAKE_BUILD_TYPE 决定。
# 未显式指定时默认使用 Debug；CLion 中推荐通过 CMake preset 切换编译类型。
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)

# HC32F460 系列支持的具体型号。头文件会按型号切换，避免封装/GPIO 差异被误用。
set(HC32_SUPPORTED_PARTS
    HC32F460JCTA
    HC32F460JETA
    HC32F460JEUA
    HC32F460KCTA
    HC32F460KETA
    HC32F460KEUA
    HC32F460PCTB
    HC32F460PEHB
    HC32F460PETB
)
set(HC32_PART "HC32F460KEUA" CACHE STRING "Concrete HC32F460 part number")
set_property(CACHE HC32_PART PROPERTY STRINGS ${HC32_SUPPORTED_PARTS})
string(TOUPPER "${HC32_PART}" HC32_PART)
set(HC32_PART "${HC32_PART}" CACHE STRING "Concrete HC32F460 part number" FORCE)

if(NOT HC32_PART IN_LIST HC32_SUPPORTED_PARTS)
    message(FATAL_ERROR "Unsupported HC32_PART=${HC32_PART}. Supported parts: ${HC32_SUPPORTED_PARTS}")
endif()

# xC 型号为 256KB Flash，其余当前支持型号为 512KB Flash。
set(HC32_256K_FLASH_PARTS
    HC32F460JCTA
    HC32F460KCTA
    HC32F460PCTB
)
if(HC32_PART IN_LIST HC32_256K_FLASH_PARTS)
    set(HC32_FLASH_SUFFIX xC)
else()
    set(HC32_FLASH_SUFFIX xE)
endif()

# 工程名使用系列名；固件输出名使用具体型号，方便多型号构建时区分产物。
set(CMAKE_PROJECT_NAME HC32F460)
set(HC32_FIRMWARE_NAME "${HC32_PART}")

# 导出 compile_commands.json，方便 clangd/IDE 做代码索引
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# 初始化 CMake 工程；模板只启用 C 和汇编，避免 CI/IDE 额外探测 C++ 工具链。
project(${CMAKE_PROJECT_NAME} C ASM)
message("-- Build type: " ${CMAKE_BUILD_TYPE})

# 默认按 HC32_PART 选择链接脚本；HC32_LINKER_SCRIPT 非空时作为手动覆盖。
set(HC32_DEFAULT_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/Platform/HC32F460${HC32_FLASH_SUFFIX}.ld")
set(HC32_LINKER_SCRIPT "" CACHE FILEPATH "Optional linker script override; empty selects by HC32_PART")
if(HC32_LINKER_SCRIPT STREQUAL "")
    set(HC32_LINKER_SCRIPT_SELECTED "${HC32_DEFAULT_LINKER_SCRIPT}")
else()
    set(HC32_LINKER_SCRIPT_SELECTED "${HC32_LINKER_SCRIPT}")
endif()

# 将链接脚本转换成绝对路径，避免 Ninja 从构建目录执行时找不到文件
if(IS_ABSOLUTE "${HC32_LINKER_SCRIPT_SELECTED}")
    set(HC32_LINKER_SCRIPT_ABS "${HC32_LINKER_SCRIPT_SELECTED}")
else()
    cmake_path(ABSOLUTE_PATH HC32_LINKER_SCRIPT_SELECTED
        BASE_DIRECTORY "${CMAKE_SOURCE_DIR}"
        NORMALIZE
        OUTPUT_VARIABLE HC32_LINKER_SCRIPT_ABS)
endif()

set(HC32_SVD_FILE "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/HDSC/HC32F460/SVD/${HC32_PART}.svd")
message(STATUS "HC32 part: ${HC32_PART}")
message(STATUS "HC32 linker script: ${HC32_LINKER_SCRIPT_ABS}")

# 浮点 ABI 选择：HC32F460 为 Cortex-M4F，默认使用硬浮点
set(HC32_FLOAT_ABI "hard" CACHE STRING "Floating-point ABI: soft, softfp, or hard")
set_property(CACHE HC32_FLOAT_ABI PROPERTY STRINGS soft softfp hard)

# MCU 通用编译/链接参数
set(HC32_MCU_FLAGS -mcpu=cortex-m4 -mthumb -mthumb-interwork)
if(HC32_FLOAT_ABI STREQUAL "soft")
    list(APPEND HC32_MCU_FLAGS -mfloat-abi=soft)
elseif(HC32_FLOAT_ABI STREQUAL "softfp" OR HC32_FLOAT_ABI STREQUAL "hard")
    list(APPEND HC32_MCU_FLAGS -mfpu=fpv4-sp-d16 -mfloat-abi=${HC32_FLOAT_ABI})
else()
    message(FATAL_ERROR "HC32_FLOAT_ABI must be one of: soft, softfp, hard")
endif()

# 工程级编译/链接选项，供主程序和驱动目标复用
add_library(hc32_build_options INTERFACE)
target_compile_options(hc32_build_options INTERFACE
    ${HC32_MCU_FLAGS}
    -ffunction-sections
    -fdata-sections
    -fmessage-length=0
    -fsigned-char
    -fno-common
    $<$<CONFIG:Debug>:-Og;-g3>
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:MinSizeRel>:-Os>
    $<$<CONFIG:RelWithDebInfo>:-O2;-g>
)
target_link_options(hc32_build_options INTERFACE
    ${HC32_MCU_FLAGS}
    -T${HC32_LINKER_SCRIPT_ABS}
    # 使用 newlib-nano，并由 libnosys 提供最小系统调用桩函数
    --specs=nano.specs
    --specs=nosys.specs
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -Wl,--no-warn-rwx-segments
    -Wl,-Map=${CMAKE_BINARY_DIR}/firmware/${HC32_FIRMWARE_NAME}.map
)

# 创建最终 ELF 目标
add_executable(${CMAKE_PROJECT_NAME})
target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE c_std_11)

# 添加 HC32 DDL/CMSIS 相关源码和 include
add_subdirectory(cmake/hc32ddl)

# 用户自定义库搜索路径可加在这里
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # 在这里添加用户库搜索路径
)

# 用户自定义源码可加在这里
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # 在这里添加用户源码
)

# 用户自定义头文件路径可加在这里
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # 在这里添加用户头文件路径
)

# 用户自定义宏定义可加在这里
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # 在这里添加用户宏定义
)

# 使用 C++ 文件时，移除部分工具链可能误带入的 libob.a 依赖
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# 链接 HC32 DDL 接口库、通用构建选项，以及用户自定义库
target_link_libraries(${CMAKE_PROJECT_NAME}
    hc32ddl
    hc32_build_options

    # 在这里添加用户库
)

# 统一固件输出目录，并声明链接脚本变化会触发重新链接
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    SUFFIX ".elf"
    OUTPUT_NAME "${HC32_FIRMWARE_NAME}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/firmware"
    LINK_DEPENDS "${HC32_LINKER_SCRIPT_ABS}"
    ADDITIONAL_CLEAN_FILES "${CMAKE_BINARY_DIR}/firmware/${HC32_FIRMWARE_NAME}.map"
)

set(HC32_HEX_FILE "${CMAKE_BINARY_DIR}/firmware/${HC32_FIRMWARE_NAME}.hex")
set(HC32_BIN_FILE "${CMAKE_BINARY_DIR}/firmware/${HC32_FIRMWARE_NAME}.bin")

# ELF 链接完成后自动生成 HEX/BIN，并打印 size 信息
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> "${HC32_HEX_FILE}"
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> "${HC32_BIN_FILE}"
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    BYPRODUCTS "${HC32_HEX_FILE}" "${HC32_BIN_FILE}"
    COMMENT "Generating HEX/BIN images"
    VERBATIM
)

# 如果系统安装了 pyOCD，则自动提供 flash-pyocd 烧录目标
find_program(PYOCD_EXECUTABLE NAMES pyocd pyocd.exe)
if(PYOCD_EXECUTABLE)
    string(TOLOWER "hc32f460${HC32_FLASH_SUFFIX}" HC32_DEFAULT_PYOCD_TARGET)
    set(HC32_PYOCD_TARGET "${HC32_DEFAULT_PYOCD_TARGET}" CACHE STRING "pyOCD target name")
    add_custom_target(flash-pyocd
        COMMAND ${PYOCD_EXECUTABLE} flash -t ${HC32_PYOCD_TARGET} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
        DEPENDS ${CMAKE_PROJECT_NAME}
        USES_TERMINAL
        COMMENT "Flashing ${HC32_FIRMWARE_NAME} with pyOCD"
        VERBATIM
    )
endif()
