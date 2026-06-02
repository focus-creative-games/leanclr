using System;
using test;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Ariths
{
    class TC_mul : GeneralTestCaseBase
    {
        [UnitTest]
        public static void byte_1()
        {
            byte a = 2;
            byte b = 3;
            Assert.Equal(6, a * b);
        }
        [UnitTest]
        public static void sbyte_1()
        {
            sbyte a = -2;
            sbyte b = 3;
            Assert.Equal(-6, a * b);
        }

        [UnitTest]
        public static void short_1()
        {
            short a = 2;
            short b = -3;
            Assert.Equal(-6, a * b);
        }

        [UnitTest]
        public static void ushort_1()
        {
            ushort a = 2;
            ushort b = 3;
            Assert.Equal(6, a * b);
        }

        [UnitTest]
        public static void int_1()
        {
            int a = -2;
            int b = 3;
            Assert.Equal(-6, a * b);
        }

        [UnitTest]
        public static void uint_1()
        {
            uint a = 2;
            uint b = 3;
            Assert.Equal(6, a * b);
        }

        [UnitTest]
        public static void long_1()
        {
            long a = -2;
            long b = 3;
            Assert.Equal(-6, a * b);
        }

        [UnitTest]
        public static void ulong_1()
        {
            ulong a = 2;
            ulong b = 3;
            Assert.Equal(6, a * b);
        }

        [UnitTest]
        public static void nint_1()
        {
            IntPtr a = new IntPtr(-2);
            IntPtr b = new IntPtr(3);
            Assert.Equal(-6, a.ToInt64() * b.ToInt64());
        }

        [UnitTest]
        public static void float_1()
        {
            float a = 2f;
            float b = 3f;
            Assert.Equal(6f, a * b);
        }

        [UnitTest]
        public static void float_NaN()
        {
            float a = float.PositiveInfinity;
            float b = 0f;
            Assert.True(float.IsNaN(a * b));
        }

        [UnitTest]
        public static void float_NaN_1()
        {
            float a = float.NegativeInfinity;
            float b = 0f;
            Assert.True(float.IsNaN(a * b));
        }

        [UnitTest]
        public static void double_1()
        {
            double a = 2.0;
            double b = 3.0;
            Assert.Equal(6.0, a * b);
        }

        [UnitTest]
        public static void double_NaN()
        {
            double a = double.PositiveInfinity;
            double b = 0d;
            Assert.True(double.IsNaN(a * b));
        }

        [UnitTest]
        public static void double_NaN_1()
        {
            double a = double.PositiveInfinity;
            double b = 0d;
            Assert.True(double.IsNaN(a * b));
        }
    }
}
