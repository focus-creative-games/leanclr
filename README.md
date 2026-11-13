# LeanCLR

语言: [中文](./README.md) | [English](./README_EN.md)

[![GitHub](https://img.shields.io/badge/GitHub-Repository-181717?logo=github)](https://github.com/focus-creative-games/leanclr) [![Gitee](https://img.shields.io/badge/Gitee-Repository-C71D23?logo=gitee)](https://gitee.com/focus-creative-games/leanclr)

[![license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/focus-creative-games/leanclr/blob/mono-minimal/LICENSE) [![DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/focus-creative-games/leanclr) [![Discord](https://img.shields.io/badge/Discord-Join-7289DA?logo=discord&logoColor=white)](https://discord.gg/esAYcM6RDQ)

LeanCLR 是一个面向生产发布的 CLR实现，核心目标是提供高 ECMA-335 兼容度、低集成复杂度和强跨平台能力，真正实现 “Write C#, Run Anywhere”。

## 为什么需要 LeanCLR

在“将 C# 逻辑嵌入宿主并发布到多平台”这个目标下，CoreCLR、Mono、IL2CPP 通常存在以下限制：

- **CoreCLR**和**Mono**：运行时能力完整，但体积、依赖和宿主集成复杂度较高，裁剪与移植成本不适合轻量嵌入式发布场景。移植到新平台的难度高，工程复杂。
- **IL2CPP**：非开源，与 Unity 生态和工具链强绑定，且仅支持AOT发布，对 ECMA-335标准支持度不高

LeanCLR 的定位是补足上述空白：在保持 ECMA-335 高兼容的前提下，提供易集成、精简高效的跨平台CLR实现。

## 核心特点

- **极佳跨平台能力** — 使用AOT + Interpreter 混合执行架构，不支持JIT，使用标准 C++17 实现，无任何平台相关依赖。具有极佳的跨平台移植性，运行时要求小端字节序，具体目标平台仍需完成编译和测试验证。
- **易于集成** — 集成极其简单，复杂度与lua相近，轻松嵌入到任何环境，比如app ，游戏，嵌入式，车机平台，等等
- **高度兼容 ECMA-335**：几乎完整支持 ECMA-335 与 CoreCLR 主要扩展，覆盖泛型、异常、反射、委托等核心能力。
- **精简高效** — 非常小巧，内存占用小，运行高效，尤其适合资源有限的硬件环境。Mono minimal 单线程版本在 X64/WebAssembly 平台不到 **600 KB**，裁剪后可低至 **300 KB**

## 文档

完整文档站点：**https://doc.leanclr.com**

- [快速开始](https://doc.leanclr.com/docs/getting-started/overview)
- [构建与集成](https://doc.leanclr.com/docs/integration/overview)
- [AOT](https://doc.leanclr.com/docs/aot/overview)
- [互操作](https://doc.leanclr.com/docs/interop/overview)
- [测试指南](https://doc.leanclr.com/docs/development/testing)
- [贡献指南](https://doc.leanclr.com/docs/development/contributing)

## 原生宿主的平台目标

原生宿主面向以下平台；不同平台的具体 API 与验证范围见构建和测试文档：

| 平台 | 说明 |
|------|------|
| **Windows** | Win64 等桌面目标 |
| **Linux** | 桌面与嵌入式 Linux |
| **macOS** | 桌面目标 |
| **Android** | 移动端 |
| **iOS** | 移动端 |
| **HarmonyOS（鸿蒙）** | 鸿蒙生态 |
| **WebAssembly** | Web 浏览器及小游戏平台 |

## 引擎宿主仓库

Unity、Godot、Unreal Engine 的宿主适配在独立仓库维护，通过 LeanCLR 公共接口接入。三个仓库均已建立，仓库内容后续整理；本轮不据此宣称完成新的引擎 SDK 集成。

| 引擎 | 仓库 | 集成目标 |
|------|------|----------|
| Unity / 团结引擎 | [leanclr-unity](https://github.com/focus-creative-games/leanclr-unity) | Unity 宿主适配及构建发布集成 |
| Godot | [leanclr-godot](https://github.com/focus-creative-games/leanclr-godot) | Godot 宿主、对象与资源适配 |
| Unreal Engine | [leanclr-ue](https://github.com/focus-creative-games/leanclr-ue) | UE 插件、模块及构建发布集成 |

## 项目状态

### 当前进度

| 模块 | 状态 | 说明 |
|------|------|------|
| **元数据解析** | ✅ 完成 | 完整支持 PE/COFF 格式和 CLI 元数据表 |
| **类型系统** | ✅ 完成 | 类、接口、泛型、数组、值类型等 |
| **IR 解释器** | ✅ 完成 | 热点函数优化执行 |
| **异常处理** | ✅ 完成 | try/catch/finally、嵌套异常等 |
| **反射** | ✅ 完成 | Type、MethodInfo、FieldInfo 等核心 API |
| **委托** | ✅ 完成 | 单播/多播委托、泛型委托 |
| **内部调用** | ✅ 完成 | 由所选 BCL 适配器注册，支持范围以清单为准 |
| **P/Invoke** | ✅ 完成 | 支持手动注册及使用LeanAOT自动生成P/Invoke包装函数 |
| **垃圾回收** | ✅ 完成 | 准确式 Mark-Sweep 全量 GC |
| **AOT 编译器** | ✅ 完成 | 已支持 IL → C++ 转译 |
| **多线程** | 📋 规划中 | 线程、同步原语及 GC 协作，当前两种宿主配置均为单线程 |

### 稳定性

以下为各分支整合前的历史兼容情况，不代表统一后所有构建配置的验证结果：

- **历史 unity 分支（已退出运行时分支结构）**：与 Unity 2019.4.x – 6000.3.x LTS IL2CPP 的 BCL **完全兼容**，通过全部（数千个）测试用例
- **原 mono 分支（现 mono-full）**：历史测试与 Mono 4.8 的 BCL **99.95% 兼容**，仅一个测试用例失败；当前已对齐重构后的 core，需按配置重新验证
- **coreclr-minimal 分支**：正在支持 .NET 10 BCL，持续完善中

## 版本与分支

LeanCLR 的运行时分支按 **BCL × 能力级别** 命名，宿主和引擎不构成独立运行时分支：

| 分支 | 定位 | 当前状态 |
|------|------|----------|
| `mono-minimal` | Mono BCL，精简、可移植的能力范围 | 由原 `core` 改名；主 clone 的默认 checkout |
| `mono-full` | Mono BCL，扩展平台 API 与线程能力的开发目标 | 由原 `mono` 改名；多线程仍待实现 |
| `coreclr-minimal` | .NET 10 CoreCLR BCL，精简支持范围 | 由 `coreclr-net10` 改名；按支持清单验收 |
| `coreclr-full` | CoreCLR BCL，扩展能力目标 | 尚未创建，不用空分支占位 |

`full` 表示开发定位，不表示已经完整兼容全部 BCL 或实现多线程。两个 Mono 分支当前共享同一运行时基线。语言、GC、泛型、解释器和 AOT 等公共修复应在各相关运行时分支同步，避免为宿主复制核心。

## 架构与构建配置

本仓库保留公共宿主 API、SDK 无关的生命周期/错误契约、`src/tools/leanrun`、唯一的 standalone 示例 [`src/samples/startup`](src/samples/startup/README.md) 及测试，**不创建 standalone 分支**。Unity、Godot、UE 的 SDK 插件、对象绑定、资源和打包集成由独立宿主仓库维护，通过公共 API 接入，不能依赖运行时私有 BCL 布局。

分支定位与底层 host provider 是不同维度。`PORTABLE`、`NATIVE`、`CUSTOM` 保留为构建选项，不创建相应分支：

| CMake 预设 | BCL | 宿主能力 | 线程模型 |
|------|-----|----------|----------|
| `mono-minimal` | Mono | 标准 C/C++ 库宿主，无 Windows/POSIX SDK 依赖 | 单线程 |
| `mono-full` / `mono-native` | Mono | 当前原生平台服务；Full 为开发目标，API 以已实现范围为准 | 单线程 |
| `mono-custom` | Mono | 应用注入内存、时钟、I/O 等服务 | 单线程 |

`mono-native` 是兼容的构建预设，不是保留分支。CoreCLR BCL 的配置位于 `coreclr-minimal` 分支；`CORECLR_NET10` 和工具 profile `coreclr-net10` 是 BCL 版本标识，不随分支改名。

```powershell
scripts\runtime\build.bat Release x64 minimal
scripts\runtime\build.bat Release x64 native
```

Linux/macOS 对应 `./scripts/runtime/build.sh Release minimal` 或 `native`。现有脚本的 Native 默认行为、CMake 目标 `leanclr` 和公共 API 保持兼容；需要精简宿主时显式选择 minimal/PORTABLE。Minimal 仍可使用已有标准库 I/O；完全接管服务时选择 CUSTOM。运行时要求小端平台和 C++17，新目标必须单独验证。

详见[架构与分支维护](docs/architecture.md)、[运行时分层](src/runtime/bcl/README.md)及[内存所有权与失败处理](src/runtime/alloc/README.md)。

## 联系方式

- 邮箱：leanclr#code-philosophy.com
- Discord 频道：<https://discord.gg/esAYcM6RDQ>
- QQ 群：1047250380
