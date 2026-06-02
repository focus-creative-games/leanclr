# LeanCLR Test Framework

This directory contains the unit test framework and test cases for the LeanCLR runtime.

**See [TESTING.md](TESTING.md) for the full guide** (project boundaries, conventions, build/run instructions, and refactoring roadmap).

## Quick Start

```batch
rem 1. Build runner + managed tests + stage DLLs (recommended)
scripts\test\build-all.bat Debug x64

rem Or manually:
rem 1. Build the C++ test runner
scripts\test\basic-tester\build.bat Debug x64

rem 2. Build managed test assemblies
dotnet build src\tests\managed\managed.sln -c Debug

rem 3. Copy DLLs to the runner dlls/ folder, then run
scripts\test\run.bat Debug x64
```

On Linux/macOS, use the corresponding `.sh` scripts under `scripts/test/`.

## Directory Structure

```
tests/
├── basic-tester/          # C++ test runner (CI: CoreTests + CorlibTests + ILTests)
├── aot-tester/            # AOT test runner (AotTests)
├── managed/
│   ├── managed.sln
│   ├── Common/            # Shared test infrastructure
│   ├── SharedTests/       # Shared test sources (linked into CoreTests / AotTests)
│   ├── CoreTests/         # CLR runtime tests
│   ├── CorlibTests/       # BCL (mscorlib / System) tests
│   ├── AotTests/          # LeanAOT tests
│   ├── ILTests/           # IL asm tests (in solution; see ILTests/README.md)
│   └── RunTests/          # C# reflection runner (local dev)
└── TESTING.md
```

Official scripts live under [`scripts/test/`](../../scripts/test/). See [`scripts/README.md`](../../scripts/README.md).
