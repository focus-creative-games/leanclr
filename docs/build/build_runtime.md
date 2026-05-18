# Building the LeanCLR Runtime

This document describes how to build the LeanCLR runtime library from source.

## Prerequisites

### Required Tools

| Tool | Minimum Version | Purpose |
|------|-----------------|---------|
| CMake | 3.15+ | Build system generator |
| C++ Compiler | C++17 support | Compilation |
| Visual Studio | 2022 (v17) | Windows development (recommended) |

### Installing Prerequisites

#### CMake

Download and install from: https://cmake.org/download/

Verify installation:
```cmd
cmake --version
```

#### Visual Studio 2022

Download from: https://visualstudio.microsoft.com/

Ensure the following workloads are installed:
- Desktop development with C++

## Build Directory Structure

```
src/runtime/
├── CMakeLists.txt      # Main CMake configuration
├── build.bat           # Build script (compile or generate VS solution)
└── [source directories]

Build outputs (not under src/):
out/cmake/runtime/
├── Release-x64/        # Release build tree
├── Debug-x64/          # Debug build tree
└── vs-sln-x64/         # VS solution only (build.bat sln)
```

## Building on Windows

### Option 1: Using build.bat (Recommended)

The `build.bat` script automates the entire build process.

#### Basic Usage

```cmd
cd src/runtime
build.bat
```

This builds a Release x64 configuration by default.

#### Build Options

| Option | Description |
|--------|-------------|
| `sln` | Generate Visual Studio solution only (no compile) |
| `Debug` | Build with debug symbols |
| `Release` | Build optimized release (default) |
| `x86` | Build for 32-bit Windows |
| `x64` | Build for 64-bit Windows (default) |
| `clean` | Clean build directory before building |
| `shared` | Build shared library variant |

#### Examples

```cmd
# Debug build
build.bat Debug

# Release x86 build
build.bat Release x86

# Clean and rebuild
build.bat clean Release

# Debug x86 with clean
build.bat clean Debug x86

# Generate VS solution only (open in IDE, build from VS)
build.bat sln
build.bat sln x86
build.bat clean sln
```

### Option 2: Using Visual Studio

1. Generate the Visual Studio solution:
   ```cmd
   cd src/runtime
   build.bat sln
   ```

2. Open the generated solution under `out/cmake/runtime/vs-sln-x64/` (e.g. `leanclr.sln`)

3. Select the desired configuration (Debug/Release) and platform (x64/Win32)

4. Build the solution (Ctrl+Shift+B)

### Option 3: Manual CMake

```cmd
cd src/runtime
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

## Building for WebAssembly

For WebAssembly builds, see the [lean-wasm sample project](../../src/samples/lean-wasm/README.md).

### Quick Steps

1. Install Emscripten SDK:
   ```cmd
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   emsdk install latest
   emsdk activate latest
   ```

2. Activate environment:
   ```cmd
   emsdk_env.bat
   ```

3. Build:
   ```cmd
   cd src/samples/lean-wasm
   build-wasm.bat
   ```

## Build Output

After a successful build, the output files are located at:

### Windows
```
out/cmake/runtime/<Config>-x64/runtime_build/<Config>/leanclr.lib
```

Where `<Config>` is `Debug` or `Release` (e.g. `Release-x64` build tree).

### WebAssembly
```
src/samples/lean-wasm/build-wasm/bin/
├── lean.js
└── lean.wasm
```

## CMake Configuration Options

The LeanCLR CMakeLists.txt provides the following configuration:

| Setting | Value | Description |
|---------|-------|-------------|
| `CXX_STANDARD` | 17 | C++17 required |
| Library Type | STATIC | Builds as static library |
| Exceptions | Disabled | C++ exceptions disabled for smaller binary |

## Troubleshooting

### Common Issues

1. **CMake not found**
   - Ensure CMake is installed and added to PATH
   - Restart terminal after installation

2. **Visual Studio generator not found**
   - Install Visual Studio 2022 with C++ workload
   - If using a different VS version, modify `build.bat`:
     ```bat
     set VS_VERSION=16 2019  # For VS 2019
     ```

3. **Build fails with C++ standard errors**
   - Ensure your compiler supports C++17
   - Update Visual Studio to latest version

4. **Emscripten build fails**
   - Ensure `emsdk_env.bat` was run in the current terminal
   - Verify Emscripten installation: `emcc --version`

## Next Steps

After building the runtime, see [Embedding LeanCLR](./embed_leanclr.md) to learn how to integrate it into your project.
