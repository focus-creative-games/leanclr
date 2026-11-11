# Normal-path AOT and exception handoff

Run from the repository root on Windows:

```powershell
python scripts/test/aot-features.py --target native
python scripts/test/aot-features.py --target all --emsdk D:/path/to/emsdk
python scripts/test/aot-features.py --target native --config Debug
```

The driver builds the tool and fixtures, generates C++, runs CTest, and also
checks generic AOT. `interpreted-loop` selects interpreter execution for the
same `NormalLoop` method, providing an apples-to-apples normal-path comparison.

Cases verify that heap side effects are not replayed, mutated arguments and
reference/scalar locals survive, normal/exceptional finally blocks execute once,
finally can replace an exception, and rethrow preserves control flow. Nested
finalizers, multiple leave targets, void/float/double/int64 returns, successful
AOT execution without interpreter translation, invalid resume locations, filter
fallback and unsafe address/complex-finally fallback are checked explicitly.

The first implementation conservatively retains interpreter execution for
filters/fault, generic EH methods, value-type `this`, unmanaged/byref/value-type
snapshots, address-taking, handlers nested inside a finally, uninitialized locals
and snapshots larger than 128 parameters/locals. This is not a promise to compile
all EH methods. See `generic-aot-report.json` in generated output for actual
normal-path methods and fallback reasons.

Normal paths keep null/bounds checks even when general generation flags disable
them. Exception handoff initializes interpreter code lazily, packs arguments and
locals only on the failing path, and starts exception dispatch at the mapped IL
site. It does not restart the instruction or switch the current invocation back
to AOT. Generated catch bodies remain interpreter-only; ordinary finally exits
use native continuation dispatch.

Keep the original IL and managed metadata available for handoff. Existing
interpreter semantics remain the fallback, including filter handling; this work
does not add a new cross-frame filter/search implementation.
