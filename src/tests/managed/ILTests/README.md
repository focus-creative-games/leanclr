# ILTests

IL assembly tests for cases that are hard to express reliably in C#.

## Layout

```
ILTests/
├── Instructions/          # .il sources (ilasm input)
│   └── conv.ovf.il
├── Wrappers/              # C# [UnitTest] wrappers calling IL methods
│   └── TC_conv_ovf.cs
├── ILTestsEntry.cs        # Assembly marker for RunTests
└── ILTests.csproj
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

- **basic-tester** (CI): loads `ILTests.Native.dll` then `ILTests.dll`, runs wrapper `[UnitTest]` methods.
- **RunTests**: includes `ILTests` assembly in reflection scan.

Build via `dotnet build managed.sln` or `scripts/test/build-all.bat`.
