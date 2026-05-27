# Project Structure

This document describes the directory structure and organization of the LeanCLR project.

## Overview

```
leanclr/
├── src/
│   ├── runtime/      # LeanCLR runtime core
│   ├── libraries/    # Base class libraries
│   ├── tools/        # Command-line tools
│   ├── samples/      # Sample projects
│   └── tests/        # Unit tests
├── docs/             # Documentation
└── tools/            # Build utilities
```

> **Note:** Demo examples have been moved to a separate repository: [leanclr-demo](https://github.com/focus-creative-games/leanclr-demo)

---

## src/runtime (Runtime Core)

Directory: `src/runtime`

Core implementation of the LeanCLR runtime. This is the heart of the project.

| Directory | Description |
|-----------|-------------|
| **metadata** | ECMA-335 metadata parsing and representation |
| **vm** | Virtual machine core (type system, method invocation, runtime state management) |
| **interp** | IL interpreter and IR interpreter implementation |
| **gc** | Garbage collector (in development) |
| **icalls** | Internal calls implementation |
| **alloc** | Memory allocators (metadata allocation, managed object allocation) |
| **intrinsics** | Intrinsic method implementations |
| **os** | OS abstraction layer |
| **utils** | Utility functions and helpers |
| **misc** | Miscellaneous components |

---

## src/libraries (Base Class Libraries)

Directory: `src/libraries`

Contains .NET Framework base class libraries required by the runtime:

- **mscorlib.dll** - Core runtime library
- **System.dll** - System namespace types
- **System.Core.dll** - LINQ and additional system types

> **Note:** Only .NET Standard 2.x / .NET Framework 4.x core libraries are verified.

---

## src/tools (Command-Line Tools)

Directory: `src/tools`

### lean

A command-line tool with embedded LeanCLR runtime that can directly load and run .NET assemblies.

**Basic Usage:**

```cmd
lean [options] <dll_name> [-- <dll_args>...]
```

**Options:**

| Option | Description |
|--------|-------------|
| `-l, --lib-dir <dir>` | Add a DLL search path (can be used multiple times) |
| `-e, --entry <entry>` | Specify entry method in format `Namespace.Type::Method` |
| `-h, --help` | Display help information |

**Example:**

```cmd
lean -l dotnetframework -l libs CoreTests -e test.App::Main
```

---

## src/samples (Sample Projects)

Directory: `src/samples`

| Project | Description |
|---------|-------------|
| **startup** | Win64 native platform sample project |
| **lean-wasm** | WebAssembly platform sample project |

---

## src/tests (Unit Tests)

Directory: `src/tests`

Unit test framework and test cases for the LeanCLR runtime.

| Directory | Description |
|-----------|-------------|
| **basic-tester** | C++ test runner (loads and executes managed test assemblies) |
| **managed** | C# managed test projects |

See [Test Framework Documentation](../src/tests/README.md) for details on how to add test cases.

---

## docs (Documentation)

Directory: `docs`

Project documentation including build guides, embedding instructions, and API references.

---

## tools (Build Utilities)

Directory: `tools`

Build and development utilities:

- **clang-format** - Code formatting configuration
