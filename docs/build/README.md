# Build Documentation

This directory contains documentation related to building and integrating LeanCLR.

## Documents

| Document | Description |
|----------|-------------|
| [build_runtime.md](./build_runtime.md) | How to build the LeanCLR runtime library |
| [embed_leanclr.md](./embed_leanclr.md) | How to embed LeanCLR into your project |

## Overview

LeanCLR is built using CMake and supports multiple platforms:

- **Windows** - Visual Studio 2022 (MSVC)
- **Linux/macOS** - GCC/Clang
- **WebAssembly** - Emscripten

### Prerequisites

- **CMake** 3.15 or higher
- **C++17** compatible compiler
- **Visual Studio 2022** (for Windows development)
- **Emscripten SDK** (for WebAssembly builds)

## Quick Start

### Windows (Visual Studio)

```cmd
scripts\build.bat runtime Release
```

Or from `src/runtime`: `build.bat Release`

### Run all tests

```cmd
scripts\build.bat test build Release
scripts\build.bat test run Release
```

On Linux/macOS: `./scripts/ci.sh Release`

See [scripts/README.md](../../scripts/README.md) for the full script index.

### WebAssembly

```cmd
cd src/samples/lean-wasm
build-wasm.bat
```

For detailed instructions, see the individual documentation files.
