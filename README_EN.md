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

- **Strong cross-platform support** — AOT + Interpreter hybrid execution model, implemented in standard C++11 and free of platform-specific dependencies.
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

Current repository docs:

- [Documentation Index](docs/README.md)
- [.NET 10 Support Status](docs/dotnet10-support-review.md)
- [.NET 10 Runtime Contract](docs/net10-runtime-contract.md)
- [Testing](src/tests/TESTING.md)

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

LeanCLR already supports Unity and the .NET 10 BCL, is actively integrating Godot, and will keep expanding to more engines and platforms.

| Platform / Integration | Status | Notes |
|------|------|------|
| **Unity / Unity China, WebGL and Mini-Game platforms** | ✅ Complete | [leanclr-unity](https://github.com/focus-creative-games/leanclr-unity): replace IL2CPP with LeanCLR when shipping games (not limited to WebGL/mini-game platforms) |
| **CoreCLR .NET 10 BCL** | ✅ Baseline complete | The `coreclr` branch can load and run pure-logic .NET 10 (`net10.0`) assemblies and covers the core BCL; some uncommon low-level calls and libraries are still being completed. See [.NET 10 Support](#net-10-support-coreclr-branch) |
| **Godot (all platforms)** | 🚧 In development | [leanclr-godot](https://github.com/focus-creative-games/leanclr-godot) is being integrated with the Godot engine |
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

### Stability

The current Standard edition is highly stable:

- **unity branch**: fully compatible with Unity 2019.4.x – 6000.3.x LTS IL2CPP BCL, passing all (thousands of) test cases
- **mono branch**: 99.95% compatible with Mono 4.8 BCL, with only one failing test case
- **coreclr branch**: supports the .NET 10 (`net10.0`) BCL and can stably run pure-logic assemblies, with the existing runtime test assets regressed under .NET 10; some uncommon low-level calls and libraries are still being completed

### .NET 10 Support (coreclr branch)

The `coreclr` branch can already load .NET 10 (`net10.0`) assemblies against the `System.Private.CoreLib` world and execute them via the interpreter, covering the core BCL capabilities most commonly used by game and tooling projects:

- Base object model, strings, arrays, value types, enums, boxing/unboxing
- Generics, exceptions, delegates, and core IL semantics such as `isinst` / `castclass`
- Reflection, custom attribute reading, `RuntimeType` / `RuntimeHandle` handles
- `Span<T>` / `Unsafe` / `RuntimeHelpers`, RVA data, and inline arrays
- Single-threaded synchronization primitives (`Monitor` / `lock` / controlled `Task` continuations)
- Basic `System.IO` (files, paths, with cross-platform fallback)
- Limited `AssemblyLoadContext` / `Reflection.Emit` (light lambdas, dynamic assembly metadata)
- Reflection-based serialization (validated with real OdinSerializer serialize/deserialize round-trips)

#### Not Yet Supported or Limited

The current goal is to run controlled, pure-logic `net10.0` assemblies rather than to host the full `Microsoft.NETCore.App`. The following uncommon calls and low-level libraries are not yet supported and produce clear diagnostics instead of failing silently:

- Full `Microsoft.NETCore.App` (only a whitelisted BCL subset is supported on demand)
- Multi-threaded runtime: `ThreadPool`, `Timer`, background workers, cross-thread `Monitor`, complex `WaitHandle`
- Networking / sockets / HTTP and other platform IO, plus advanced IO such as file watchers, ACLs, overlapped I/O
- Dynamic native code generation: full `Reflection.Emit` IL emit, `DynamicMethod` native codegen
- Full CoreCLR `AssemblyLoadContext` (collectible / unload) and the debugger protocol
- COM interop and complex marshaling

> LeanCLR is currently documented as a single-threaded runtime; async/await is expressed through a single-threaded frame scheduler or the host dispatcher. A full multi-threaded runtime is outside the current support surface.

#### Full Test Suite

[NKGGameFramework](https://github.com/wqaetly/NKGGameFramework) provides a complete, real-world project test suite running on LeanCLR `.NET 10`, covering framework core logic, reflection, attribute enumeration, serialization (OdinSerializer), and async (UniTask) workloads. It serves as a reference baseline for the .NET 10 support scope.

## Editions

LeanCLR provides **Standard** and **Core** editions: **Core is trimmed from Standard**. Both editions currently ship under a single-threaded runtime boundary; a full multi-threaded runtime is outside the current support surface. See [Core & Standard](https://doc.leanclr.com/docs/intro/editions).

### Core Edition

The Core edition is implemented and runs on every platform with a C++11 toolchain. Core is designed as an embeddable pure scripting engine: you can run C# purely via the interpreter, or combine it with [LeanAOT](https://doc.leanclr.com/docs/aot/overview) to transpile hot IL to C++ for strong runtime performance.

### Standard Branches

Standard is split by BCL source:

| Branch | BCL Source | Notes |
|------|------------|------|
| **mono** | Mono BCL | General cross-platform integration; Mono 4.8 BCL compatible |
| **unity** | Unity IL2CPP BCL | For Unity / Unity China integration |
| **coreclr** | CoreCLR BCL | Supports the .NET 10 (`net10.0`) BCL; runs pure-logic assemblies |

### Standard vs Core

| Feature | Standard | Core |
| - | - | - |
| ECMA-335 | Standard implementation, high compatibility | Standard ECMA-335 specification |
| Thread model | Single-threaded | Single-threaded |
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
