# LeanAOT

LeanAOT is an Ahead-Of-Time (AOT) compiler that translates .NET managed assemblies into C++ source code, similar in concept to IL2CPP. The generated C++ code can then be compiled with a native toolchain and linked against the LeanCLR runtime to produce a self-contained native binary.

## Features

- Translates CIL bytecode from one or more managed assemblies into C++
- Supports generics, interfaces, delegates, and virtual dispatch
- Pluggable output: code generation targets can be extended
- Works with `.NET Framework 4.x` assemblies

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
        -o <output-dir>
```

### Options

| Option | Long form | Required | Description |
|---|---|---|---|
| `-d` | | Yes | Directory to search for DLL files. Repeat for multiple paths. |
| `-a` | `--assembly` | Yes | Name of the assembly to AOT (without `.dll` extension). Repeat for multiple assemblies. |
| `-o` | | Yes | Output directory for the generated C++ source files. |

### Example

```bat
LeanAOT ^
  -d libraries\mono-4.5 ^
  -d leanaot\Test\bin\Debug ^
  -a mscorlib ^
  -a Test ^
  -o samples\simple-aot\cpp
```

This will scan the two search paths for the `mscorlib` and `Test` assemblies, translate them to C++, and write the output into `samples\simple-aot\cpp`.
