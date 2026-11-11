using System;
using System.Runtime.CompilerServices;
using GenericAotLibrary;

namespace AotTests
{
    public class TC_Int32Returns
    {
        public enum SignedCode : int { Minimum = int.MinValue }
        public enum UnsignedCode : uint { Maximum = uint.MaxValue }
        [MethodImpl(MethodImplOptions.NoInlining)] public static int SignedLeaf(int value) { return value; }
        [MethodImpl(MethodImplOptions.NoInlining)] public static uint UnsignedLeaf(uint value) { return value; }
        public static SignedCode EnumLeaf(SignedCode value) { return value; }
        public static UnsignedCode UnsignedEnumLeaf(UnsignedCode value) { return value; }
        public static ref int ByRef(ref int value) { return ref value; }
        public static long Wide(long value) { return value; }
        public static double Floating(double value) { return value; }
        [AotMethod(false)] public static int Interpreted(int value) { return value; }
        public interface IEnum { SignedCode Read(int value); }
        public interface IInteger { int Read(int value); }
        public sealed class Reader : IEnum, IInteger
        {
            SignedCode IEnum.Read(int value) { return (SignedCode)value; }
            int IInteger.Read(int value) { return value; }
        }
        public static SignedCode EnumCall(IEnum value, int input) { return value.Read(input); }
        public static int IntCall(IInteger value, int input) { return value.Read(input); }
        [UnitTest] public void Borders()
        {
            int[] signed = { int.MinValue, -1, 0, 1, int.MaxValue };
            uint[] unsigned = { 0, 1, 0x7FFFFFFFu, 0x80000000u, uint.MaxValue };
            for (int i = 0; i < signed.Length; ++i)
                Assert.True(SignedLeaf(signed[i]) == signed[i] && UnsignedLeaf(unsigned[i]) == unsigned[i]);
        }
        [UnitTest] public void Contexts()
        {
            Assert.True(Functions.Identity<int>(int.MinValue) == int.MinValue);
            Assert.True(Functions.Identity<uint>(uint.MaxValue) == uint.MaxValue);
            var definition = typeof(Functions).GetMethod("Identity");
            Assert.True((int)definition.MakeGenericMethod(typeof(int)).Invoke(null, new object[] { int.MinValue }) == int.MinValue);
            Assert.True((uint)definition.MakeGenericMethod(typeof(uint)).Invoke(null, new object[] { uint.MaxValue }) == uint.MaxValue);
        }
        [UnitTest] public void DispatchAndEnums()
        {
            var value = new Reader();
            Assert.True(EnumCall(value, int.MinValue) == SignedCode.Minimum);
            Assert.True(IntCall(value, int.MinValue) == int.MinValue);
            Assert.True(EnumLeaf(SignedCode.Minimum) == SignedCode.Minimum && UnsignedEnumLeaf(UnsignedCode.Maximum) == UnsignedCode.Maximum);
        }
        public static int Effects;
        public static int Divide(int divisor) { ++Effects; return 12 / divisor; }
        public static int CatchDivide(int divisor)
        {
            int before = Effects;
            try { return Divide(divisor); }
            catch (DivideByZeroException) { return int.MinValue + Effects - before; }
        }
        public static int CheckedAdd(int value) { return checked(value + 1); }
        [UnitTest] public void Errors()
        {
            Effects = 0;
            Assert.True(CatchDivide(0) == int.MinValue + 1 && Effects == 1);
            Assert.True(CatchDivide(3) == 4 && Effects == 2);
            bool caught = false;
            try { CheckedAdd(int.MaxValue); }
            catch (OverflowException) { caught = true; }
            Assert.True(caught && SignedLeaf(-1) == -1);
        }
        [UnitTest] public void RuntimeBoundaries()
        {
            Assert.True(Interpreted(int.MinValue) == int.MinValue);
            string value = "abc";
            Assert.True(value.Length == 3 && value.IndexOf('b') == 1 && Math.Abs(-7) == 7);
        }
        [UnitTest] public void OtherReturns()
        {
            int value = int.MinValue;
            ref int alias = ref ByRef(ref value);
            alias = int.MaxValue;
            Assert.True(value == int.MaxValue && Wide(long.MinValue) == long.MinValue && Floating(-0.25) == -0.25);
        }
        [UnitTest] public void Delegates()
        {
            Func<int, int> signed = SignedLeaf;
            Func<uint, uint> unsigned = UnsignedLeaf;
            Func<int, int> interpreted = Interpreted;
            Assert.True(signed(int.MinValue) == int.MinValue && unsigned(uint.MaxValue) == uint.MaxValue && interpreted(-1) == -1);
        }
    }
}
