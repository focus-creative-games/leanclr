using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;

namespace Tests.Bugs
{

    internal class OptionalParamBinding_2022_10_26 : GeneralTestCaseBase
    {
        public static int Hello(int x)
        {
            return x;
        }

        public static int Foo(int x = 1)
        {
            return x;
        }

        public static string Foo2(string s = "abc")
        {
            return s;
        }

        public static string FooStringNull(string s = null)
        {
            return s;
        }

        public static Action Foo3(Action a = null)
        {
            return a;
        }

        public enum Color
        {
            Red,
            Green,
            Blue,
        }

        public struct MyPoint
        {
            public int X;
            public int Y;
        }

        static Color Foo4(Color c = Color.Red)
        {
            return c;
        }

        public static int[] FooSzArray(int[] a = null)
        {
            return a;
        }

        public static int[,] FooMdArray(int[,] a = null)
        {
            return a;
        }

        public static int? FooNullableNull(int? x = null)
        {
            return x;
        }

        public static int? FooNullableVal(int? x = 42)
        {
            return x;
        }

        public static Color? FooNullableEnumNull(Color? c = null)
        {
            return c;
        }

        public static Color? FooNullableEnumVal(Color? c = Color.Blue)
        {
            return c;
        }

        public static MyPoint? FooNullableStructNull(MyPoint? p = null)
        {
            return p;
        }

        public static IntPtr FooIntPtr(IntPtr p = default)
        {
            return p;
        }

        public static UIntPtr FooUIntPtr(UIntPtr p = default)
        {
            return p;
        }

        public static MyPoint FooMyPointDefault(MyPoint p = default)
        {
            return p;
        }

        public static int FooIntDefault(int x = default) => x;

        public static long FooLongDefault(long x = default) => x;

        public static float FooFloatDefault(float x = default) => x;

        public static double FooDoubleDefault(double x = default) => x;

        public static byte FooByteDefault(byte x = default) => x;

        public static sbyte FooSByteDefault(sbyte x = default) => x;

        public static short FooInt16Default(short x = default) => x;

        public static ushort FooUInt16Default(ushort x = default) => x;

        public static uint FooUInt32Default(uint x = default) => x;

        public static ulong FooUInt64Default(ulong x = default) => x;

        public static bool FooBoolDefault(bool x = default) => x;

        public static char FooCharDefault(char x = default) => x;

        public static decimal FooDecimalDefault(decimal decimailDefaultValue = default) => decimailDefaultValue;

