# LeanRun

LeanRun is a command-line tool that embeds the LeanCLR runtime, allowing you to execute methods from .NET assemblies (DLL files) directly from the command line.

## Overview

LeanRun provides a lightweight way to run .NET assemblies without requiring a full .NET runtime installation. It supports:

- Executing the default entry point of an assembly
- Specifying a custom entry point method
- Multiple library search directories for resolving assembly dependencies
- Passing command-line arguments to the target assembly

## Building

### Prerequisites

Before building the project, ensure you have the following tools installed:

#### CMake

CMake 3.15 or higher is required.

- **Download**: https://cmake.org/download/
- Verify installation:
  ```cmd
  cmake --version
  ```

#### Visual Studio

Visual Studio 2022 with C++ development tools is required.

- **Download**: https://visualstudio.microsoft.com/
- Ensure the "Desktop development with C++" workload is installed

### Option 1: Generate Visual Studio Solution

1. Navigate to the `leanrun` tool directory:
   ```cmd
   cd src/tools/leanrun
   ```

2. Run the solution generator script:
   ```cmd
   generate-vs-sln.bat [ARCH]
   ```
   - `ARCH`: Optional. Specify `x64` (default) or `x86`

3. Open the generated solution file under `out/cmake/tools/leanrun/vs-sln-x64/`.

4. Build the project in Visual Studio (Ctrl+Shift+B)

### Option 2: Command-Line Build

1. Navigate to the `leanrun` tool directory:
   ```cmd
   cd src/tools/leanrun
   ```

2. Run the build script:
   ```cmd
   build.bat [CONFIG] [ARCH]
   ```
   - `CONFIG`: Optional. Specify `Debug` (default) or `Release`
   - `ARCH`: Optional. Specify `x64` (default) or `x86`

   Examples:
   ```cmd
   build.bat                    # Debug x64
   build.bat Release            # Release x64
   build.bat Release x86        # Release x86
   ```

### Build Output

After a successful build, the executable will be located at:

```
out/cmake/tools/leanrun/<CONFIG>-x64/bin/<CONFIG>/leanrun.exe
```

## Usage

### Basic Syntax

```cmd
leanrun [options] <dll_name> [-- <dll_args>...]
```

### Options

| Option | Description |
|--------|-------------|
| `-l, --lib-dir <dir>` | Add a library search directory for resolving assembly dependencies |
| `-e, --entry <entry>` | Specify a custom entry point (format: `FullClassName::MethodName`) |
| `-h, --help` | Display help information |
| `--` | Arguments after this are passed to the target DLL |

### Examples

#### Run an assembly with default entry point

```cmd
leanrun MyApp
```

This will:
1. Search for `MyApp.dll` in the current directory
2. Find and execute the assembly's default entry point

#### Specify library search directories

```cmd
leanrun -l . -l libs -l bin/Release MyApp
```

This will search for assemblies in the following order:
1. Current directory (`.`)
2. `libs` directory

#### Specify a custom entry point

```cmd
leanrun -e MyNamespace.MyClass::Main MyApp
```

This will execute the `Main` method in the `MyNamespace.MyClass` class.

#### Pass arguments to the target DLL

```cmd
leanrun -l . MyApp -- arg1 arg2 arg3
```

Arguments after `--` are passed to the target assembly and can be accessed via `Environment.GetCommandLineArgs()`.

#### Complete example

```cmd
leanrun -l . -l ../libs -e test.App::Run CoreTests -- --verbose --output result.txt
```

### Entry Point Requirements

The entry point method must satisfy the following conditions:

- Must be a **static** method
- Must have **no parameters**
- Must **not** be a generic method

### Exit Codes

| Code | Description |
|------|-------------|
| 0 | Success |
| -1 | Runtime error (initialization failed, assembly not found, method invocation failed, etc.) |
| 2 | Invalid command-line arguments |

## Troubleshooting

### Common Issues

1. **Assembly not found**
   - Ensure the DLL file exists in one of the library search directories
   - Use `-l` option to add additional search paths
   - Note: Do not include the `.dll` extension in the assembly name

2. **Entry point not found**
   - Check the entry point format: `Namespace.ClassName::MethodName`
   - Ensure the method is public and static

3. **Entry method must be static**
   - The specified entry point method must be declared as `static`

4. **Entry method must have no parameters**
   - The entry point method cannot accept any parameters
   - Use `Environment.GetCommandLineArgs()` to access command-line arguments within the DLL

## License

See the [LICENSE](../../../LICENSE) file in the project root for license information.
