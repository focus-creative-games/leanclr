using System;

/// <summary>
/// Marks a unit test as covering a LeanCLR internal call or intrinsic listed in
/// <c>src/leanaot/LeanAOT/icalls.json</c>, <c>intrinsics.json</c>, or <c>intrinsics_newobj.json</c>.
/// Used by <c>scripts/test/corlib-icall-coverage.py</c> to maintain ICALL_INTRINSIC_COVERAGE.md.
/// </summary>
[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
public sealed class CoversIcallAttribute : Attribute
{
    public CoversIcallAttribute(string icallName) => IcallName = icallName;

    public string IcallName { get; }
}
