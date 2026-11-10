using System;
using System.Runtime.InteropServices;

public interface ITypeMarker { }
public class TypeBase : ITypeMarker { }
public interface IProducer<out T> { }
public interface IConsumer<in T> { }
public interface IInvariant<T> { }
public class PlainParameter<T> { }
public class ReferenceParameter<T> where T : class { }
public class InterfaceParameter<T> where T : ITypeMarker { }
public class ValueParameter<T> where T : struct { }
public class BaseParameter<T> where T : TypeBase { }
public class ChainedParameters<T, U> where T : U where U : TypeBase { }

[StructLayout(LayoutKind.Explicit)]
public struct ExplicitReferences
{
    [FieldOffset(0)] public object First;
    [FieldOffset(0)] public object Alias;
    [FieldOffset(8)] public int Number;
}
public struct NestedReferences { public ExplicitReferences Value; public object Tail; }
public class LayoutHolder { public NestedReferences Value; }

public class TypeSystemFixture
{
    public static void MethodConstraint<T, U>() where T : U where U : TypeBase { }
    public static int Run()
    {
        Type plain = typeof(PlainParameter<>).GetGenericArguments()[0];
        Type reference = typeof(ReferenceParameter<>).GetGenericArguments()[0];
        Type iface = typeof(InterfaceParameter<>).GetGenericArguments()[0];
        Type value = typeof(ValueParameter<>).GetGenericArguments()[0];
        Type basis = typeof(BaseParameter<>).GetGenericArguments()[0];
        Type[] chain = typeof(ChainedParameters<,>).GetGenericArguments();
        Type[] method = typeof(TypeSystemFixture).GetMethod("MethodConstraint").GetGenericArguments();
        int errors = 0;
        if (!typeof(TypeBase).IsAssignableFrom(basis)) errors |= 1;
        if (!typeof(ITypeMarker).IsAssignableFrom(basis)) errors |= 2;
        if (basis.IsAssignableFrom(typeof(TypeBase))) errors |= 4;
        if (!chain[1].IsAssignableFrom(chain[0])) errors |= 8;
        if (chain[0].IsAssignableFrom(chain[1])) errors |= 16;
        if (!typeof(TypeBase).IsAssignableFrom(chain[0])) errors |= 32;
        if (!typeof(object).IsAssignableFrom(plain)) errors |= 64;
        if (!typeof(ValueType).IsAssignableFrom(value)) errors |= 128;
        if (typeof(ValueType).IsAssignableFrom(plain)) errors |= 256;
        if (!typeof(ITypeMarker).IsAssignableFrom(iface)) errors |= 512;
        if (typeof(IProducer<object>).IsAssignableFrom(typeof(IProducer<>).MakeGenericType(plain))) errors |= 1024;
        if (!typeof(IProducer<object>).IsAssignableFrom(typeof(IProducer<>).MakeGenericType(reference))) errors |= 2048;
        if (typeof(IProducer<object>).IsAssignableFrom(typeof(IProducer<>).MakeGenericType(iface))) errors |= 4096;
        if (!typeof(IProducer<object>).IsAssignableFrom(typeof(IProducer<>).MakeGenericType(basis))) errors |= 8192;
        if (typeof(IProducer<object>).IsAssignableFrom(typeof(IProducer<int>))) errors |= 16384;
        if (!typeof(IConsumer<string>).IsAssignableFrom(typeof(IConsumer<object>))) errors |= 32768;
        if (typeof(IInvariant<object>).IsAssignableFrom(typeof(IInvariant<string>))) errors |= 65536;
        if (typeof(object[]).IsAssignableFrom(plain.MakeArrayType())) errors |= 131072;
        if (!typeof(object[]).IsAssignableFrom(reference.MakeArrayType())) errors |= 262144;
        if (!method[1].IsAssignableFrom(method[0])) errors |= 524288;
        if (!typeof(TypeBase).IsAssignableFrom(method[0])) errors |= 1048576;
        ExplicitReferences union = new ExplicitReferences { First = new object(), Number = 17 };
        if (!object.ReferenceEquals(union.First, union.Alias) || union.Number != 17) errors |= 2097152;
        return errors;
    }
}

[System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential, Pack = 1)]
public struct PackedData
{
    public byte Tag;
    public long Integer;
    public double Real;
    public short Small;
}
public static class PackedMemoryFixture
{
    static void Change(ref long value) { value = -9223372036854775000L; }
    public static int Run()
    {
        PackedData data = new PackedData();
        data.Tag = 17;
        data.Integer = 0x123456789ABCDEF;
        data.Real = -3.25;
        data.Small = -12345;
        if (data.Integer != 0x123456789ABCDEF || data.Real != -3.25 || data.Small != -12345) return 1;
        Change(ref data.Integer);
        if (data.Integer != -9223372036854775000L) return 2;
        PackedData[] array = new PackedData[3];
        array[1] = data;
        array[2] = array[1];
        if (array[2].Integer != data.Integer || array[2].Real != data.Real || array[2].Tag != 17) return 3;
        object boxed = data;
        System.Reflection.FieldInfo integer = typeof(PackedData).GetField("Integer");
        if ((long)integer.GetValue(boxed) != data.Integer) return 4;
        integer.SetValue(boxed, 123456789012345L);
        if (((PackedData)boxed).Integer != 123456789012345L) return 5;
        return 0;
    }
}
