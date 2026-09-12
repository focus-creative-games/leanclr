# LeanAOT

LeanAOT is an Ahead-Of-Time (AOT) compiler that translates .NET managed assemblies into C++ source code, similar in concept to IL2CPP. The generated C++ code can then be compiled with a native toolchain and linked against the LeanCLR runtime to produce a self-contained native binary.

## Features

- Translates CIL bytecode from one or more managed assemblies into C++
- Supports generics, interfaces, delegates, and virtual dispatch
- Pluggable output: code generation targets can be extended
- Uses the current `coreclr-net10` runtime API profile by default

## Project Structure

| Project | Description |
|---|---|
| `LeanAOT` | CLI entry point |
| `LeanAOT.Core` | Assembly loading and metadata utilities |
| `LeanAOT.GenerationPlan` | Determines which types and methods to AOT |
| `LeanAOT.ToCpp` | C++ code generation backend |

## Build

```bat
# Publish a Release build to tools/leanaot/
scripts\publish_leanaot.bat
```

(`src\leanaot\publish.bat` forwards to the script above.)

## Usage

```
LeanAOT -d <dll-search-path> [-d <dll-search-path> ...]
        -a <assembly-name>   [-a <assembly-name> ...]
        [--leanaot-runtime-api-profile coreclr-net10]
        -o <output-dir>
```

### Options

| Option | Long form | Required | Description |
|---|---|---|---|
| `-d` | | Yes | Directory to search for DLL files. Repeat for multiple paths. |
| `-a` | `--assembly` | Yes | Name of the assembly to AOT (without `.dll` extension). Repeat for multiple assemblies. |
| `-o` | | Yes | Output directory for the generated C++ source files. |
| | `--leanaot-runtime-api-profile` | No | Runtime API profile to load. Defaults to `LEANCLR_RUNTIME_API_PROFILE` or `coreclr-net10`. |

### Example

```bat
LeanAOT ^
  -d artifacts\dotnet10-runtime-pack ^
  -d out\dotnet\ManagedNet10.Smoke\Release\net10.0 ^
  -a System.Private.CoreLib ^
  -a ManagedNet10.Smoke ^
  --leanaot-runtime-api-profile coreclr-net10 ^
  -o out\leanaot\ManagedNet10.Smoke
```

This scans the .NET 10 runtime pack and smoke assembly output, translates the requested assemblies to C++, and writes the generated source into `out\leanaot\ManagedNet10.Smoke`.
