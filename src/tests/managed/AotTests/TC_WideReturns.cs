using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using GenericAotLibrary;

namespace AotTests
{
    public class TC_WideReturns
    {
        public enum LongEnum : long { Minimum = long.MinValue }
        [StructLayout(LayoutKind.Sequential)] public struct Packet { public long A; public double B; public object Reference; }
        public interface IWide { long Integer(long value); double Floating(double value); Packet Record(Packet value); LongEnum Enum(); }
        public class Target : IWide
        {
            [MethodImpl(MethodImplOptions.NoInlining)] public long Integer(long value) { return value; }
            [MethodImpl(MethodImplOptions.NoInlining)] public double Floating(double value) { return value; }
            [MethodImpl(MethodImplOptions.NoInlining)] public Packet Record(Packet value) { return value; }
            public LongEnum Enum() { return LongEnum.Minimum; }
        }
        public struct ValueTarget : IWide
        {
            public long Integer(long value) { return value; }
            public double Floating(double value) { return value; }
            public Packet Record(Packet value) { return value; }
            public LongEnum Enum() { return LongEnum.Minimum; }
        }
        [MethodImpl(MethodImplOptions.NoInlining)] public static long Integer(long value, long divisor) { return value / divisor; }
        [MethodImpl(MethodImplOptions.NoInlining)] public static ulong Unsigned(ulong value, ulong divisor) { return value / divisor; }
        [MethodImpl(MethodImplOptions.NoInlining)] public static double Floating(double value) { return value; }
        [MethodImpl(MethodImplOptions.NoInlining)] public static Packet Record(Packet value, int divisor) { int x = 12 / divisor; if (x == -1) value.A++; return value; }
        [MethodImpl(MethodImplOptions.NoInlining)] public static void RecordError() { Record(new Packet(), 0); }
        public static long ReadInteger(IWide value, long input) { return value.Integer(input); }
        [UnitTest] public void IntegerBoundaries()
        {
            Assert.True(Integer(long.MinValue, 1) == long.MinValue && Integer(long.MaxValue, 1) == long.MaxValue);
            Assert.True(Unsigned(ulong.MaxValue, 1) == ulong.MaxValue && Unsigned(0, 1) == 0);
            Assert.True(Functions.Identity<long>(long.MinValue) == long.MinValue && Functions.Identity<ulong>(ulong.MaxValue) == ulong.MaxValue);
        }
        [UnitTest] public void FloatingBits()
        {
            Assert.True(double.IsNaN(Floating(double.NaN)) && Floating(double.PositiveInfinity) == double.PositiveInfinity);
            double negativeZero = -1.0 / double.PositiveInfinity;
            Assert.True(1.0 / Floating(negativeZero) == double.NegativeInfinity);
            Assert.True(Functions.Identity<double>(-0.125) == -0.125);
        }
        [UnitTest] public void StructWithReference()
        {
            object marker = new object();
            Packet value = new Packet { A = long.MinValue, B = -0.25, Reference = marker };
            Packet result = Record(value, 3);
            Assert.True(result.A == value.A && result.B == value.B && result.Reference == marker);
            result = Functions.Identity<Packet>(value);
            Assert.True(result.A == value.A && result.Reference == marker);
        }
        [UnitTest] public void InterfaceAndEnum()
        {
            IWide target = new Target();
            Assert.True(target.Integer(long.MinValue) == long.MinValue && target.Floating(-0.5) == -0.5);
            Packet value = new Packet { A = 17, B = double.NegativeInfinity, Reference = target };
            Packet result = target.Record(value);
            Assert.True(result.A == 17 && result.B == double.NegativeInfinity && result.Reference == target);
            Assert.True(target.Enum() == LongEnum.Minimum);
            IWide boxed = new ValueTarget();
            Assert.True(boxed.Integer(long.MaxValue) == long.MaxValue && boxed.Floating(-0.25) == -0.25);
            Assert.True(boxed.Record(value).Reference == target && boxed.Enum() == LongEnum.Minimum);
        }
        [UnitTest] public void ErrorPropagation()
        {
            int caught = 0;
            try { Integer(1, 0); } catch (DivideByZeroException) { ++caught; }
            try { Unsigned(1, 0); } catch (DivideByZeroException) { ++caught; }
            try { RecordError(); } catch (DivideByZeroException) { ++caught; }
            try { Integer(long.MinValue, -1); } catch (OverflowException) { ++caught; }
            Assert.True(caught == 4);
        }
        [UnitTest] public void ReflectionAndDelegates()
        {
            Func<long, long, long> call = Integer;
            Assert.True(call(long.MaxValue, 1) == long.MaxValue);
            object result = typeof(TC_WideReturns).GetMethod("Floating").Invoke(null, new object[] { -0.125 });
            Assert.True((double)result == -0.125);
        }
        [UnitTest] public void GenericEhBoundary()
        {
            Assert.True(Eh.Echo<long>(long.MinValue, 0) == long.MinValue);
            Assert.True(double.IsNaN(Eh.Echo<double>(double.NaN, 0)));
        }
    }
}
