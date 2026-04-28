using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Tests.CSharp
{

    class ConstValues
    {
        public const string Str = "abc";

        public const byte x1 = 10;

        public const sbyte x2 = 1;

        public const sbyte x3 = -1;

        public const short x4 = -2;

        public const short x5 = 2;

        public const int x6 = 3;

        public const int x7 = -3;

        public const long x8 = -4;

        public const long x9 = 4;

        public const float y1 = 1f;

        public const double y2 = 2f;
    }

    class FT_Property
    {
        public int Value { get; set; }
    }

    internal class TC_Reflection
    {
        [UnitTest]
        public void const_string()
        {
            var strField = typeof(ConstValues).GetField("Str");
            string s = (string)strField.GetValue(null);
            Assert.Equal("abc", s);
        }

        [UnitTest]
        public void const_byte()
        {
            var strField = typeof(ConstValues).GetField("x1");
            byte s = (byte)strField.GetValue(null);
            Assert.Equal(10, s);
        }

        [UnitTest]
        public void const_sbyte()
        {
            var strField = typeof(ConstValues).GetField("x2");
            sbyte s = (sbyte)strField.GetValue(null);
            Assert.Equal(1, s);
        }

        [UnitTest]
        public void const_sbyte2()
        {
            var strField = typeof(ConstValues).GetField("x3");
            sbyte s = (sbyte)strField.GetValue(null);
            Assert.Equal(-1, s);
        }

        [UnitTest]
        public void const_short()
        {
            var strField = typeof(ConstValues).GetField("x4");
            var s = (short)strField.GetValue(null);
            Assert.Equal(-2, s);
        }

        [UnitTest]
        public void const_short2()
        {
            var strField = typeof(ConstValues).GetField("x5");
            var s = (short)strField.GetValue(null);
            Assert.Equal(2, s);
        }

        [UnitTest]
        public void const_int()
        {
            var strField = typeof(ConstValues).GetField("x6");
            int s = (int)strField.GetValue(null);
            Assert.Equal(3, s);
        }

        [UnitTest]
        public void const_int2()
        {
            var strField = typeof(ConstValues).GetField("x7");
            var s = (int)strField.GetValue(null);
            Assert.Equal(-3, s);
        }

        [UnitTest]
        public void const_long()
        {
            var strField = typeof(ConstValues).GetField("x8");
            var s = (long)strField.GetValue(null);
            Assert.Equal(-4, s);
        }

        [UnitTest]
        public void const_long2()
        {
            var strField = typeof(ConstValues).GetField("x9");
            var s = (long)strField.GetValue(null);
            Assert.Equal(4, s);
        }


        [UnitTest]
        public void const_float()
        {
            var strField = typeof(ConstValues).GetField("y1");
            var s = (float)strField.GetValue(null);
            Assert.Equal(1f, s);
        }

        [UnitTest]
        public void const_double()
        {
            var strField = typeof(ConstValues).GetField("y2");
            var s = (double)strField.GetValue(null);
            Assert.Equal(2f, s);
        }

        [UnitTest]
        public void property()
        {
            var properties = typeof(FT_Property).GetProperties();
            Assert.Equal(1, properties.Length);
            {
                var value = properties[0];
                Assert.Equal("Value", value.Name);
                MethodInfo getter = value.GetGetMethod();
                Assert.Equal("get_Value", getter.Name);
                MethodInfo setter = value.GetSetMethod();
                Assert.Equal("set_Value", setter.Name);
            }
        }

        [UnitTest]
        public void GetExecutingAssembly()
        {
            Assembly ass = Assembly.GetExecutingAssembly();
            Assert.Equal(typeof(TC_Reflection).Assembly, ass);
        }

        [UnitTest]
        public void GetMethodBaseCurrentMethod()
        {
            MethodBase method = MethodBase.GetCurrentMethod();
            Assert.Equal(nameof(GetMethodBaseCurrentMethod), method.Name);
            Assert.Equal(GetType(), method.DeclaringType);
        }
    }
}
