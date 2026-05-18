# LeanCLR

语言: [中文](./README.md) | [English](./README_EN.md)

[![GitHub](https://img.shields.io/badge/GitHub-Repository-181717?logo=github)](https://github.com/focus-creative-games/leanclr) [![Gitee](https://img.shields.io/badge/Gitee-Repository-C71D23?logo=gitee)](https://gitee.com/focus-creative-games/leanclr)

[![license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/focus-creative-games/leanclr/blob/main/LICENSE) [![DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/focus-creative-games/leanclr) [![Discord](https://img.shields.io/badge/Discord-Join-7289DA?logo=discord&logoColor=white)](https://discord.gg/esAYcM6RDQ)

LeanCLR 是一个面向全平台的精练的 CLR（Common Language Runtime）实现。LeanCLR 的设计目标是在高度符合 ECMA-335 规范的前提下，提供更紧凑、易嵌入、低内存占用的运行时，实现对移动端、H5 与小游戏等资源受限平台的友好支持。

## 为什么需要 LeanCLR

业界已有 CoreCLR、Mono、IL2CPP 等成熟的 CLR 实现，为什么还需要 LeanCLR？

1. CoreCLR、Mono、IL2PP发布后的代码体积过于庞大，发布后的二进制文件大小高达数M到数十M，不适合资源受限的平台如移动App或H5或小游戏平台。
2. IL2Cpp是闭源方案，且只支持AOT发布，对ECMA-335标准支持度不高。
3. CoreCLR代码过于庞大，Mono代码结构陈旧，两者都存在修改难度高，移植新平台难度高的问题。

LeanCLR 从零设计，专注于以下核心目标：

- **极致精简** — 单线程版本在 X64/WebAssembly 平台不到 **600 KB**，裁剪后可低至 **300 KB**
- **极佳跨平台能力** — 标准 C++17 实现，无任何平台相关依赖。使用AOT + Interpreter 混合执行架构，不支持JIT，具有极佳的跨平台移植性，不需要任何修改就可以在所有支持标准C++ 17编译器的平台运行。
- **易于集成** — 构建流程简单，引用LeanCLR的CMakelists.txt模块即可以完成构建，CLR API清晰，集成难度远远低于CoreCLR和Mono。
- **可维护性** — 代码结构清晰，便于理解、定制和二次开发

LeanCLR 特别适合以下场景：

- 移动端游戏和应用（iOS/Android）
- H5 游戏和小游戏平台（微信小游戏、抖音小游戏等）
- 需要热更新能力的游戏客户端
- 嵌入式系统和 IoT 设备

## 特性与优势

### 标准兼容性

- **高度兼容 ECMA-335** — 几乎完整实现 ECMA-335 规范，完整度高于 `IL2CPP + HybridCLR`，低于Mono。
- **支持现代 C# 特性** — 完整支持泛型、异常处理、反射、委托、LINQ 等核心特性
- **支持CoreCLR 扩展**（规划中）— 支持接口静态虚方法（.NET 7+）等扩展特性
- **精简设计** — 仅移除已废弃的过时特性（如 `arglist`、`jmp` 指令）

### 极致轻量

- **超小体积** — 单线程版本约 **600 KB**；裁剪 IR 解释器和非必要 icall 后可缩至 **300 KB** 甚至更小
- **低内存占用** — 优化的元数据表示，支持方法体元数据按需回收
- **精细化对齐** — 按元数据/托管对象的实际对齐需求使用独立分配池，避免统一 8 字节对齐的内存浪费
- **紧凑对象头** — 单线程版本对象头仅占一个指针大小

### 高效现代的运行模式

- **AOT + Interpreter 混合执行** — 兼顾启动速度和运行效率，优异的跨平台能力
- **代码 AOT** — 将 IL 代码转译为 C++（IL → C++）
- **双解释器架构** — IL 解释器处理冷路径，IR 解释器优化热点函数，平衡编译开销与执行性能
- **函数粒度 AOT** — 每个托管函数可以单独控制是否编译为AOT。如果只对性能敏感函数编译到AOT，可以保持整体良好性能的同时显著减少二进制代码大小。
- **异常路径兜底** — 异常处理由解释器统一兜底，显著简化 AOT 代码体积

### 跨平台能力

- **纯 C++ 实现** — 基于 C++17 标准，零平台特定依赖
- **无异常机制依赖** — 不依赖 C++ 异常，可在禁用异常的环境下编译运行
- **零移植成本** — 可直接编译到任何支持 C++17 的平台（Windows、Linux、macOS、iOS、Android、WebAssembly 等）

## 文档

详细文档位于 [docs](./docs) 目录：

- [文档概览](./docs/README.md) - 文档结构和导航
- [构建文档](./docs/build/README.md) - 构建相关文档概述
- [构建运行时](./docs/build/build_runtime.md) - 如何构建 LeanCLR 运行时
- [嵌入 LeanCLR](./docs/build/embed_leanclr.md) - 如何将 LeanCLR 集成到您的项目
- [AOT 文档](./docs/aot.md) - AOT 能力与使用说明
- [测试框架](./src/tests/README.md) - 单元测试框架和测试用例编写指南
- [脚本说明](./scripts/README.md) - 构建、测试与开发脚本索引

## 已集成的引擎和平台

LeanCLR本身是完全跨平台的，为了方便开发者使用，我们提供某些平台的集成：

|平台|状态|说明|
|-|-|-|
|微信小游戏和小应用| 部分完成| 提供[微信SDK](https://github.com/focus-creative-games/leanclr-sdk)，当前已封装部分API，可以纯C#开发微信小游戏和应用。|
|**Unity及团结引擎 WebGL和小游戏平台**|开发中| 预计2026年3月上旬发布。|
|**Unreal Engine 全平台**|开发中| 发布时间待定 |
|**Godot 全平台**|开发中| 发布时间待定 |

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
| **内部调用** | ✅ 完成 | 仅完成Core版本 icall |
| **P/Invoke** | ✅ 完成 | 已支持手动注册，自动化支持依赖 AOT 编译器 |
| **垃圾回收** | 📋 开发中 | 基础框架已就绪 |
| **AOT 编译器** | ✅ 完成 | 已支持 IL → C++ 转译 |
| **多线程** | 📋 规划中 | 线程、同步原语等 |

### 稳定性

目前版本版本已经达到**非常高**的稳定性水平。

- 与Unity 2019.4.x - 6000.3.x LTS il2cpp的 BCL 完全兼容，通过全部（数千个）测试用例。
- 与mono 4.8的 BCL 99.95%兼容，仅一个测试用例失败。

## 版本说明

LeanCLR 提供 **Core** 和 **Standard** 两个版本，以满足不同场景的需求。

### Core 版本

状态：**已完成**。

**设计目标**：最小化二进制代码大小和内存占用，最佳跨平台能力

- **单线程执行** — 简化内存模型，无需考虑线程同步
- **AOT + Interpreter 混合执行** — 灵活的执行策略
- **准确+协助式 GC** — 精确追踪托管引用，内存回收高效
- **零平台依赖** — 不依赖任何操作系统或平台特定函数
- **标准 C++17 实现** — 可直接编译运行于任何支持 C++17 标准的平台
- **未实现平台相关 icall** — 如 `System.IO.File` 等需自行桥接或使用纯托管实现

**最佳适用场景**：移动应用App、手游App、WebAssembly、嵌入式系统、IoT 设备、对跨平台一致性要求极高的项目

### Standard 版本

状态：**规划中**。

**设计目标**：功能完整的生产级运行时

- **多线程支持** — 完整的线程模型和同步原语
- **AOT + Interpreter 混合执行** — 灵活的执行策略
- **保守式 GC** — 兼容性更好，适合复杂应用场景
- **完整平台 icall** — 实现 `System.IO`、`System.Net` 等平台相关功能
- **标准 C++17 实现** — 移植到新平台时需要适配少量平台相关接口

**最佳适用场景**：桌面应用、重度移动端App和游戏、需要完整 .NET 基础库功能的项目


## Demo

[leanclr-demo](https://github.com/focus-creative-games/leanclr-demo) 提供两个平台的示例用于快速体验 LeanCLR 的功能：

| 示例 | 说明 |
|------|------|
| **win64** | Windows x64 平台示例，运行 `run.bat` 即可执行 |
| **h5** | WebAssembly 浏览器示例，通过 HTTP 服务器访问 `index.html` |

## 联系方式

- 邮箱：leanclr#code-philosophy.com
- discord 频道： <https://discord.gg/esAYcM6RDQ>
- QQ群：1047250380
