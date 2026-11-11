# Minimal .NET 10 API coverage

The API scanner uses catalog schema 2: **validated**, **unsupported**, and **pending**.
Unknown references are pending and fail the gate unless `--warn-only` is explicit.
An assembly or type match never implicitly validates its members. The initial exact
member list is deliberately small; smoke coverage does not establish every overload
or every generic instantiation. Unsupported wins over validated rules.

Positive and negative runtime contracts live in `ManagedNet10.ApiContracts`:
array interfaces and bounds, multidimensional cloning and nonzero lower bounds,
Span/collections/string operations, reflection, overlapping struct returns, and
explicit rejection of worker threads without executing their callbacks, and rejected
varargs/ArgIterator initialization (including default and forged native cookies).
Default RuntimeArgumentHandle is rejected by CoreLib with ArgumentException; a
nonzero cookie reaches the unsupported native initializer and receives
NotSupportedException without being dereferenced.

## Historical matrix

Windows x64 Release, installed Microsoft.NETCore.App 10.0.8: **434/459 groups pass**.
Groups overlap; this is not a count of individual tests. Each group runs in its own
process, with a timeout. Every failure is also run on stock .NET 10. No original
failure is silently skipped or treated as support.

| Entry | Stock .NET 10 | Classification |
| --- | --- | --- |
| `RunConvertI4` | Fail | Historical expectation also fails on stock runtime |
| `RunConvertInstructions` | Fail | Historical expectation also fails on stock runtime |
| `RunConvertSignedInteger` | Fail | Historical expectation also fails on stock runtime |
| `RunConvertU4` | Pass | Pending implementation |
| `RunConvertU8` | Pass | Pending implementation |
| `RunConvertUnsignedInteger` | Pass | Pending implementation |
| `RunCoreRegressionGenericMemberRef` | Pass | Pending implementation |
| `RunCoreRegressions` | Pass | Pending implementation |
| `RunCorlibConsole` | Pass | Pending implementation |
| `RunCorlibIO` | Pass | Unsupported OS P/Invoke; explicitly rejected |
| `RunCorlibIOCreateStreamOnly` | Pass | Unsupported OS P/Invoke; explicitly rejected |
| `RunCorlibIOCreateStreamWriteOnly` | Pass | Unsupported OS P/Invoke; explicitly rejected |
| `RunCorlibIOFileExistsOnly` | Pass | Unsupported OS P/Invoke; explicitly rejected |
| `RunCorlibIOFileStream` | Pass | Unsupported OS P/Invoke; explicitly rejected |
| `RunCorlibIOFullPathOnly` | Pass | Unsupported OS P/Invoke; explicitly rejected |
| `RunCorlibIOOpenHandleOnly` | Pass | Unsupported OS P/Invoke; explicitly rejected |
| `RunCorlibIOPathOnly` | Pass | Unsupported OS P/Invoke; explicitly rejected |
| `RunCorlibIOWriteOnly` | Pass | Unsupported OS P/Invoke; explicitly rejected |
| `RunCorlibThreadPool` | Pass | Unsupported worker scheduling in minimal profile |
| `RunCorlibThreading` | Pass | Unsupported worker scheduling in minimal profile |
| `RunRuntimeAsync` | Pass | Unsupported worker scheduling in minimal profile |
| `RunRuntimeAsyncTestEnum` | Pass | Unsupported worker scheduling in minimal profile |
| `RunRuntimeAsyncTestInt` | Pass | Unsupported worker scheduling in minimal profile |
| `RunRuntimeAsyncTestString` | Pass | Unsupported worker scheduling in minimal profile |
| `RunRuntimeAsyncTestStruct` | Pass | Unsupported worker scheduling in minimal profile |

Reproduce (after building the legacy project and leanrun):

```powershell
python scripts/dotnet10/legacy-matrix.py --runner out/net10-integration/bin/Release/leanrun.exe --runtime "C:/Program Files/dotnet/shared/Microsoft.NETCore.App/10.0.8" --output out/net10-legacy-differential.json
```

The JSON contains per-entry exit status, elapsed time and diagnostics, including the
reference runtime's diagnostics for every failure. This inventory command does not
turn a partial matrix into a passing compatibility gate.
