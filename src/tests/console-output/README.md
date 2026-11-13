# Console output contracts

Public Console.WriteLine overloads must flow through Console.Out. The runtime must
not register a native string overload that bypasses SetOut, virtual TextWriter dispatch,
encoding, newline handling, or exception propagation. Only the underlying platform I/O
belongs in native adapters. Tests cover string/int/null output, embedded null and Unicode,
SetError, throwing writers, writer replacement/restoration, and the real default sink.

Build ConsoleOutputTests.csproj and run entries through leanrun, or configure this
CMake directory with LEANCLR_CONSOLE_MANAGED_DIR and LEANCLR_CONSOLE_RUNTIME_DIR.
The default test asserts that both actual output lines reach stdout. The same source
can be compiled against Mono mscorlib using Roslyn -nostdlib -target:library.

Mono minimal's default PORTABLE host does not provide the OS queries used by Mono's
Console initializer. Run this suite with LEANCLR_HOST=NATIVE or a console-capable custom
host. The CMake suite skips that unsupported composition. The CoreCLR portable adapter
maps standard handles through the existing C stdio abstraction, so Wasm retains default
output while managed TextWriter dispatch handles redirection.
