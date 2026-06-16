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

LeanCLR plans **Standard** and **Core** editions: **Core is trimmed from Standard**. The currently available release is **Standard (single-threaded)**; full multi-threading and the Core edition are still on the roadmap. See [Core & Standard](https://doc.leanclr.com/docs/intro/editions).

| Feature | Standard (current) | Core (planned) |
| - | - | - |
| Thread model | Single-threaded (multi-threading planned) | Single-threaded |
| Platform-dependent icalls | Partial, still evolving | No platform-specific code; C++11 only |
| GC | Precise mark-sweep full GC | Leaner active full GC |

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
