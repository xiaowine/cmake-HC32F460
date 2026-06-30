# HC32F460 CMake Template

面向 HC32F460 系列 Cortex-M4 MCU 的 CMake/Ninja 工程模板，集成 CMSIS、HC32F460 DDL 驱动、启动文件、链接脚本和常用构建产物生成流程。

模板默认使用 Arm GNU Toolchain 构建，适合在 CLion、VS Code、命令行或 CI 环境中使用。

## 特性

- 支持 HC32F460 多个具体封装/容量型号
- 使用 CMake Presets 管理型号和编译类型
- 使用 Ninja 作为默认生成器
- 自动选择 256KB/512KB Flash 链接脚本
- 统一输出 ELF、HEX、BIN、MAP 文件
- 支持 `Debug`、`Release`、`RelWithDebInfo`、`MinSizeRel`
- 支持 `hard`、`softfp`、`soft` 浮点 ABI
- 用户源码和 DDL 源码支持构建时自动重新扫描

## 支持型号

当前模板支持以下 `HC32_PART`：

| 型号 | 默认链接脚本 |
| --- | --- |
| `HC32F460JCTA` | `Platform/HC32F460xC.ld` |
| `HC32F460JETA` | `Platform/HC32F460xE.ld` |
| `HC32F460JEUA` | `Platform/HC32F460xE.ld` |
| `HC32F460KCTA` | `Platform/HC32F460xC.ld` |
| `HC32F460KETA` | `Platform/HC32F460xE.ld` |
| `HC32F460KEUA` | `Platform/HC32F460xE.ld` |
| `HC32F460PCTB` | `Platform/HC32F460xC.ld` |
| `HC32F460PEHB` | `Platform/HC32F460xE.ld` |
| `HC32F460PETB` | `Platform/HC32F460xE.ld` |

默认型号为 `HC32F460KEUA`。

## 环境要求

- CMake 3.22 或更高版本
- Ninja
- Arm GNU Toolchain `arm-none-eabi`

工具链路径在 [cmake/gcc-arm-none-eabi.cmake](cmake/gcc-arm-none-eabi.cmake) 中配置。若工具链不在系统 `PATH` 中，请根据本机安装路径调整该文件。

## 快速开始

查看可用 preset：

```powershell
cmake --list-presets
```

构建默认型号 Debug 版本：

```powershell
cmake --preset HC32F460KEUA-Debug
cmake --build --preset HC32F460KEUA-Debug
```

构建其它型号：

```powershell
cmake --preset HC32F460KCTA-Debug
cmake --build --preset HC32F460KCTA-Debug
```

构建 Release 版本：

```powershell
cmake --preset HC32F460KCTA-Release
cmake --build --preset HC32F460KCTA-Release
```

构建产物位于对应构建目录的 `firmware` 文件夹，例如：

```text
build/HC32F460KCTA-Debug/firmware/HC32F460KCTA.elf
build/HC32F460KCTA-Debug/firmware/HC32F460KCTA.hex
build/HC32F460KCTA-Debug/firmware/HC32F460KCTA.bin
build/HC32F460KCTA-Debug/firmware/HC32F460KCTA.map
```

## CLion 使用

使用 CLion 打开包含 `CMakeLists.txt` 和 `CMakePresets.json` 的项目根目录。

CLion 会识别 CMake Presets。选择需要的 CMake Profile，例如：

```text
HC32F460KEUA-Debug
HC32F460KEUA-Release
HC32F460KCTA-Debug
HC32F460KCTA-Release
```

然后执行 `Reload CMake Project` 并正常构建。

启用 CMake Presets 后，CLion 的 `Build type` 可能显示为 `Default` 且不可切换。此时以 CMake Profile 为准，直接选择 `型号-编译类型` 对应的 preset。

如果 CLion 仍显示旧 preset 或旧型号，可执行 `Reset Cache and Reload Project`。

## Preset 命名规则

常规 preset 名称格式：

```text
<HC32_PART>-<BUILD_TYPE>
```

示例：

```text
HC32F460KEUA-Debug
HC32F460KEUA-Release
HC32F460KCTA-RelWithDebInfo
HC32F460PCTB-MinSizeRel
```

每个 preset 使用独立构建目录：

```text
build/<presetName>
```

这样可以避免不同型号或不同编译类型共用同一个 `CMakeCache.txt`。

## 手动配置

不使用 preset 时，也可以手动指定构建参数：

```powershell
cmake -S . -B build/HC32F460KCTA-Debug -G Ninja --toolchain cmake/gcc-arm-none-eabi.cmake -DCMAKE_BUILD_TYPE=Debug -DHC32_PART=HC32F460KCTA
cmake --build build/HC32F460KCTA-Debug
```

