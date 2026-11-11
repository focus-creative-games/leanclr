# Minimal .NET 10 development status

This file tracks execution of `coreclr-net10-migration-plan.md`, not a claim of full framework compatibility.

| Stage | Status | Evidence / remaining work |
| --- | --- | --- |
| P0 baseline | Passed on Windows x64 | `421aecfd` registers native bindings, clones argv and makes unsupported calls fail explicitly. Both previously failing smoke methods now pass. 27 managed checks and handle contracts pass in Release and Debug. Static interface dispatch, zero-offset branch stack propagation, hardware fallback and real method preparation have regressions; Mono 41/41 and native generic/EH AOT checks pass. |
| P0 ABI / handles | Implemented; regression validation in progress | Canonical type handles and explicit RuntimeTypeHandle byrefs, two-slot TypedReference, typed/offset/size gates for native CoreLib views, exact metadata identities with fallible compact indexing and module cleanup. Inline arrays include repeated reference bitmaps; unmanaged function-pointer metadata parses without enabling an unsupported calli ABI. 77 managed entries pass in Release/Debug; invalid native handles and incompatible CoreLib rejection have regressions. |
| P0 GC / exceptions | Root and real-heap regressions implemented; broader EH/sanitizer validation in progress | 13 real-heap tests pass on Windows x64 Release/Debug and the reference .NET 10 runtime. Call safepoints retain live evaluation-stack slots; native arguments and exception flows are roots; small/large interior pointers use an ordered heap-range index; finalizer queues survive recursive collections. Nested catch-in-finally and calls at try boundaries have regressions. MSVC ASan passes the complete 13-case GC/EH executable; UBSan is still unavailable locally (broken WSL). The imported matrix currently passes 428/459 independent entries, with remaining failures classified in the next API stage. |
| P1 minimal API catalog | Pending | Full imported matrix and negative API contracts required. |
| P1 net10 AOT | Pending | Profile ABI, roots, inline arrays, varargs and fallback matrix required. |
| P1 platforms / budgets | Pending | Local WSL cannot start: its registered ext4.vhdx is missing. No Linux, ARM64 or Wasm pass is claimed from Windows-only testing. |
| P2 engine host | Pending | Self-owned host bridge, lifecycle and engine-specific tests required. |

The original migration report records the pre-development baseline; this ledger records later progress. Every completed stage must link its commits and reproducible test results.
