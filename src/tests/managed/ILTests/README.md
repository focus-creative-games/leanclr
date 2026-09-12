# ILTests

IL assembly tests for cases that are hard to express reliably in C#.

## Layout

```
ILTests/
├── Instructions/          # .il sources (ilasm input)
│   └── conv.ovf.il
├── Wrappers/              # C# [UnitTest] wrappers calling IL methods
│   └── TC_conv_ovf.cs
└── ILTestsEntry.cs        # Assembly marker for the legacy source set
```

Build produces two assemblies:

| Assembly | Contents |
|----------|----------|
| `ILTests.Native.dll` | Pure IL from `Instructions/*.il` |
| `ILTests.dll` | C# wrapper tests referencing `ILTests.Native` |

## Adding a test

1. Add `Instructions/your_case.il` with public static methods on a type.
2. Add `Wrappers/TC_*.cs` with `[UnitTest]` methods that call the IL API and assert via `Assert`.
3. Ensure `.il` declares `.assembly ILTests.Native { }`.

Requires **ILAsm** NuGet package (restored automatically via `dotnet restore`).

## Running

The old Mono `ILTests.csproj`, `basic-tester`, `RunTests`, and `managed.sln` entries were removed with the Mono profile cleanup. Keep the source files here as legacy material and link migrated cases into `src/tests/managed-net10/ManagedNet10.LegacyTests`.

Run the current net10 regression suite with:

```batch
powershell -ExecutionPolicy Bypass -File scripts\dotnet10\interp-smoke.ps1 -Configuration Release -AssemblyName ManagedNet10.LegacyTests -Entry "ManagedNet10.LegacyTests.Program::RunAll"
```
