# Minimal .NET 10 globalization

The CoreCLR adapter preserves native Windows NLS on Windows. On other hosts,
the minimal profile selects CoreLib's invariant globalization algorithms before
first use, through runtime-bound GlobalizationMode getters. The validated gates
use Windows-flavor CoreLib 10.0.8 of matching pointer width (win-x86 for Wasm32,
win-x64 for 64-bit targets). Host OS and BCL flavor are different dimensions;
this does not claim compatibility with arbitrary Unix or Mono runtime packs.
The usual BCL layout and pointer-width checks still apply.
The AOT runtime-implemented catalog reserves these getters,
so generated code cannot replace the platform policy.

Invariant mode supports Unicode casing, ordinal comparison/search, invariant
number/date formatting, and the invariant Gregorian calendar through CoreLib's
managed implementation. Named cultures are not supported on these hosts;
CultureInfo construction follows CoreLib's PredefinedCulturesOnly policy
(default: throw CultureNotFoundException). Disabling that policy permits named
culture objects with invariant data, not localized behavior.

The unavailable native NLS locale/calendar/search/map entry points throw
NotSupportedException at the managed bridge on non-Windows hosts. They must not
silently return a no-match, fake ordering, or empty culture data. A full profile
will require a real globalization provider; ASCII folding is not a replacement
for linguistic ordering, normalization, accents, or Turkish casing.

The Mono adapters use their existing BCL globalization implementation and are
not switched to this CoreCLR policy. UE packaging and BCL fingerprinting belong
to the separate host repository.

Regression fixture: src/tests/managed-net10/ManagedNet10.GlobalizationContracts.
Compare against stock .NET 10 with DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1 on
non-Windows targets, and stock Windows defaults for the native NLS path.
