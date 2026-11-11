# Minimal engine host contract

The self-owned entry point is `src/runtime/public/engine_host.h`, implemented in the shared runtime core. It uses only opaque session identities, native file buffers, numeric status and caller-owned error storage. It never exports a RuntimeType, MethodTable, managed object, exception layout, GC slot or CoreLib private structure. The older imported `bcl/coreclr-net10/host/leanclr_host_bridge.h` is an unimplemented historical callback-table proposal, not this executable hosting API.

## Lifecycle and ownership

`leanclr_engine_start` accepts versioned configuration, copies assembly/type names, loads the selected profile's CoreLib and entry assembly, validates and caches three managed methods, then invokes `Initialize`. The entry contract is exactly:

```csharp
public static void Initialize();
public static void Tick(float deltaSeconds);
public static void Shutdown();
```

Open generic entry types, missing methods, ambiguous overloads, instance methods and incorrect argument/return types are rejected. Resolution scans only that entry type at startup. Tick uses its cached method and a stack argument; the bridge performs no allocation, type lookup or assembly/method scan per frame. Managed code and first-use VM preparation can allocate and collect.

There is one owner and one initialization attempt per process. Invalid configuration can be corrected before an attempt begins. Once startup begins, initialization failure and shutdown are terminal. A second start is rejected without calling old host callbacks; it never replaces a live session. Shutdown is idempotent, including after failed startup; it is quiescence, not metadata unload, automatic finalization, editor domain reload or in-process restart. Runtime storage remains process-owned. A runtime already owned through another embedding API is not shut down by a failed engine start. Embedders must not concurrently initialize through different hosting APIs.

All VM operations run on the native thread that started the session. A required host callback supplies stable, nonzero thread identities; off-thread Tick/Shutdown is rejected before VM entry. Reentrant calls during initialization, Tick and Shutdown are rejected. Configuring the owner callback correctly is the embedder's responsibility. Terminal state queries, ticks, repeated starts and repeated stops do not invoke the released session's callbacks. Hosts must quiesce external callback/tick sources before freeing their context.

File callbacks return positive for a borrowed buffer, zero for missing files and negative for host failures. Every positive result receives exactly one release callback, including invalid buffers, copy allocation failure and over-budget rejection. The runtime copies bytes before releasing the host buffer. A failed callback retains its own buffer ownership. Assembly size defaults to 64 MiB; the facade count uses the profile default (net10: 16,384), with an optional startup override. Mono rejects a nonzero net10-only facade override. These limits are not a complete VM memory quota. Callbacks must not throw C++ exceptions.

Errors contain stable status, the runtime error code and a fixed-size phase message. They remain valid after return and allocate no error object. Managed exceptions are converted to status and cleared from the current exception root; the bridge does not call managed `ToString` while handling an error. A failed Tick leaves the session running; managed state may have changed and the host decides whether to stop. A failed managed Shutdown still makes the runtime terminal and is reported once; subsequent stop succeeds. Finite Tick deltas are required.

An optional opaque AOT module registry is the data symbol emitted by the **same LeanAOT/runtime build**, not an arbitrary pointer supplied by scripts. The runtime validates its registry ABI during initialization. This hosting gate currently exercises interpreted managed entries; generated-code/GC/EH acceptance is tested separately by the AOT suite.

## Godot and Unreal hooks

`src/host-bridge/engine_adapters.h` contains separate, noncopyable owners with actual lifecycle calls:

| Persistent owner | Initialization | Frame | Terminal exit |
| --- | --- | --- | --- |
| Godot autoload / extension owner | `enter_tree(config, error)` | `process(delta, error)` | `exit_tree(error)` |
| Unreal persistent module | `startup_module(config, error)` | `tick_module(delta, error)` | `shutdown_module(error)` |

The application's Godot `_enter_tree` / `_process` / `_exit_tree`, or Unreal `StartupModule` / registered ticker / `ShutdownModule`, forward to these hooks and report returned errors using the engine logger. Own one runtime for the application, not a runtime per scene. Remove engine tick/event registrations before terminal exit; keep loader context alive through stop. An adapter deliberately does not silently stop in its destructor, since destruction on the wrong thread would violate the VM contract. Exit must be explicit and checked.

These SDK-neutral adapters are compiled and exercised independently of runtime smokes. No Godot/Unreal SDK is installed in the local test environment. Real SDK module registration, packaged builds, platform resource loading and editor process-restart behavior remain integration acceptance requirements; the hook tests are not represented as successful engine/editor launches. In-process hot reload, engine object/Variant marshalling, dispatching worker threads and scene-owned restart are outside this minimal lifecycle contract.

## Independent acceptance

`src/tests/engine-host` compiles the public header as C and runs each scenario in a fresh process. Fourteen scenarios cover direct lifecycle, Godot hooks, Unreal hooks, invalid ABI/config, retry after validation failure, missing CoreLib, host loader failure, copy allocation failure, assembly byte limit, method signature/varargs/open-generic rejection, managed initialization failure and managed shutdown failure. Lifecycle also covers duplicate starts, startup callback reentry, actual native worker-thread rejection (native targets), non-finite delta, failed Tick followed by successful ticks/GC, double stop and Tick after stop. Acquired/released file counts must balance. Terminal callback access is forbidden by the fixture.

```powershell
python scripts/dotnet10/engine-contracts.py
python scripts/dotnet10/engine-contracts.py --shared
python scripts/dotnet10/engine-contracts.py --configuration Debug
# Put the MSVC ASan runtime DLL directory on PATH and allow fallible allocation:
$env:ASAN_OPTIONS='detect_leaks=0:halt_on_error=1:allocator_may_return_null=1'
python scripts/dotnet10/engine-contracts.py --configuration RelWithDebInfo --asan
python scripts/dotnet10/fetch-corelib.py
python scripts/dotnet10/engine-contracts.py --target wasm --runtime out/net10-runtime-win-x86/runtimes/win-x86/lib/net10.0 --emsdk D:/workspace/wasmclr/emsdk
```

Native Linux/ARM64 and Clang ASan/UBSan use the same independent suite in `net10-platforms.yml`. Their remote results must be collected before claiming those target gates pass. Native package/SDK tests stay separate from the 98-entry runtime/AOT matrix.

Local evidence: all 14 cases pass in Windows x64 Debug, MSVC ASan and shared-DLL Release, and in Node Wasm32 using 10.0.8 x86 CoreLib. The runtime and engine suites remain independent CTest/managed executables. No real engine SDK launch is counted in these results.
