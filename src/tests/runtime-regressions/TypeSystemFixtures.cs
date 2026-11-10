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
