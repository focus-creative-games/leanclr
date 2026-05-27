using System;

[AttributeUsage(AttributeTargets.Class | AttributeTargets.Method | AttributeTargets.Constructor)]
public class IgnoreTestAttribute : Attribute
{
}
