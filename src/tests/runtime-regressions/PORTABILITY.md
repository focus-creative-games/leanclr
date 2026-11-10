# Core portability checks

CI runs the standalone native-data regressions on Windows x86/x64, Linux x64,
Linux ARM64, and wasm32 under Node. The CUSTOM host composition is linked and
checked separately. All jobs run on push/PR to core/main/master and manually.
These tests need no external BCL binaries. The managed suite remains a separate
local/full-test check; this matrix does not claim managed-suite target coverage.

Configure native sanitizers with `-DLEANCLR_SANITIZERS=address,undefined`. This
instruments all runtime layers as well as their consumers. The Clang-only
`-DLEANCLR_BUILD_IL_FUZZER=ON` adds `il-fuzz` and runtime coverage instrumentation.
For longer local fuzzing, copy fuzz-corpus into a writable output directory and
run `il-fuzz out/corpus -max_total_time=3600 -max_len=4096`. Keep generated inputs
out of the checked-in seed directory. A crash is a failing CI run; minimized
reproductions should become deterministic regression tests.

The harness decodes arbitrary code and exception ranges without executing managed
instructions. It bounds inputs to 4096 bytes to keep parser state and allocation
bounded; it is not an execution sandbox or a complete IL verifier.

Local portable builds exercise fallback implementations but do not substitute for
these target executions. An enabled workflow is not evidence that a remote run
has passed: inspect each architecture's actual GitHub Actions result after push.
