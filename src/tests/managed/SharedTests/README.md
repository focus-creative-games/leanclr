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
├── Fixtures/           # shared helper types (`Tests.Fixtures`)
├── Instructions/
│   ├── Arithmetic/
│   ├── Arrays/
│   ├── Boxing/
│   ├── Branches/
│   ├── Compare/
│   ├── Exceptions/
│   ├── Fields/
│   ├── Funcs/
│   ├── Memory/
│   ├── Misc/
│   ├── NotImpls/
│   ├── Objects/
│   ├── Ariths/         # div, rem, ckfinite
│   └── Converts/       # conv / conv.ovf
└── Runtime/
    ├── Misc/
    └── TC_*.cs         # runtime / C# language tests
```

## What belongs here

- Tests that must run in **both** the interpreter (`CoreTests`) and AOT (`AotTests`) pipelines.
- Shared fixture types under `Fixtures/` (`namespace Tests.Fixtures`).

Tests that depend on `CoreTests.App`, `Bootstrap/`, or `Regression/` stay under `CoreTests/`.

## Adding shared tests

1. Add the `.cs` file under the appropriate subdirectory.
2. Use namespace `Tests.Instruments.*`, `Tests.CSharp`, or `Tests.Mics`, and inherit `GeneralTestCaseBase`.
3. Ensure both `CoreTests.csproj` and `AotTests.csproj` include the `SharedTests` glob (already configured).