        [UnitTest]
        public void ParamIsNotOptional()
        {
            var method = GetType().GetMethod("Hello", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.False(method.GetParameters()[0].IsOptional);
        }

        [UnitTest]
        public void ParamIsOptional()
        {
            var method = GetType().GetMethod("Foo", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.True(method.GetParameters()[0].IsOptional);
        }

        [UnitTest]
        public void GetParamDefaultValue()
        {
            var method = GetType().GetMethod("Foo", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.Equal(1, method.GetParameters()[0].DefaultValue);
        }

        [UnitTest]
        public void invoke()
        {
            int x = (int)GetType().InvokeMember("Foo", BindingFlags.Public | BindingFlags.NonPublic |
            BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Equal(1, x);
        }

        [UnitTest]
        public void invoke2()
        {
            string x = (string)GetType().InvokeMember("Foo2", BindingFlags.Public | BindingFlags.NonPublic |
            BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Equal("abc", x);
        }

        [UnitTest]
        public void StringParamOptional_DefaultNull_DefaultValueAndInvoke()
        {
            var method = GetType().GetMethod("FooStringNull", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.True(method.GetParameters()[0].IsOptional);
            Assert.Null(method.GetParameters()[0].DefaultValue);

            string r = (string)GetType().InvokeMember("FooStringNull", BindingFlags.Public | BindingFlags.NonPublic |
                BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Null(r);
        }

        [UnitTest]
        public void invoke3()
        {
            Action x = (Action)GetType().InvokeMember("Foo3", BindingFlags.Public | BindingFlags.NonPublic |
            BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Null(x);
        }

        [UnitTest]
        public void invoke4()
        {
            Color x = (Color)GetType().InvokeMember("Foo4", BindingFlags.Public | BindingFlags.NonPublic |
            BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Equal(Color.Red, x);
        }

        [UnitTest]
        public void SzArrayParamOptional_DefaultNull_DefaultValueAndInvoke()
        {
            var method = GetType().GetMethod("FooSzArray", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.True(method.GetParameters()[0].IsOptional);
            Assert.Null(method.GetParameters()[0].DefaultValue);

            int[] r = (int[])GetType().InvokeMember("FooSzArray", BindingFlags.Public | BindingFlags.NonPublic |
                BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Null(r);
        }

        [UnitTest]
        public void MdArrayParamOptional_DefaultNull_DefaultValueAndInvoke()
        {
            var method = GetType().GetMethod("FooMdArray", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.True(method.GetParameters()[0].IsOptional);
            Assert.Null(method.GetParameters()[0].DefaultValue);

            int[,] r = (int[,])GetType().InvokeMember("FooMdArray", BindingFlags.Public | BindingFlags.NonPublic |
                BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Null(r);
        }

        [UnitTest]
        public void NullableParamOptional_DefaultNull_DefaultValueAndInvoke()
        {
            var method = GetType().GetMethod("FooNullableNull", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.True(method.GetParameters()[0].IsOptional);
            Assert.Null(method.GetParameters()[0].DefaultValue);

            int? r = (int?)GetType().InvokeMember("FooNullableNull", BindingFlags.Public | BindingFlags.NonPublic |
                BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Null(r);
        }

        [UnitTest]
        public void NullableParamOptional_DefaultConstant_DefaultValueAndInvoke()
        {
            var method = GetType().GetMethod("FooNullableVal", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.True(method.GetParameters()[0].IsOptional);
            Assert.Equal(42, method.GetParameters()[0].DefaultValue);

            int? r = (int?)GetType().InvokeMember("FooNullableVal", BindingFlags.Public | BindingFlags.NonPublic |
                BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Equal(42, r);
        }

        [UnitTest]
        public void NullableEnumOptional_DefaultNull_DefaultValueAndInvoke()
        {
            var method = GetType().GetMethod("FooNullableEnumNull", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.True(method.GetParameters()[0].IsOptional);
            Assert.Null(method.GetParameters()[0].DefaultValue);

            Color? r = (Color?)GetType().InvokeMember("FooNullableEnumNull", BindingFlags.Public | BindingFlags.NonPublic |
                BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Null(r);
        }

        [UnitTest]
        public void NullableEnumOptional_DefaultEnumConstant_DefaultValueAndInvoke()
        {
            var method = GetType().GetMethod("FooNullableEnumVal", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.True(method.GetParameters()[0].IsOptional);
            object dv = method.GetParameters()[0].DefaultValue;
            if (dv is Color boxedColor)
            {
                Assert.Equal(Color.Blue, boxedColor);
            }
            else if (dv is int boxedInt)
            {
                Assert.Equal((int)Color.Blue, boxedInt);
            }
            else
            {
                Assert.Fail($"FooNullableEnumVal default value: expected boxed Color or int: type:{dv?.GetType()} value:{dv}");
            }

            Color? r = (Color?)GetType().InvokeMember("FooNullableEnumVal", BindingFlags.Public | BindingFlags.NonPublic |
                BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Equal<Color?>((Color?)Color.Blue, r);
        }

        [UnitTest]
        public void NullableStructOptional_DefaultNull_DefaultValueAndInvoke()
        {
            var method = GetType().GetMethod("FooNullableStructNull", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.True(method.GetParameters()[0].IsOptional);
            Assert.Null(method.GetParameters()[0].DefaultValue);

            MyPoint? r = (MyPoint?)GetType().InvokeMember("FooNullableStructNull", BindingFlags.Public | BindingFlags.NonPublic |
                BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Null(r);
        }

        [UnitTest]
        public void NullableStructOptional_InvokeWithExplicitStructValue()
        {
            MyPoint? arg = new MyPoint { X = 11, Y = 22 };
            MyPoint? r = (MyPoint?)GetType().InvokeMember("FooNullableStructNull", BindingFlags.Public | BindingFlags.NonPublic |
                BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod, null, null,
                new object[] { arg });
            Assert.Equal(11, r.Value.X);
            Assert.Equal(22, r.Value.Y);
        }

        [UnitTest]
        public void IntPtrParamOptional_Default_DefaultValueAndInvoke()
        {
            var method = GetType().GetMethod("FooIntPtr", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.True(method.GetParameters()[0].IsOptional);
            object dv = method.GetParameters()[0].DefaultValue;
            if (dv is IntPtr ip)
            {
                Assert.True(ip == IntPtr.Zero);
            }
            else if (dv is int i)
            {
                Assert.Equal(0, i);
            }
            else if (dv is long l)
            {
                Assert.Equal(0L, l);
            }
            else
            {
                Assert.Fail("FooIntPtr default value: expected IntPtr, int, or long");
            }

            IntPtr r = (IntPtr)GetType().InvokeMember("FooIntPtr", BindingFlags.Public | BindingFlags.NonPublic |
                BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.True(r == IntPtr.Zero);
        }

        [UnitTest]
        public void UIntPtrParamOptional_Default_DefaultValueAndInvoke()
        {
            var method = GetType().GetMethod("FooUIntPtr", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.True(method.GetParameters()[0].IsOptional);
            object dv = method.GetParameters()[0].DefaultValue;
            if (dv is UIntPtr up)
            {
                Assert.True(up == UIntPtr.Zero);
            }
            else if (dv is uint u)
            {
                Assert.True(u == 0u);
            }
            else if (dv is int si)
            {
                Assert.Equal(0, si);
            }
            else if (dv is ulong ul)
            {
                Assert.True(ul == 0ul);
            }
            else
            {
                Assert.Fail($"FooUIntPtr default value: expected UIntPtr, uint, or ulong: type:{dv?.GetType()} value:{dv}");
            }

            UIntPtr r = (UIntPtr)GetType().InvokeMember("FooUIntPtr", BindingFlags.Public | BindingFlags.NonPublic |
                BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.True(r == UIntPtr.Zero);
        }

        private const BindingFlags OptionalPrimitiveBf =
            BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static;

        private const BindingFlags OptionalPrimitiveInvoke =
            BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static |
            BindingFlags.Instance | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding;

        private void AssertOptionalIntegralDefaultZero(string methodName)
        {
            var m = GetType().GetMethod(methodName, OptionalPrimitiveBf);
            Assert.True(m.GetParameters()[0].IsOptional);
            Assert.Equal(0L, Convert.ToInt64(m.GetParameters()[0].DefaultValue));
            object r = GetType().InvokeMember(methodName, OptionalPrimitiveInvoke, null, null, null);
            Assert.Equal(0L, Convert.ToInt64(r));
        }

        [UnitTest]
        public void IntOptional_DefaultValueAndOptionalParamBinding()
        {
            AssertOptionalIntegralDefaultZero("FooIntDefault");
        }

        [UnitTest]
        public void LongOptional_DefaultValueAndOptionalParamBinding()
        {
            AssertOptionalIntegralDefaultZero("FooLongDefault");
        }

        [UnitTest]
        public void ByteOptional_DefaultValueAndOptionalParamBinding()
        {
            AssertOptionalIntegralDefaultZero("FooByteDefault");
        }

        [UnitTest]
        public void SByteOptional_DefaultValueAndOptionalParamBinding()
        {
            AssertOptionalIntegralDefaultZero("FooSByteDefault");
        }

        [UnitTest]
        public void Int16Optional_DefaultValueAndOptionalParamBinding()
        {
            AssertOptionalIntegralDefaultZero("FooInt16Default");
        }

        [UnitTest]
        public void UInt16Optional_DefaultValueAndOptionalParamBinding()
        {
            AssertOptionalIntegralDefaultZero("FooUInt16Default");
        }

        [UnitTest]
        public void UInt32Optional_DefaultValueAndOptionalParamBinding()
        {
            AssertOptionalIntegralDefaultZero("FooUInt32Default");
        }

        [UnitTest]
        public void UInt64Optional_DefaultValueAndOptionalParamBinding()
        {
            AssertOptionalIntegralDefaultZero("FooUInt64Default");
        }

        [UnitTest]
        public void FloatOptional_DefaultValueAndOptionalParamBinding()
        {
            var m = GetType().GetMethod("FooFloatDefault", OptionalPrimitiveBf);
            Assert.True(m.GetParameters()[0].IsOptional);
            Assert.Equal(0f, Convert.ToSingle(m.GetParameters()[0].DefaultValue));
            object r = GetType().InvokeMember("FooFloatDefault", OptionalPrimitiveInvoke, null, null, null);
            Assert.Equal(0f, Convert.ToSingle(r));
        }

        [UnitTest]
        public void DoubleOptional_DefaultValueAndOptionalParamBinding()
        {
            var m = GetType().GetMethod("FooDoubleDefault", OptionalPrimitiveBf);
            Assert.True(m.GetParameters()[0].IsOptional);
            Assert.Equal(0d, Convert.ToDouble(m.GetParameters()[0].DefaultValue));
            object r = GetType().InvokeMember("FooDoubleDefault", OptionalPrimitiveInvoke, null, null, null);
            Assert.Equal(0d, Convert.ToDouble(r));
        }

        [UnitTest]
        public void BoolOptional_DefaultValueAndOptionalParamBinding()
        {
            var m = GetType().GetMethod("FooBoolDefault", OptionalPrimitiveBf);
            Assert.True(m.GetParameters()[0].IsOptional);
            Assert.Equal(false, (bool)m.GetParameters()[0].DefaultValue);
            object r = GetType().InvokeMember("FooBoolDefault", OptionalPrimitiveInvoke, null, null, null);
            Assert.Equal(false, (bool)r);
        }

        [UnitTest]
        public void CharOptional_DefaultValueAndOptionalParamBinding()
        {
            var m = GetType().GetMethod("FooCharDefault", OptionalPrimitiveBf);
            Assert.True(m.GetParameters()[0].IsOptional);
            Assert.Equal('\0', Convert.ToChar(m.GetParameters()[0].DefaultValue));
            object r = GetType().InvokeMember("FooCharDefault", OptionalPrimitiveInvoke, null, null, null);
            Assert.Equal('\0', Convert.ToChar(r));
        }

        [UnitTest]
        public void DecimalOptional_DefaultValueAndOptionalParamBinding()
        {
            var m = GetType().GetMethod("FooDecimalDefault", OptionalPrimitiveBf);
            Assert.True(m.GetParameters()[0].IsOptional);
            object dv = m.GetParameters()[0].DefaultValue;
            if (!(dv is decimal))
            {
                Assert.Fail("FooDecimalDefault DefaultValue should be boxed decimal");
            }

            Assert.True(decimal.Equals(0m, (decimal)dv));
            object r = GetType().InvokeMember("FooDecimalDefault", OptionalPrimitiveInvoke, null, null, null);
            if (!(r is decimal))
            {
                Assert.Fail("FooDecimalDefault invoke result should be boxed decimal");
            }

            Assert.True(decimal.Equals(0m, (decimal)r));
        }
    }
}
