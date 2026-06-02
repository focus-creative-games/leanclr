# SharedTests

Shared C# test sources compiled into multiple test assemblies (`CoreTests`, `AotTests`, …).

Each consumer links these files with MSBuild:

```xml
<Compile Include="..\SharedTests\**\*.cs">
  <Link>Shared\%(RecursiveDir)%(Filename)%(Extension)</Link>
</Compile>
```

## Layout

```
SharedTests/
├── Instructions/
│   ├── Ariths/       # div, rem, ckfinite (5 files)
│   └── Converts/     # conv / conv.ovf (21 files)
└── Runtime/
    └── TC_String.cs
```

## What belongs here

- Tests that are **self-contained** (no `AOTDefs`, no `CoreTests.App` helpers).
- Tests that must run in **both** the interpreter (`CoreTests`) and AOT (`AotTests`) pipelines.

Tests that depend on `CoreTests/AOTDefs/` or other CoreTests-only fixtures stay under `CoreTests/` until fixtures are shared separately.

## Adding shared tests

1. Add the `.cs` file under the appropriate subdirectory.
2. Use namespace `Tests.Instruments.*` or `Tests.CSharp`, and inherit `GeneralTestCaseBase`.
3. Ensure both `CoreTests.csproj` and `AotTests.csproj` include the `SharedTests` glob (already configured).
