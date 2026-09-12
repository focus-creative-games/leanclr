# Legacy Managed Test Sources

This directory is source material for the current .NET 10 legacy regression suite. It is not a standalone test entry.

Run managed regression tests through the .NET 10 entry points:

```batch
powershell -ExecutionPolicy Bypass -File scripts\dotnet10\interp-smoke.ps1 -Configuration Release -AssemblyName ManagedNet10.LegacyTests -Entry "ManagedNet10.LegacyTests.Program::RunAll"
powershell -ExecutionPolicy Bypass -File scripts\dotnet10\legacy-managed-audit.ps1
```

`src/tests/managed-net10/ManagedNet10.LegacyTests` links the active legacy sources from this directory and carries the small set of .NET 10-specific replacement tests.
