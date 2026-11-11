extern alias GenericArguments;
using System;
using System.Reflection;
using GenericAotLibrary;
using OtherPayload = GenericArguments::GenericCollision.Payload;

namespace AotTests
{
    public class TC_GenericAot
    {
        public sealed class LocalReference { public int Marker; }
        public struct LocalValue { public long Marker; public object Reference; public double Number; }
        public struct Nested<T> { public T Value; public long Marker; }
        public class RefNest<T> { }
        public enum Wide : ulong { Value = 0xFEDCBA9876543210UL }
        public sealed class GenericReader<T> : IRead { public int Read() { return typeof(T) == typeof(int) ? 19 : 23; } }
        public sealed class PlainReader : IGeneric<int> { public int Read() { return 29; } }
        public sealed class Derived : VirtualBase
        {
            public override T Echo<T>(T value) { return Functions.Identity<T>(value); }
            // No concrete call anchor: correctly stays interpreted.
            public override T Dynamic<T>(T value) { return Functions.Identity<T>(value); }
        }
        public static int Interface(IRead value) { return value.Read(); }
        public static int GenericInterface(IGeneric<int> value) { return value.Read(); }

        [UnitTest] public void SignedValues()
        {
            Assert.True(Functions.Identity<int>(int.MinValue) == int.MinValue);
            Assert.True(Functions.Identity<long>(long.MinValue) == long.MinValue);
            Assert.True(Functions.Identity<sbyte>(-128) == -128 && Functions.Identity<short>(-32768) == -32768);
        }
        [UnitTest] public void UnsignedAndFloating()
        {
            Assert.True(Functions.Identity<uint>(uint.MaxValue) == uint.MaxValue);
            Assert.True(Functions.Identity<ulong>(ulong.MaxValue) == ulong.MaxValue);
            Assert.True(Functions.Identity<float>(1.25f) == 1.25f && Functions.Identity<double>(-17.125) == -17.125);
            Assert.True(Functions.Identity<bool>(true) && Functions.Identity<char>('中') == '中');
        }
        [UnitTest] public void NullAndReferences()
        {
            var value = new LocalReference { Marker = 37 };
            Assert.True(Functions.Identity<LocalReference>(value) == value);
            Assert.True(Functions.Identity<string>("hello") == "hello" && Functions.Identity<object>(null) == null);
        }
        [UnitTest] public void ValueWithReferences()
        {
            var reference = new LocalReference();
            var value = Functions.Identity<LocalValue>(new LocalValue { Marker = 0x123456789L, Reference = reference, Number = 0.125 });
            Assert.True(value.Marker == 0x123456789L && value.Reference == reference && value.Number == 0.125);
        }
        [UnitTest] public void NestedValues()
        {
            var value = new Nested<Nested<long>> { Value = new Nested<long> { Value = 0x234567891L, Marker = 31 }, Marker = 53 };
            var result = Functions.Identity<Nested<Nested<long>>>(value);
            Assert.True(result.Value.Value == value.Value.Value && result.Value.Marker == 31 && result.Marker == 53);
        }
        [UnitTest] public void SameSignatureContexts()
        {
            Assert.True(Functions.Context<int>() == 11 && Functions.Context<string>() == 22 && Functions.Context<LocalReference>() == 33);
        }
        [UnitTest] public void DeepTypeIdentity()
        {
            Assert.True(Functions.Identity<RefNest<RefNest<RefNest<RefNest<RefNest<RefNest<RefNest<RefNest<RefNest<RefNest<RefNest<RefNest<RefNest<RefNest<RefNest<RefNest<int>>>>>>>>>>>>>>>>>(null) == null);
        }
        [UnitTest] public void SameNamesDifferentAssemblies()
        {
            var left = Functions.Identity<GenericCollision.Payload>(new GenericCollision.Payload { Marker = 7 });
            var reference = new LocalReference();
            var right = Functions.Identity<OtherPayload>(new OtherPayload { Marker = 0x234567891L, Reference = reference });
            Assert.True(left.Marker == 7 && right.Marker == 0x234567891L && right.Reference == reference);
            Assert.True(Functions.SameAssembly<GenericCollision.Payload, AotMethodAttribute>());
            Assert.True(!Functions.SameAssembly<OtherPayload, AotMethodAttribute>());
            Assert.True(Functions.SameAssembly<OtherPayload, GenericArguments::GenericAotArguments.ReferenceValue>());
            Assert.True(!Functions.SameAssembly<GenericCollision.Payload, OtherPayload>());
        }
        [UnitTest] public void ThreeAssemblyContexts()
        {
            var local = new LocalReference { Marker = 61 };
            var box = new Box<LocalReference>(local);
            var result = box.Combine<OtherPayload>(local, new OtherPayload { Marker = 0x345678912L, Reference = local });
            Assert.True(box.Read() == local && result.Marker == 0x345678912L && result.Reference == local);
        }
        [UnitTest] public void GenericStatics()
        {
            Box<int>.Static = 73;
            Box<string>.Static = "separate";
            Assert.True(Box<int>.Static == 73 && Box<string>.Static == "separate");
            Assert.True(Box<int>.Initializations == 1 && Box<string>.Initializations == 1);
        }
        [UnitTest] public void Constraints()
        {
            Assert.True(GenericAotLibrary.Constraints.Read<Reader>(new Reader { Value = 39 }) == 39);
            var reference = new LocalReference();
            Assert.True(GenericAotLibrary.Constraints.Reference<LocalReference>(reference) == reference);
        }
        [UnitTest] public void InterfaceContexts()
        {
            var reader = new GenericReader<int>();
            Assert.True(reader.Read() == 19 && Interface(reader) == 19);
            Assert.True(GenericInterface(new PlainReader()) == 29);
        }
        [UnitTest] public void VirtualGeneric()
        {
            var concrete = new Derived();
            Assert.True(concrete.Echo<int>(17) == 17); // Explicit discovery anchor.
            VirtualBase value = concrete;
            Assert.True(value.Echo<int>(23) == 23);
        }
        [UnitTest] public void VirtualFallback()
        {
            VirtualBase value = new Derived();
            Assert.True(value.Dynamic<long>(0x456789123L) == 0x456789123L);
        }
        [UnitTest] public void Delegates()
        {
            Func<int, int> number = Functions.Identity<int>;
            Func<string, string> text = Functions.Identity<string>;
            Assert.True(number(47) == 47 && text("delegate") == "delegate");
        }
        [UnitTest] public void ReflectionRegistered()
        {
            var method = typeof(Functions).GetMethod("Identity").MakeGenericMethod(typeof(int));
            Assert.True((int)method.Invoke(null, new object[] { 83 }) == 83);
        }
        [UnitTest] public void ReflectionUnregistered()
        {
            var method = typeof(Functions).GetMethod("Identity").MakeGenericMethod(typeof(DateTime));
            var value = new DateTime(637000000000000000L);
            Assert.True(((DateTime)method.Invoke(null, new object[] { value })).Ticks == value.Ticks);
        }
        [UnitTest] public void Recursion()
        {
            Assert.True(Functions.Recurse<int>(7) == 7 && Functions.Grow<LocalReference>(0) == 0);
        }
        [UnitTest] public void ArraysAndEnums()
        {
            Assert.True(Functions.Array<int>(89)[0] == 89 && Functions.Array<string>("array")[0] == "array");
            Assert.True(Functions.Identity<Wide>(Wide.Value) == Wide.Value);
            int? value = 41;
            Assert.True(Functions.Identity<int?>(value).Value == 41);
        }
        [UnitTest] public void Boxing()
        {
            Assert.True(Functions.Box<int>(97) == 97 && Functions.Box<string>("box") == "box");
        }
        [UnitTest] public void ByRef()
        {
            int value = 3;
            Functions.ByRef<int>(ref value) = 101;
            Assert.True(value == 101);
        }
        [UnitTest] public void ExceptionPropagation()
        {
            bool caught = false;
            try { Functions.Divide<LocalReference>(0); }
            catch (DivideByZeroException) { caught = true; }
            Assert.True(caught && Functions.Divide<LocalReference>(3) == 4);
        }
        [UnitTest] public void ExplicitFallbacks()
        {
            Assert.True(Functions.WithCatch<int>(103) == 103 && Functions.Excluded<int>(107) == 107);
        }
        [UnitTest] public void GenericInheritance()
        {
            var reference = new LocalReference { Marker = 109 };
            var derived = new GenericDerived<LocalReference>(reference);
            Assert.True(derived.ReadValue() == reference);
            GenericBase<LocalReference> parent = derived;
            Assert.True(parent.ReadValue() == reference && parent.Value.Marker == 109);
        }
        public sealed class GenericDerived<T> : GenericBase<T>
        {
            public GenericDerived(T value) { Value = value; }
            public override T ReadValue() { return base.ReadValue(); }
        }
    }
}
