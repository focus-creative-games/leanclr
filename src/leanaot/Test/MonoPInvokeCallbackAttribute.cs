using System;
/// <summary>LeanAOT / Mono-style marker for reverse P/Invoke thunks (recognized by name <c>MonoPInvokeCallbackAttribute</c>).</summary>
[AttributeUsage(AttributeTargets.Method)]
internal sealed class MonoPInvokeCallbackAttribute : Attribute
{
    public MonoPInvokeCallbackAttribute(Type delegateType)
    {
        DelegateType = delegateType;
    }

    public Type DelegateType { get; }
}
