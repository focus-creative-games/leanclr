# LeanCLR

语言: [中文](./README.md) | [English](./README_EN.md)

[![GitHub](https://img.shields.io/badge/GitHub-Repository-181717?logo=github)](https://github.com/focus-creative-games/leanclr) [![Gitee](https://img.shields.io/badge/Gitee-Repository-C71D23?logo=gitee)](https://gitee.com/focus-creative-games/leanclr)

[![license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/focus-creative-games/leanclr/blob/main/LICENSE) [![DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/focus-creative-games/leanclr) [![Discord](https://img.shields.io/badge/Discord-Join-7289DA?logo=discord&logoColor=white)](https://discord.gg/esAYcM6RDQ)

LeanCLR 是一个面向生产发布的 CLR实现，核心目标是提供高 ECMA-335 兼容度、低集成复杂度和强跨平台能力，真正实现 “Write C#, Run Anywhere”。

## 为什么需要 LeanCLR

在“将 C# 逻辑嵌入宿主并发布到多平台”这个目标下，CoreCLR、Mono、IL2CPP 通常存在以下限制：

- **CoreCLR**和**Mono**：运行时能力完整，但体积、依赖和宿主集成复杂度较高，裁剪与移植成本不适合轻量嵌入式发布场景。移植到新平台的难度高，工程复杂。
- **IL2CPP**：非开源，与 Unity 生态和工具链强绑定，且仅支持AOT发布，对 ECMA-335标准支持度不高

LeanCLR 的定位是补足上述空白：在保持 ECMA-335 高兼容的前提下，提供易集成、精简高效的跨平台CLR实现。

## 核心特点

- **极佳跨平台能力** — 使用AOT + Interpreter 混合执行架构，不支持JIT，使用标准 C++11 实现，无任何平台相关依赖。具有极佳的跨平台移植性，不需要任何修改就可以在所有支持标准C++ 11编译器的平台运行。
- **易于集成** — 集成极其简单，复杂度与lua相近，轻松嵌入到任何环境，比如app ，游戏，嵌入式，车机平台，等等
- **高度兼容 ECMA-335**：几乎完整支持 ECMA-335 与 CoreCLR 主要扩展，覆盖泛型、异常、反射、委托等核心能力。
- **精简高效** — 非常小巧，内存占用小，运行高效，尤其适合资源有限的硬件环境。Core 单线程版本在 X64/WebAssembly 平台不到 **600 KB**，裁剪后可低至 **300 KB**

## 文档

完整文档站点：**https://doc.leanclr.com**

- [快速开始](https://doc.leanclr.com/docs/getting-started/overview)
- [构建与集成](https://doc.leanclr.com/docs/integration/overview)
- [AOT](https://doc.leanclr.com/docs/aot/overview)
- [互操作](https://doc.leanclr.com/docs/interop/overview)
- [测试指南](https://doc.leanclr.com/docs/development/testing)
- [贡献指南](https://doc.leanclr.com/docs/development/contributing)

## 支持平台（Standard）

Standard 版本目前已支持以下平台：

| 平台 | 说明 |
|------|------|
| **Windows** | Win64 等桌面目标 |
| **Linux** | 桌面与嵌入式 Linux |
| **macOS** | 桌面目标 |
| **Android** | 移动端 |
| **iOS** | 移动端 |
| **HarmonyOS（鸿蒙）** | 鸿蒙生态 |
| **WebAssembly** | Web 浏览器及小游戏平台 |

## 生态与集成

LeanCLR 已支持 Unity 引擎，正在接入 Godot 与 CoreCLR BCL，并将持续扩展更多引擎和平台。

| 平台 / 集成 | 状态 | 说明 |
|------|------|------|
| **Unity 及团结引擎 WebGL 和小游戏平台** | ✅ 完成 | [leanclr-unity](https://github.com/focus-creative-games/leanclr-unity) 是 LeanCLR 的 Unity 插件，发布游戏（不限于 WebGL/小游戏平台）时替换 IL2CPP 为 LeanCLR |
| **Godot 全平台** | 🚧 开发中 | [leanclr-godot](https://github.com/focus-creative-games/leanclr-godot) 正在接入 Godot 引擎 |
| **CoreCLR .NET 10 BCL** | 🚧 开发中 | Standard 的 coreclr 分支正在支持 .NET 10 BCL |
| **Unreal Engine 全平台** | 📋 规划中 | 发布时间待定 |

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
| **内部调用** | ✅ 完成 | 当前以 Core 版本 icall 为主 |
| **P/Invoke** | ✅ 完成 | 支持手动注册及使用LeanAOT自动生成P/Invoke包装函数 |
| **垃圾回收** | ✅ 完成 | 准确式 Mark-Sweep 全量 GC |
| **AOT 编译器** | ✅ 完成 | 已支持 IL → C++ 转译 |
| **多线程** | 📋 规划中 | 线程、同步原语等（Standard 版本） |

### 稳定性

当前 Standard 版本稳定性较高：

- **unity 分支**：与 Unity 2019.4.x – 6000.3.x LTS IL2CPP 的 BCL **完全兼容**，通过全部（数千个）测试用例
- **mono 分支**：与 Mono 4.8 的 BCL **99.95% 兼容**，仅一个测试用例失败
- **coreclr 分支**：正在支持 .NET 10 BCL，持续完善中

## 版本说明

LeanCLR 提供 **Standard** 与 **Core** 两个版本：**Core 自 Standard 裁剪而来**。两者均已实现（单线程）；Standard 的多线程能力仍在规划中。详见 [Core 与 Standard](https://doc.leanclr.com/docs/intro/editions)。

### Core 版本

Core 版本已经实现，可运行于所有支持 C++11 的平台。Core 适合作为嵌入式的纯脚本引擎：既可以直接纯解释执行，也可以配合 [LeanAOT](https://doc.leanclr.com/docs/aot/overview) 将热点 IL 转译为 C++，获得良好的运行性能。

### Standard 分支

Standard 版本按 BCL 来源分为三个分支：

| 分支 | BCL 来源 | 说明 |
|------|----------|------|
| **mono** | Mono BCL | 通用跨平台集成，兼容 Mono 4.8 BCL |
| **unity** | Unity IL2CPP BCL | 面向 Unity / 团结引擎集成 |
| **coreclr** | CoreCLR BCL | 正在支持 .NET 10 BCL |

### Standard 与 Core 对比

| 特性 | Standard | Core |
|------|----------|------|
| ECMA-335 | 标准实现，高兼容 | 标准 ECMA-335 规范实现 |
| 线程模型 | 单线程（多线程规划中） | 单线程 |
| 跨平台 | Win、Linux、macOS、Android、iOS、HarmonyOS、WebAssembly 等 | 全平台，纯 C++11，无平台相关依赖 |
| BCL | mono / unity / coreclr 分支，平台相关 icall 部分实现 | mono-4.5 BCL，仅实现其中部分平台相关调用 |
| GC | 准确式 Mark-Sweep 全量 GC | 准确式 Mark-Sweep 全量 GC |

## Demo

### leanclr-demo

[leanclr-demo](https://github.com/focus-creative-games/leanclr-demo) 提供两个平台的示例用于快速体验 LeanCLR 的功能：

| 示例 | 说明 |
|------|------|
| **win64** | Windows x64 平台示例，运行 `run.bat` 即可执行 |
| **h5** | WebAssembly 浏览器示例，通过 HTTP 服务器访问 `index.html` |

### leanclr-unity-demo

[leanclr-unity-demo](https://github.com/focus-creative-games/leanclr-unity-demo)项目展示如何使用leanclr-unity插件，在发布WebGL/小游戏/Win64目标平台时替换il2cpp为leanclr运行时。

## 相关仓库

| 仓库 | 说明 |
|------|------|
| [leanclr-unity](https://github.com/focus-creative-games/leanclr-unity) | leanclr-unity 是 LeanCLR 的 Unity 插件，发布 WebGL / 小游戏平台时替换 IL2CPP 为 LeanCLR，大幅缩减包体、降低内存占用 |
| [leanclr-godot](https://github.com/focus-creative-games/leanclr-godot) | LeanCLR Godot 插件（开发中） |
| [hybridclr](https://github.com/focus-creative-games/hybridclr) | **HybridCLR**：特性完整、零成本、高性能、低内存的 Unity 全平台原生 C# 热更新方案 |

## 联系方式

- 邮箱：leanclr#code-philosophy.com
- Discord 频道：<https://discord.gg/esAYcM6RDQ>
- QQ 群：1047250380
