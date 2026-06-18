# LeanCLR

Language: [中文](./README.md) | [English](./README_EN.md)

[![GitHub](https://img.shields.io/badge/GitHub-Repository-181717?logo=github)](https://github.com/focus-creative-games/leanclr) [![Gitee](https://img.shields.io/badge/Gitee-Repository-C71D23?logo=gitee)](https://gitee.com/focus-creative-games/leanclr)

[![license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/focus-creative-games/leanclr/blob/main/LICENSE) [![DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/focus-creative-games/leanclr) [![Discord](https://img.shields.io/badge/Discord-Join-7289DA?logo=discord&logoColor=white)](https://discord.gg/esAYcM6RDQ)

LeanCLR is a production-oriented CLR implementation. Its core goal is to provide high ECMA-335 compatibility, low integration complexity, and strong cross-platform capability, so developers can truly achieve **“Write C#, Run Anywhere.”**

## Why LeanCLR

For teams that need to embed C# logic into a host application and ship to multiple platforms, CoreCLR, Mono, and IL2CPP typically have the following limitations:

- **CoreCLR** and **Mono**: Feature-complete runtimes, but with relatively high binary size, dependency footprint, and host integration complexity. Their trimming and porting costs are often too high for lightweight embedded deployment scenarios.
- **IL2CPP**: Closed-source, tightly coupled to Unity tooling and ecosystem, AOT-only, and with limited ECMA-335 coverage.

LeanCLR is designed to fill this gap: maintain high ECMA-335 compatibility while delivering an embeddable, compact, and efficient cross-platform CLR.

## Key Features

- **Strong cross-platform support** — AOT + Interpreter hybrid execution model with no JIT, implemented in standard C++11 and free of platform-specific dependencies.
- **Easy integration** — Integration complexity is close to Lua; easy to embed into apps, games, embedded devices, IVI/automotive platforms, and more.
- **High ECMA-335 compatibility** — Near-complete support for ECMA-335 and major CoreCLR extensions, including generics, exceptions, reflection, and delegates.
- **Compact and efficient** — Small binary size, low memory usage, and high runtime efficiency; single-thread core build is under **600 KB** on x64/WebAssembly and can be reduced to around **300 KB** after trimming.

## Documentation

Full documentation site: **https://doc.leanclr.com**

- [Getting Started](https://doc.leanclr.com/docs/getting-started/overview)
- [Build & Integration](https://doc.leanclr.com/docs/integration/overview)
- [AOT](https://doc.leanclr.com/docs/aot/overview)
- [Interop](https://doc.leanclr.com/docs/interop/overview)
- [Testing](https://doc.leanclr.com/docs/development/testing)
- [Contributing](https://doc.leanclr.com/docs/development/contributing)

## Supported Platforms (Standard)

Standard edition currently supports:

| Platform | Notes |
|------|------|
| **Windows** | Desktop targets such as Win64 |
| **Linux** | Desktop and embedded Linux |
| **macOS** | Desktop targets |
| **Android** | Mobile |
| **iOS** | Mobile |
| **HarmonyOS** | Harmony ecosystem |
| **WebAssembly** | Web browsers and mini-game platforms |

## Ecosystem & Integrations

LeanCLR already supports Unity and is actively integrating Godot and CoreCLR BCL, with more engines and platforms to follow.

| Platform / Integration | Status | Notes |
|------|------|------|
| **Unity / Unity China, WebGL and Mini-Game platforms** | ✅ Complete | [leanclr-unity](https://github.com/focus-creative-games/leanclr-unity): replace IL2CPP with LeanCLR when shipping games (not limited to WebGL/mini-game platforms) |
| **Godot (all platforms)** | 🚧 In development | [leanclr-godot](https://github.com/focus-creative-games/leanclr-godot) is being integrated with the Godot engine |
| **CoreCLR .NET 10 BCL** | 🚧 In development | The Standard `coreclr` branch is adding .NET 10 BCL support |
| **Unreal Engine (all platforms)** | 📋 Planned | ETA TBD |

## Project Status

### Current Progress

| Module | Status | Notes |
|------|------|------|
| **Metadata Parsing** | ✅ Complete | Full PE/COFF and CLI metadata table support |
| **Type System** | ✅ Complete | Classes, interfaces, generics, arrays, value types, etc. |
| **IR Interpreter** | ✅ Complete | Optimized execution for hot functions |
| **Exception Handling** | ✅ Complete | try/catch/finally, nested exceptions, etc. |
| **Reflection** | ✅ Complete | Type, MethodInfo, FieldInfo, and other core APIs |
| **Delegates** | ✅ Complete | Unicast/multicast, generic delegates |
| **Internal Calls** | ✅ Complete | Currently focused on Core edition icalls |
| **P/Invoke** | ✅ Complete | Supports manual registration and LeanAOT-generated P/Invoke wrappers |
| **Garbage Collection** | ✅ Complete | Precise Mark-Sweep full GC |
| **AOT Compiler** | ✅ Complete | IL → C++ transpilation supported |
| **Multi-threading** | 📋 Planned | Threads and synchronization primitives (Standard edition) |

### Stability

The current Standard edition is highly stable:

- **unity branch**: fully compatible with Unity 2019.4.x – 6000.3.x LTS IL2CPP BCL, passing all (thousands of) test cases
- **mono branch**: 99.95% compatible with Mono 4.8 BCL, with only one failing test case
- **coreclr branch**: .NET 10 BCL support is in progress

## Editions

LeanCLR provides **Standard** and **Core** editions: **Core is trimmed from Standard**. Both are implemented (single-threaded); full multi-threading for Standard remains on the roadmap. See [Core & Standard](https://doc.leanclr.com/docs/intro/editions).

### Core Edition

The Core edition is implemented and runs on every platform with a C++11 toolchain. Core is designed as an embeddable pure scripting engine: you can run C# purely via the interpreter, or combine it with [LeanAOT](https://doc.leanclr.com/docs/aot/overview) to transpile hot IL to C++ for strong runtime performance.

### Standard Branches

Standard is split by BCL source:

| Branch | BCL Source | Notes |
|------|------------|------|
| **mono** | Mono BCL | General cross-platform integration; Mono 4.8 BCL compatible |
| **unity** | Unity IL2CPP BCL | For Unity / Unity China integration |
| **coreclr** | CoreCLR BCL | .NET 10 BCL support in progress |

### Standard vs Core

| Feature | Standard | Core |
| - | - | - |
| ECMA-335 | Standard implementation, high compatibility | Standard ECMA-335 specification |
| Thread model | Single-threaded (multi-threading planned) | Single-threaded |
| Cross-platform | Windows, Linux, macOS, Android, iOS, HarmonyOS, WebAssembly, etc. | All platforms; pure C++11 with no platform-specific dependencies |
| BCL | mono / unity / coreclr branches; platform icalls partially implemented | mono-4.5 BCL; only a subset of platform-related calls implemented |
| GC | Precise Mark-Sweep full GC | Precise Mark-Sweep full GC |

## Demo

### leanclr-demo

[leanclr-demo](https://github.com/focus-creative-games/leanclr-demo) provides two demos for quickly trying LeanCLR:

| Demo | Description |
|------|------|
| **win64** | Windows x64 demo; run `run.bat` |
| **h5** | WebAssembly browser demo; open `index.html` via an HTTP server |

### leanclr-unity-demo

[leanclr-unity-demo](https://github.com/focus-creative-games/leanclr-unity-demo) shows how to use `leanclr-unity` to replace IL2CPP with LeanCLR when shipping to WebGL, mini-game, and Win64 targets.

## Related Repositories

| Repository | Description |
|------|------|
| [leanclr-unity](https://github.com/focus-creative-games/leanclr-unity) | Unity plugin for LeanCLR; replace IL2CPP on WebGL / mini-game targets to reduce package size and memory usage |
| [leanclr-godot](https://github.com/focus-creative-games/leanclr-godot) | LeanCLR Godot plugin (in development) |
| [hybridclr](https://github.com/focus-creative-games/hybridclr) | **HybridCLR**: full-featured, low-overhead, high-performance C# hot-update solution for Unity |

## Contact

- Email: leanclr#code-philosophy.com
- Discord: <https://discord.gg/esAYcM6RDQ>
- QQ Group: 1047250380
