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

- **极佳跨平台能力** — 使用AOT + Interpreter 混合执行架构，不支持JIT，使用标准 C++17 实现，无任何平台相关依赖。具有极佳的跨平台移植性，运行时要求小端字节序，具体目标平台仍需完成编译和测试验证。
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

## 生态与集成

LeanCLR 已支持 Unity 引擎，正在接入 Godot 与 CoreCLR BCL，并将持续扩展更多引擎和平台。

| 平台 / 集成 | 状态 | 说明 |
|------|------|------|
| **Unity 及团结引擎 WebGL 和小游戏平台** | ✅ 完成 | [leanclr-unity](https://github.com/focus-creative-games/leanclr-unity) 是 LeanCLR 的 Unity 插件，发布游戏（不限于 WebGL/小游戏平台）时替换 IL2CPP 为 LeanCLR |
| **Godot 全平台** | 🚧 开发中 | [leanclr-godot](https://github.com/focus-creative-games/leanclr-godot) 正在接入 Godot 引擎 |
| **CoreCLR .NET 10 BCL** | 🚧 开发中 | coreclr 分支正在支持 .NET 10 BCL |
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
| **多线程** | 📋 规划中 | 线程、同步原语及 GC 协作，当前两种宿主配置均为单线程 |

### 稳定性

以下为各分支整合前的历史兼容情况，不代表统一后所有构建配置的验证结果：

- **unity 分支**：与 Unity 2019.4.x – 6000.3.x LTS IL2CPP 的 BCL **完全兼容**，通过全部（数千个）测试用例
- **mono 分支**：历史测试与 Mono 4.8 的 BCL **99.95% 兼容**，仅一个测试用例失败；当前已对齐重构后的 core，需按配置重新验证
- **coreclr 分支**：正在支持 .NET 10 BCL，持续完善中

## 架构与构建配置

Core 与 Mono 已收敛为**同一套运行时实现**，以重构后的 core 为共享基线。
`core` / `mono` 分支名称暂时保留供现有集成迁移，部署能力通过构建配置选择。

| 配置 | BCL | 宿主能力 | 线程模型 |
|------|-----|----------|----------|
| **mono-minimal**（原 Core 定位） | Mono | 标准 C/C++ 库宿主，无 Windows/POSIX SDK 依赖 | 单线程 |
| **mono-native**（原 Mono 定位） | Mono | 现有原生平台服务，具体 API 支持取决于实现 | 单线程 |
| **mono-custom** | Mono | 应用注入内存、时钟、I/O 等宿主服务 | 单线程 |

三种配置共享类型系统、解释器、LeanAOT 和 GC。Minimal 仍可使用已有标准库 I/O；
需要完全接管宿主服务时选择 Custom。当前运行时要求小端平台与 C++17 工具链，
新平台仍需验证；Native 不代表已实现多线程或完整平台 API。

```powershell
scripts\runtime\build.bat Release x64 minimal
scripts\runtime\build.bat Release x64 native
```

Linux/macOS 对应 `./scripts/runtime/build.sh Release minimal` 或 `native`。
默认构建仍使用 Native，现有 CMake 目标 `leanclr` 和公共 API 保持不变。

CoreCLR BCL、Unity、Godot、UE 的长期方向是兼容层与宿主适配包；
它们目前仍保持原开发状态，本次仅完成 core/mono 收敛。

详见[架构树、分支迁移与构建入口](docs/architecture.md)、
[运行时分层](src/runtime/bcl/README.md)及[内存所有权与失败处理](src/runtime/alloc/README.md)。

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
