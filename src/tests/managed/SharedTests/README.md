# SharedTests

Shared C# test sources compiled into multiple test assemblies (CoreTests, AotTests, …).

Each consumer project links these files with MSBuild `<Compile Include="..\SharedTests\..." Link="..." />` so the same tests run in both the interpreter and AOT pipelines without copy-paste.

## Layout

```
SharedTests/
└── Instructions/
    └── Converts/     # conv / conv.ovf IL instruction tests (21 files)
```

## Adding shared tests

1. Add the `.cs` file under the appropriate `SharedTests/` subdirectory.
2. Use namespace `Tests.Instruments.*` (or the matching area) and inherit `GeneralTestCaseBase`.
3. Reference the file from every assembly that should run the test (at minimum CoreTests and AotTests).
