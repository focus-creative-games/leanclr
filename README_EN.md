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

Detailed documentation is available under [docs](./docs):

- [Documentation Overview](./docs/README.md) - Documentation structure and navigation
- [Build Documentation](./docs/build/README.md) - Build-related documentation overview
- [Build Runtime](./docs/build/build_runtime.md) - How to build the LeanCLR runtime
- [Embed LeanCLR](./docs/build/embed_leanclr.md) - How to integrate LeanCLR into your project
- [AOT Documentation](./docs/aot.md) - AOT capabilities and usage
- [Test Framework](./src/tests/README.md) - Unit test framework and test authoring guide
- [Scripts](./scripts/README.md) - Build, test, and development scripts index

## Ecosystem & Integrations

LeanCLR already supports Unity and will support more engines/platforms soon.

| Platform | Status | Notes |
|------|------|------|
| **Unity / Unity China, WebGL and Mini-Game platforms** | Complete | [leanclr-unity](https://github.com/focus-creative-games/leanclr-unity): replace IL2CPP with LeanCLR when shipping games (not limited to WebGL/mini-game platforms) |
| **Godot (all platforms)** | In development | Preview planned for 2026-10 |
| **Unreal Engine (all platforms)** | In development | ETA TBD |

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
| **Garbage Collection** | ✅ Complete | mark-sweep precise full GC |
| **AOT Compiler** | ✅ Complete | IL → C++ transpilation supported |
| **Multi-threading** | 📋 Planned | Threads and synchronization primitives (Standard edition) |

### Stability

Current versions are highly stable:

- Fully compatible with Unity 2019.4.x – 6000.3.x LTS IL2CPP BCL, passing all (thousands of) test cases
- 99.95% compatible with Mono 4.8 BCL, with only one failing test case

## Editions

LeanCLR provides **Core** and **Standard** editions.  
The Core edition offers maximum portability, is single-threaded, and includes no platform-specific code; it can be directly compiled on all platforms with C++11 support, and is suitable as a pure scripting runtime.  
The Standard edition includes multi-threading and full platform-dependent BCL icalls, and is intended for full-featured CLR scenarios.

Main differences:

| Feature | Core | Standard |
| - | - | - |
| Thread model | Single-threaded | Multi-threaded |
| Platform-dependent icalls | Partial (only those implementable with C++11 standard library) | Full |
| GC | Active, precise full GC only | Precise, incremental GC with multiple GC strategies |

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
| [leanclr-godot](https://github.com/maidopi-usagi/leanCLR-godot) | LeanCLR Godot plugin |
| [hybridclr](https://github.com/focus-creative-games/hybridclr) | **HybridCLR**: full-featured, low-overhead, high-performance C# hot-update solution for Unity |

## Contact

- Email: leanclr#code-philosophy.com
- Discord: <https://discord.gg/esAYcM6RDQ>
- QQ Group: 1047250380
