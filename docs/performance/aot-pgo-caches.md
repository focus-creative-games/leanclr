# Receiver-guided PGO cache decisions

PGO now combines method call/cost ranking with bounded receiver observations,
per-invocation cache reuse, seed-method budgets and stale-profile rejection.
All new optimization flags remain off by default.

## Collection

Build the runtime with `-DLEANCLR_ENABLE_PROFILE=ON` and generate with
`--leanaot-enable-pgo-profile`. Every generated interface dispatch records the
actual receiver before lookup, including hits from an enabled callsite cache.
Null receivers and runtime-disabled profiling do not add observations.
Interpreter-only callsites are not observed by this AOT instrumentation.

The receiver table has 256 callsite keys (actual caller plus IL offset), at
most eight hash probes and four receiver identities per site. Additional types
enter an explicit other count; full probe windows increment a dropped counter.
The observer does no allocation, type-name formatting or locking. With the current field layout the table occupies 63,488 nominal bytes on x64
and 49,152 on wasm32/Win32, plus 24 bytes of counters; a static assertion caps
the table at 64 KiB. An instrumented normal AOT frame carries one 64-bit
invocation ID, which the compiler may keep in a register.
This is profiling-only storage, compiled out of production builds; existing
method-profile slot storage remains separate.

Each site tracks shadow one-entry and two-entry round-robin reuse. A new or
reentered invocation resets history, so short repeated frames cannot look like
one persistent cache. Recursion can cause extra conservative resets; these
are not a promise of exact hardware/cache timing. Period reset/end operations
retain their documented period/global semantics and refresh period identities.
Counters saturate instead of wrapping. The runtime retains its single-thread
contract; this does not introduce concurrent profiling support.

JSON adds IL bytes, a FNV1a64 hash of the IL byte stream, receiver histograms,
shadow observations/hits and dropped observations. Method and receiver names
use explicit StringBuilder lengths. This also repairs a preexisting unbounded
method-name read in profile export, fixes JSON control escaping and checks
write/close failures.

## Conversion and generation

`pgo2aot convert` accepts repeated `-i` inputs as documented. Method snapshots
merge by max calls/cost. Receivers merge by identity/max counts; excess identities
become other. Complete reuse captures use the worst captured hit ratio when
merged. Conflicting IL fingerprints discard cache evidence. Invalid identities,
counters, offsets and capacities are rejected.

After normal threshold/top/pareto ranking, `--max-methods` and `--max-il-bytes`
optionally cap selected PGO roots. Oversized and unknown-size roots are skipped
when an IL budget is requested. These are **root IL budgets**, not bounds on
machine-code bytes, explicit roots or transitive generic closure; the existing
generic instance/depth budgets remain independent. Pareto aggregation no longer
wraps when multiple large 64-bit counters are combined.

Sites need at least 64 observations by default (`--min-site-calls`). A one-entry
shadow hit ratio of at least 90% selects one entry; otherwise a two-entry ratio
of at least 95% selects two. Complete new captures can therefore reject a cold
one-call frame and retain two entries for a four-type phase-local pattern.
Legacy captures without reuse data can use conservative type coverage at the
same thresholds; partially known invocation evidence does not justify a cache.

The converter ranks sites by observations and fits them within twelve pointer
words per method: three words for one entry, six for two. Generation rechecks
that candidates are actual interface calls in loops and enforces that budget.
Hints only apply when IL length/hash match the current method. Missing/stale
hints choose ordinary dispatch. Existing receiver/declaration guards, miss
resolution, failure handling, replacement policy and stack-local lifetimes
remain unchanged. No runtime global target cache or deoptimization is added.

Use `--leanaot-profiled-interface-caches --leanaot-interface-call-cache
--leanaot-pgo-rule-file pgo.xml` to consume hints. The report adds per-site
capacity decisions. Old PGO include files remain valid and supply no cache
hints. Generic closed-method profile signatures are not normalized to open
method definitions for hint matching; unmatched generic callers conservatively
keep ordinary dispatch in guided mode. Generic AOT selection still follows
static reachability and its independent budgets.

## Validation

```powershell
python scripts/test/pgo-caches.py --emsdk D:/workspace/wasmclr/emsdk
python scripts/benchmarks/compare_pgo_caches.py --build-only --emsdk D:/workspace/wasmclr/emsdk
# After all builds finish:
python scripts/benchmarks/compare_pgo_caches.py --skip-build --emsdk D:/workspace/wasmclr/emsdk
```

Four collection configurations and twelve guided/stale configurations pass
71 managed cases plus mono/bi/mega/mixed/hot-later/cold/phase scenarios each.
The pipeline asserts exact collection counts including cache hits, 0/1/2-entry
choices, mixed-site storage, hot-site priority, full stale fallback, all return
protocols plus reference sharing, Native/Wasm, Debug and Win32. CLI regressions
cover IL/root budgets, unknown sizes, oversized roots, Pareto overflow and
invalid counters. The profiling-enabled runtime suite passes 42/42, including
bounded probes, receiver overflow, period/global reset and shadow invocation
isolation. CI includes these checks; remote CI has not been run in this session.

## Recorded performance and decision

Median ns per loop iteration, seven samples of 500,000 iterations. Training
uses a separate profiling build; every timed variant disables profiling and
uses identical managed bytes. The training patterns repeat during evaluation,
so this is not evidence of generalization to an unseen workload.

| Workload | Native disabled | Native guided | Wasm disabled | Wasm guided |
| --- | ---: | ---: | ---: | ---: |
| monomorphic | 5.066 | 3.075 | 7.943 | 3.901 |
| alternating two types | 5.327 | 3.830 | 7.916 | 4.701 |
| generic interface | 16.249 | 3.145 | 23.560 | 4.757 |
| three types | 5.169 | 5.169 | 8.704 | 8.507 |
| phase change | 5.237 | 3.599 | 8.664 | 4.867 |
| five callsites | 31.178 | 18.297 | 34.414 | 21.519 |

Guided caching avoids the three-type miss penalty and preserves the two-type
phase benefit. It reduces many-site cost versus uniform two-entry caches
(26.377 -> 18.297 native, 29.373 -> 21.519 Wasm), using four single-entry sites
under the same frame storage budget. It is **not uniformly fastest**: uniform
single/double caches outperform guided code in some native microbenchmarks,
even where selected capacities match; code layout and measurement sensitivity
remain relevant. Straight-line control differences also show that sensitivity.
Do not treat capacity choice alone as proof of a universal speedup.

Native artifact sizes: disabled 896,512, single/guided 897,536, double 898,048
bytes. Wasm: disabled 845,618, single 846,199, double 846,862, guided 846,358
bytes. Guided code costs +740 bytes versus disabled and saves 504 versus
uniform double. All retain the twelve-word per-method cache storage bound.

Keep PGO an explicit workflow: collect representative data, set root budgets,
review per-site decisions, regenerate, then measure the application's own
workload. Raw timings/ranges, profiles, decisions, commands and artifact hashes:
[aot-pgo-caches.json](data/aot-pgo-caches.json).
