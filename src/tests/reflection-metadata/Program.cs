using System;
using System.Reflection;
[assembly:ReflectionMetadataTests.Mark("assembly", 7, Named = 8)]
[module:ReflectionMetadataTests.Mark("module", 7, Named = 8)]
namespace ReflectionMetadataTests
{
public class Base
{
    public virtual int V()
    {
        return 1;
    }
    protected virtual int P()
    {
        return 1;
    }
    public int Over(int x)
    {
        return x;
    }
    public int Over(string x)
    {
        return 2;
    }
    public virtual T G<T>(T x)
    {
        return x;
    }
}
public class Middle : Base
{
    public override int V()
    {
        return 2;
    }
    protected override int P()
    {
        return 2;
    }
    public override T G<T>(T x)
    {
        return x;
    }
}
public class Derived : Middle
{
    public override int V()
    {
        return 3;
    }
}
public class NewDerived : Base
{
    public new virtual int V()
    {
        return 4;
    }
}
public class GenericBase<T>
{
    public virtual T V(T x)
    {
        return x;
    }
    public virtual int V(int x)
    {
        return x;
    }
}
public class GenericDerived<T> : GenericBase<T>
{
    public override T V(T x)
    {
        return x;
    }
}
public class Generic<T>
{
    public class Nested<U>
    {
    }
}
public class Pair<A, B>
{
}
public static class Counters
{
    public static int Constructors;
    public static int Setters;
    public static int TypeInitializers;
}
[AttributeUsage(AttributeTargets.All)]
public class MarkAttribute : Attribute
{
    static MarkAttribute()
    {
        Counters.TypeInitializers++;
    }
    public MarkAttribute(string text, int value)
    {
        Counters.Constructors++;
    }
    public int Named
    {
        get {
            return 0;
        }
        set {
            Counters.Setters++;
        }
    }
}
public class BombAttribute : Attribute
{
    public BombAttribute()
    {
        throw new InvalidOperationException("attribute constructor");
    }
}
[Mark("type", 7, Named = 8)]
public class Decorated
{
    [Mark("field", 7, Named = 8)]
    public int Field;
    [Mark("property", 7, Named = 8)]
    public int Property { get; set; }
    [Mark("event", 7, Named = 8)]
    public event Action Event;
    [Mark("method", 7, Named = 8)]
    [return:Mark("return", 7, Named = 8)]
    public int Method([Mark("parameter", 7, Named = 8)] int x)
    {
        return x;
    }
    [Bomb]
    public void ThrowingAttribute()
    {
    }
}
[System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Explicit)]
public struct PseudoAttributeProbe
{
    [System.Runtime.InteropServices.FieldOffset(8)] public int Field;
}
public class Program
{
    static void Check(bool b, string message)
    {
        if (!b)
            throw new Exception(message);
    }
    static int Count(Type t, string name, BindingFlags flags)
    {
        int n = 0;
        foreach (var m in t.GetMethods(flags))
            if (m.Name == name)
                n++;
        return n;
    }
    public static void Overrides()
    {
        Check(Count(typeof(Derived), "V", BindingFlags.Instance | BindingFlags.Public) == 1, "three levels");
        Check(typeof(Derived).GetMethod("V").DeclaringType == typeof(Derived), "most derived");
    }
    public static void Overloads()
    {
        Check(Count(typeof(Derived), "Over", BindingFlags.Instance | BindingFlags.Public) == 2, "overloads");
    }
    public static void Protected()
    {
        Check(Count(typeof(Derived), "P", BindingFlags.Instance | BindingFlags.NonPublic) == 1, "protected override");
    }
    public static void Declared()
    {
        Check(Count(typeof(Derived), "V", BindingFlags.Instance | BindingFlags.Public | BindingFlags.DeclaredOnly) == 1,
              "declared override");
        Check(Count(typeof(Derived), "Over", BindingFlags.Instance | BindingFlags.Public | BindingFlags.DeclaredOnly) ==
                  0,
              "declared inherited");
    }
    public static void GenericOverrides()
    {
        Check(Count(typeof(GenericDerived<string>), "V", BindingFlags.Instance | BindingFlags.Public) == 2,
              "generic class overloads");
        Check(Count(typeof(Derived), "G", BindingFlags.Instance | BindingFlags.Public) == 1, "generic method override");
    }
    public static void NewSlots()
    {
        Check(Count(typeof(NewDerived), "V", BindingFlags.Instance | BindingFlags.Public) == 2, "distinct new slots");
    }
    public static void GenericDefinitionNames()
    {
        Type t = typeof(Generic<>);
        Check(t.Name == "Generic`1" && t.FullName == "ReflectionMetadataTests.Generic`1", "definition name");
        Check(t.ToString() == "ReflectionMetadataTests.Generic`1[T]", "definition display");
        Check(t.AssemblyQualifiedName == t.FullName + ", " + t.Assembly.FullName, "definition assembly");
        Check(typeof(Generic<>.Nested<>).FullName == "ReflectionMetadataTests.Generic`1+Nested`1", "nested definition");
    }
    public static void ClosedGenericNames()
    {
        Type t = typeof(Generic<string>);
        string expected = "ReflectionMetadataTests.Generic`1[[" + typeof(string).AssemblyQualifiedName + "]]";
        Check(t.FullName == expected, "closed full name: " + t.FullName);
        Check(t.AssemblyQualifiedName == expected + ", " + t.Assembly.FullName, "closed assembly");
        Check(t.ToString() == "ReflectionMetadataTests.Generic`1[System.String]", "closed display");
        Type nested = typeof(Generic<int>.Nested<string>);
        Check(nested.FullName == "ReflectionMetadataTests.Generic`1+Nested`1[[" + typeof(int).AssemblyQualifiedName +
                                     "],[" + typeof(string).AssemblyQualifiedName + "]]",
              "nested closed");
    }
    public static void OpenGenericNames()
    {
        Type p = typeof(Generic<>).GetGenericArguments()[0];
        Check(p.FullName == null && p.AssemblyQualifiedName == null && p.ToString() == "T", "parameter");
        Type partial = typeof(Pair<, >).MakeGenericType(typeof(int), p);
        Check(partial.FullName == null && partial.AssemblyQualifiedName == null, "partial");
        Check(partial.ToString() == "ReflectionMetadataTests.Pair`2[System.Int32,T]", "partial display");
        Check(p.MakeArrayType().FullName == null && partial.MakeArrayType().FullName == null, "open arrays");
        Check(p.MakeByRefType().FullName == null && p.MakePointerType().FullName == null, "open modifiers");
    }
    public static void ModifiedGenericNames()
    {
        Type t = typeof(Generic<string>);
        Check(t.MakeArrayType().FullName == t.FullName + "[]", "closed array");
        Check(t.MakeArrayType(2).FullName == t.FullName + "[,]", "closed rank");
        Check(t.MakeByRefType().FullName == t.FullName + "&", "closed byref");
        Check(t.MakePointerType().FullName == t.FullName + "*", "closed pointer");
        Check(t.MakeArrayType().AssemblyQualifiedName == t.FullName + "[], " + t.Assembly.FullName,
              "array assembly suffix");
        Check(typeof(Generic<>).MakeArrayType().FullName == "ReflectionMetadataTests.Generic`1[]", "definition array");
    }
    static void CheckData(System.Collections.Generic.IList<CustomAttributeData> data, string label)
    {
        int count = 0;
        foreach (var item in data)
        {
            if (item.Constructor.DeclaringType != typeof(MarkAttribute))
                continue;
            count++;
            Check((string)item.ConstructorArguments[0].Value == label && (int)item.ConstructorArguments[1].Value == 7,
                  "constructor metadata " + label);
            Check(item.NamedArguments.Count == 1 && item.NamedArguments[0].MemberName == "Named" &&
                      (int)item.NamedArguments[0].TypedValue.Value == 8,
                  "named metadata " + label);
        }
        Check(count == 1, "attribute count " + label);
        Check(Counters.Constructors == 0 && Counters.Setters == 0 && Counters.TypeInitializers == 0,
              "metadata executed attribute code");
    }
    public static void AttributeMetadata()
    {
        Type t = typeof(Decorated);
        CheckData(CustomAttributeData.GetCustomAttributes(t), "type");
        CheckData(CustomAttributeData.GetCustomAttributes(t.GetField("Field")), "field");
        CheckData(CustomAttributeData.GetCustomAttributes(t.GetProperty("Property")), "property");
        CheckData(CustomAttributeData.GetCustomAttributes(t.GetEvent("Event")), "event");
        MethodInfo m = t.GetMethod("Method");
        CheckData(CustomAttributeData.GetCustomAttributes(m), "method");
        CheckData(CustomAttributeData.GetCustomAttributes(m.GetParameters()[0]), "parameter");
        CheckData(CustomAttributeData.GetCustomAttributes(m.ReturnParameter), "return");
        CheckData(CustomAttributeData.GetCustomAttributes(t.Assembly), "assembly");
        CheckData(CustomAttributeData.GetCustomAttributes(t.Module), "module");
    }
    public static void ThrowingAttributeMetadata()
    {
        var m = typeof(Decorated).GetMethod("ThrowingAttribute");
        var data = CustomAttributeData.GetCustomAttributes(m);
        Check(data.Count == 1 && data[0].Constructor.DeclaringType == typeof(BombAttribute), "throwing metadata");
        Check(data[0].ConstructorArguments.Count == 0 && data[0].NamedArguments.Count == 0, "empty arguments");
    }
    public static void FieldPseudoAttributes()
    {
        var field = typeof(PseudoAttributeProbe).GetField("Field");
        var attributes = field.GetCustomAttributes(typeof(System.Runtime.InteropServices.FieldOffsetAttribute), false);
        Check(attributes.Length == 1 && ((System.Runtime.InteropServices.FieldOffsetAttribute)attributes[0]).Value == 8,
              "field offset pseudo attribute");
        Check(field.IsDefined(typeof(System.Runtime.InteropServices.FieldOffsetAttribute), false), "field offset IsDefined");
    }
    public static void PropertyAttributeInstances()
    {
        var property = typeof(Decorated).GetProperty("Property");
        Check(property.GetCustomAttributesData().Count == 1, "property metadata");
        Check(property.IsDefined(typeof(MarkAttribute), false), "property IsDefined");
        Check(Attribute.IsDefined(property, typeof(MarkAttribute)), "Attribute.IsDefined property");
        Check(Counters.Constructors == 0 && Counters.Setters == 0 && Counters.TypeInitializers == 0, "IsDefined executed user code");
        Check(property.GetCustomAttributes(typeof(MarkAttribute), false).Length == 1, "property typed instances");
        Check(property.GetCustomAttributes(false).Length == 1, "property untyped instances");
        Check(Counters.Constructors == 2 && Counters.Setters == 2, "property instance constructors and setters");
        Check(!property.IsDefined(typeof(BombAttribute), false), "absent property attribute");
        try { property.IsDefined(null, false); throw new Exception("null attribute type accepted"); }
        catch (ArgumentNullException) { }
    }
    public static void AttributeInstances()
    {
        var items = typeof(Decorated).GetCustomAttributes(typeof(MarkAttribute), false);
        Check(items.Length == 1 && Counters.Constructors == 1 && Counters.Setters == 1,
              "actual instances still execute user code");
    }
    public static void Main(string[] args)
    {
        typeof(Program).GetMethod(args[0]).Invoke(null, null);
        Console.WriteLine("PASS " + args[0]);
    }
}
}
