# Runtime core, host services and BCL adapter

Core now compiles as three object targets, composed into the existing `leanclr`
static/shared library. The public C API is unchanged. This checkout supplies the
Mono 4.5 BCL adapter; `LEANCLR_BCL_ADAPTER=MONO` is explicit, and other values are
rejected until their adapter implementation is supplied. Other branch worktrees
are not modified by this change.

| Layer | Target and sources | Responsibility |
| --- | --- | --- |
| Runtime core | `leanclr-core-objects`: `vm`, `metadata`, `interp`, `gc`, `codegen`, allocation and shared utilities | CLI metadata, object/array/string storage, type rules, execution, exception execution state and GC |
| Host services | `leanclr-host-objects`: `host`, native `platform` providers | Assembly bytes, stream handles, clocks, entropy, logging and native encoding/OS primitives |
| BCL adapter | `leanclr-bcl-mono-objects`: `bcl/mono`, existing `icalls`, `intrinsics`, `pinvokes`, `vmutils`, compression backend | Managed names/signatures, managed reflection/delegate/exception/thread/AppDomain behavior, BCL layout access and native PAL record conversion |

The object targets are independently compilable. Core uses the link-time
`bcl/adapter.h` contract for corlib discovery, ABI validation and binding
registration, and opaque VM facade declarations for managed reflection, delegates,
exception construction, stack traces, threads and domains. Their BCL-dependent
implementations now live in `bcl/mono`. Generic method metadata remains in core;
creation of managed method-body/reflection objects belongs to the adapter.
Current exception storage, exception GC roots, interpreter dispatch and AOT error
propagation remain core-owned.

## Managed layout boundary

`vm/rt_managed_types.h` defines only core intrinsic representations: object,
array/bounds, string, typed reference and native span. Adapter-owned objects such
as reflection types, delegates, exceptions, threads and AppDomain are opaque there.
Their definitions live in the private `bcl/mono/managed_layouts.h`. They retain the
`vm` namespace for compatibility with existing binding/generated code, without
making their layouts available to core translation units.

Opaque pointer alignment has an explicit minimum contract so `RtResult<T*>` has
the same tagged-pointer representation in core and adapter translation units.
Adapter static assertions validate that contract; corlib validation checks managed
object sizes. A core header cannot infer a Mono field offset or use `sizeof` on
these opaque objects. Core reads the intrinsic object header when needed, and uses
an adapter method to update BCL fields such as `Exception.inner_exception`.

The native assembly-name view is now `metadata::AssemblyNameInfo`;
`RtMonoAssemblyName` remains a source alias. Existing MonoPInvokeCallback AOT wire
record names remain compatible; they are native callback metadata, not managed
Mono object layouts.

`cmake/verify_layers.cmake` runs on every configure. It rejects private BCL binding
or OS provider includes in core, and managed/BCL includes in host/native provider
sources. The adapter contract header is checked too. `core-boundary` additionally
compiles core headers while asserting that key BCL objects remain incomplete, and
passes pointer results between opaque and complete-layout translation units.

## Host configuration

`host/services.h` is a native C++ interface with no managed object or metadata
headers. `Services` is exported for shared-library C++ consumers. Configure it on
the single host/runtime thread before the first service use or
`Runtime::initialize`; initialization freezes the table. `configure` returns
`false` without mutation for late configuration or an incomplete file table.
A callback state may change while retaining the same resource ownership backend.

Non-null fields override existing/default callbacks. Null fields leave them
unchanged. Stream file callbacks form one complete handle ownership domain:
standard handles, open, close, read, write, seek, length and file type must all be
provided together. Do not combine a custom handle creator with a default closer.
Callbacks and their state must remain usable for the runtime lifetime.

```cpp
#include "host/services.h"

bool configure_host(leanclr::host::FileLoader loader,
                    int64_t (*utc_100ns)(), int64_t (*steady_ns)())
{
    leanclr::host::ServiceCallbacks callbacks = {};
    callbacks.load_assembly = loader;
    callbacks.utc_ticks_100ns = utc_100ns;
    callbacks.monotonic_nanoseconds = steady_ns;
    return leanclr::host::Services::configure(callbacks);
}
```

