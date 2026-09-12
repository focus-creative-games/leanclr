# LeanCLR Test Framework

This directory contains the current .NET 10 regression projects plus shared legacy source material used by those projects.

## Quick Start

```batch
rem Default .NET 10 interpreter smoke
powershell -ExecutionPolicy Bypass -File scripts\dotnet10\interp-smoke.ps1 -Configuration Release

rem Full linked legacy regression suite under net10.0
powershell -ExecutionPolicy Bypass -File scripts\dotnet10\interp-smoke.ps1 -Configuration Release -AssemblyName ManagedNet10.LegacyTests -Entry "ManagedNet10.LegacyTests.Program::RunAll"

rem API whitelist and runtime API signature gates
powershell -ExecutionPolicy Bypass -File scripts\dotnet10\api-scan.ps1 -Configuration Release
python src\generator\check_runtime_api_signatures.py --profile coreclr-net10 --repo-root .
```

## Directory Structure

```text
tests/
├── managed-net10/         # Current net10.0 smoke and regression projects
├── managed/               # Legacy source material; not a runnable test entry
└── TESTING.md
```

The old Mono-oriented `basic-tester`, `managed.sln`, and `scripts/test` entry points were removed with the Mono profile cleanup. Treat `managed-net10/` plus `scripts/dotnet10/` as the only runnable managed test entry. Do not delete `managed/` wholesale yet: `managed-net10/ManagedNet10.LegacyTests` still links source files from it.
