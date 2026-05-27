using System;

[AttributeUsage(AttributeTargets.Method | AttributeTargets.Constructor)]
public class AotMethodAttribute : Attribute
{
    private readonly bool _isAotMethod;
    public AotMethodAttribute(bool isAotMethod = true)
    {
        _isAotMethod = isAotMethod;
    }
}