The callback table currently covers assembly loading and stream I/O, not a complete
virtual filesystem. Legacy path/directory/stat, console control, timezone and
other PAL mappings keep their existing default behavior. Managed/native argument
conversion for those mappings is confined to the adapter's compatibility bridges
under `bcl/mono/platform`, rather than leaking managed types into native host
providers or core. Custom stream handles can be passed directly to managed
`FileStream`; path-based BCL APIs can also invoke those additional PAL operations.

`vm::Settings` file-loader/debugger setters remain startup facades over the same
table, and their typedefs alias host-native types. Setters used after freeze fail
explicitly; new code can use the boolean host API to report a configuration error.
A null debugger logger disables that channel before startup. Native callback
buffers are borrowed for the duration of a call and must not be retained.

Assembly loader success returns `host::FileData`. `shared=true` borrows bytes that
must remain valid for the loaded module; `shared=false` transfers bytes that must
be releasable by `GeneralAllocation::free`. Failed loading leaves the caller's
output unchanged. Install memory callbacks before any allocation-producing setup;
see [memory ownership and failure policy](../alloc/README.md).

## Units and behavior mappings

- UTC is **100ns since Unix epoch**. DateTime's 1601 FILETIME epoch conversion is
  a BCL adapter operation, not a host clock property.
- Monotonic time is **nanoseconds since an arbitrary steady epoch**. The Mono
  Stopwatch/Timer adapter converts to its 100ns tick unit; Environment.TickCount
  converts to wrapping milliseconds. Wall-clock changes cannot affect these paths.
- Entropy callbacks receive a `RandomKind`. Cryptographic requests must use a
  CSPRNG and return failure if unavailable. Defaults use Windows BCrypt or the
  native `/dev/urandom` device. Portable/sandboxed hosts without that source must
  supply one, e.g. Web Crypto. There is no insecure PRNG fallback. BCrypt signature
  and failure-status mapping is BCL behavior and is registered independently of
  the host OS, so Windows-built Mono BCL helpers can use a portable host callback.
- Internal diagnostics use UTF-8 text plus a native severity. The debugger channel
  uses borrowed UTF-16 spans and explicit lengths. Core diagnostics/fatal policy
  and debugger output route to these sinks.
- Stream paths are null-terminated UTF-16. Mode values are create-new=1, create=2,
  open=3, open-or-create=4, truncate=5, append=6; access is read=1, write=2, both=3;
  seek origins are begin=0, current=1, end=2. Status uses Win32-compatible native
  error numbers; the Mono binding maps its identical numeric error vocabulary.
  The callbacks operate on native integers/buffers, never Mono managed objects.

## Verification

The regression suite includes:

- `layer-boundaries`: configure-time source dependency checks (also verified with
  an intentionally forbidden relative include).
- `core-boundary`: opaque headers and `RtResult` ABI exchange across two TUs.
- `host-services`: injected clocks, stream handles, failed-loader output preservation,
  file-domain validation/locking, entropy failure/status mapping and log/debugger routing.
- `default-host-services`: native default clocks and bounded entropy output.
- `managed-host-services`: managed DateTime, Stopwatch, TickCount, FileStream on a
  host-owned in-memory stream, Guid entropy and managed entropy-failure handling.

Run with the instructions in [runtime regression tests](../../tests/runtime-regressions/README.md).
`LEANCLR_FORCE_PORTABLE=ON` exercises the C++ portable provider path on the current
machine; it is not evidence of ARM hardware or another BCL adapter being validated.

## Host selection

Configure `-DLEANCLR_HOST=NATIVE` (default), `PORTABLE`, or `CUSTOM`.
PORTABLE uses only C/C++ library host implementations, without Windows/POSIX SDK
headers or bcrypt. CUSTOM excludes the default file/time/random/heap sources and
starts with empty service and memory tables. Install complete MemoryCallbacks and
the required ServiceCallbacks before initialization. C/C++ standard library and
Mono adapter shims are still required; CUSTOM does not mean freestanding C++.
No native OS ABI is selected in PORTABLE or CUSTOM, even on a Windows compiler.
