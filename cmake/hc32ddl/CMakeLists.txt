cmake_minimum_required(VERSION 3.22)

# 启用 C 和汇编语言；本子工程会把启动文件加入主目标
enable_language(C ASM)

# HC32 DDL/CMSIS 需要的全局宏定义
set(HC32_Defines_Syms
        HC32F460
        HC32F460KEUA
        USE_DDL_DRIVER
        $<$<CONFIG:Debug>:__DEBUG>
)

# HC32 应用、DDL、设备头文件和 CMSIS Core 头文件路径
set(HC32_Include_Dirs
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/HC32F460_DDL_Driver/Inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/HDSC/HC32F460KEUA/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include
)

# 应用入口源码和启动文件
set(HC32_Application_Src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/main.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/syscalls.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Platform/startup_hc32f460keua.S
)

# 收集 DDL 驱动源码，新增 DDL .c 文件时会自动参与构建
file(GLOB HC32_DDL_DRIVER_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/HC32F460_DDL_Driver/Src/*.c
)
# USB 驱动依赖 USB 示例配置和 middleware，默认模板不直接编译
list(FILTER HC32_DDL_DRIVER_SOURCES EXCLUDE REGEX ".*/hc32_ll_usb\\.c$")

# 系统初始化文件和 DDL 驱动源码
set(HC32_Drivers_Src
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/system_hc32f460keua.c
        ${HC32_DDL_DRIVER_SOURCES}
)

# 中间件源码可在这里扩展


# DDL 或中间件需要额外库搜索路径时，在这里添加
set(HC32_LINK_DIRS

)

# 主目标需要链接的驱动对象库和工具链默认库
set(HC32_LINK_LIBS
        HC32_Drivers
        ${TOOLCHAIN_LINK_LIBRARIES}
)

# 接口库只传播 include 路径和宏定义，不产生目标文件
add_library(hc32ddl INTERFACE)
target_include_directories(hc32ddl INTERFACE ${HC32_Include_Dirs})
target_compile_definitions(hc32ddl INTERFACE ${HC32_Defines_Syms})

# DDL 驱动对象库，最终由主 ELF 目标链接
add_library(HC32_Drivers OBJECT)
target_sources(HC32_Drivers PRIVATE ${HC32_Drivers_Src})
target_link_libraries(HC32_Drivers PUBLIC
        hc32ddl
        hc32_build_options
)

# 将应用源码和启动文件加入主目标
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${HC32_Application_Src})

# 将 DDL/中间件库搜索路径加入主目标
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${HC32_LINK_DIRS})

# 将驱动对象库和工具链默认库链接到主目标
target_link_libraries(${CMAKE_PROJECT_NAME} ${HC32_LINK_LIBS})

# 检查 C 标准版本，DDL 模板要求 C11 或更高
if ((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif ()