建议不同型号使用不同构建目录，避免缓存中的 `HC32_PART` 与当前目标型号不一致。

## 浮点 ABI

HC32F460 为 Cortex-M4F，默认使用硬浮点：

```text
HC32_FLOAT_ABI=hard
```

可选值：

- `hard`
- `softfp`
- `soft`

命令行示例：

```powershell
cmake --preset HC32F460KEUA-Debug -DHC32_FLOAT_ABI=softfp
cmake --build --preset HC32F460KEUA-Debug
```

模板也提供了一个软浮点示例 preset：

```powershell
cmake --preset HC32F460KEUA-SoftFloat-Debug
cmake --build --preset HC32F460KEUA-SoftFloat-Debug
```

## 链接脚本

CMake 会根据 `HC32_PART` 自动选择默认链接脚本：

- `HC32F460JCTA`、`HC32F460KCTA`、`HC32F460PCTB` 使用 `Platform/HC32F460xC.ld`
- 其它当前支持型号使用 `Platform/HC32F460xE.ld`

如需手动覆盖链接脚本：

```powershell
cmake --preset HC32F460KEUA-Debug -DHC32_LINKER_SCRIPT=Platform/HC32F460_RAM.ld
cmake --build --preset HC32F460KEUA-Debug
```

## 工程结构

```text
.
├── CMakeLists.txt
├── CMakePresets.json
├── Core
│   ├── Inc
│   └── Src
├── Drivers
│   ├── CMSIS
│   └── HC32F460_DDL_Driver
├── Platform
├── cmake
└── docs
```

目录说明：

- `Core/Inc`：用户应用头文件和 DDL 配置文件
- `Core/Src`：用户应用源码、系统初始化文件和系统调用桩函数
- `Drivers/CMSIS`：CMSIS Core、HDSC 设备头文件和 SVD 文件
- `Drivers/HC32F460_DDL_Driver`：HC32F460 DDL 驱动源码
- `Platform`：启动文件和链接脚本
- `cmake`：工具链文件和驱动集成 CMake 文件
- `docs`：数据手册和参考手册

## 添加源码

用户应用源码放入 `Core/Src`，头文件放入 `Core/Inc`。

`Core/Src/*.c` 和 DDL 驱动源码使用 `CONFIGURE_DEPENDS` 自动扫描。使用 Ninja 构建时，如果新增或删除源码文件，CMake 会在构建前重新检查文件列表。

如需添加额外源码、头文件路径、宏定义或库，可在根目录 [CMakeLists.txt](CMakeLists.txt) 中对应位置扩展：

- `target_sources`
- `target_include_directories`
- `target_compile_definitions`
- `target_link_libraries`

## 烧录

如果系统安装了 `pyOCD`，CMake 会自动提供 `flash-pyocd` 目标：

```powershell
cmake --build --preset HC32F460KEUA-Debug --target flash-pyocd
```

默认 pyOCD target 会根据 Flash 容量选择：

- `hc32f460xc`
- `hc32f460xe`

如需覆盖：

```powershell
cmake --preset HC32F460KEUA-Debug -DHC32_PYOCD_TARGET=hc32f460xe
```

## GitHub Actions

模板提供 GitHub Actions 编译工作流：

```text
.github/workflows/build.yml
```

触发条件：

- `push`
- `pull_request`
- `workflow_dispatch`

`push` 和 `pull_request` 默认只构建项目目标型号的 `Debug` 与 `Release` 版本，并上传固件产物。默认目标型号为 `HC32F460KEUA`。

可在 GitHub 仓库变量中覆盖默认 CI 参数：

| 变量 | 默认值 | 说明 |
| --- | --- | --- |
| `HC32_CI_PART` | `HC32F460KEUA` | PR/提交时构建的目标型号 |
| `HC32_CI_FLOAT_ABI` | `hard` | PR/提交时使用的浮点 ABI |

仓库变量路径：

```text
Settings -> Secrets and variables -> Actions -> Variables
```

手动触发时可选择：

- `part`：具体 MCU 型号，或 `all`
- `build_type`：`Debug`、`Release`、`RelWithDebInfo`、`MinSizeRel`
- `float_abi`：`hard`、`softfp`、`soft`
- `upload_artifacts`：是否上传构建产物

当 `part` 选择 `all` 时，工作流会构建全部支持型号，用于检查模板或公共代码的跨型号兼容性。实际项目使用了某个封装特有资源时，建议 PR/提交 CI 只构建项目目标型号。

## 文档

芯片资料位于 `docs` 目录：

- `HC32F460_Datasheet_Rev1.61.pdf`
- `HC32F460_Reference_Manual_Rev1.71.pdf`
