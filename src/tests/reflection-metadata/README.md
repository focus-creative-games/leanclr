# Reflection metadata contracts

Virtual enumeration is compared with stock .NET 10. Tests cover a three-level override,
protected methods, DeclaredOnly, ordinary overloads, inflated generic classes, generic
method overrides, and two distinct new virtual slots. Enumeration scans the hierarchy
once and uses a one-bit-per-slot bitmap (up to 512 slots inline) rather than pairwise
name/signature comparisons. Byref types still return no methods.

Build ReflectionMetadataTests.csproj and configure this CMake directory using
LEANCLR_REFLECTION_MANAGED_DIR and LEANCLR_REFLECTION_RUNTIME_DIR, as in the adjacent
reflection-invoke suite. For Mono, compile Program.cs with Roslyn against the Mono
mscorlib to ReflectionMetadataTests.dll, and select the MONO adapter.
