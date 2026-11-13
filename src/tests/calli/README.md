# calli contracts

Stage 03-A validates managed target provenance before dereference, full return/parameter
signatures, byref and instance flags. MethodHandle.GetFunctionPointer exports the same
managed descriptor as ldftn; Marshal/registered native callbacks use a separate native
ABI protocol. Arbitrary native addresses are never treated as metadata descriptors.

The index grows fallibly and records only escaped methods. Native tests cover invalid
addresses, different parameter counts/types/returns, invocation-convention metadata,
10,000 allocation-free repeat registrations/validations, and failed growth/retry.
Positive managed IL is compared against .NET 10. BadTarget/NullTarget/WrongSignature
are LeanCLR guard contracts and must not be run as arbitrary native calls on stock CLR.

Build CalliTests.csproj and configure this CMake directory with LEANCLR_CALLI_MANAGED_DIR
and LEANCLR_CALLI_RUNTIME_DIR. For Mono compile Program.cs with modern Roslyn using
-unsafe -target:library -nostdlib against mono-4.5/mscorlib.dll, and select MONO.

Unmanaged signature parsing is tested independently of call execution. At this stage
execution still rejects unmanaged conventions, ExplicitThis and unsupported modifiers;
subsequent 03-B to 03-F commits add those capabilities only behind their platform gates.
