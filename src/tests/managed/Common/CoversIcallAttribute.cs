using System;

/// <summary>
/// Marks a unit test as covering a LeanCLR internal call or intrinsic listed in
/// the active <c>src/leanaot/LeanAOT/runtime-apis/coreclr-net10</c> catalog.
/// </summary>
[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
public sealed class CoversIcallAttribute : Attribute
{
    public CoversIcallAttribute(string icallName) => IcallName = icallName;

    public string IcallName { get; }
}
