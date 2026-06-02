using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Ariths
{
    internal class TC_sub : GeneralTestCaseBase
    {
        [UnitTest]
        public static void byte_vv_1()
        {
            byte a = 1;
            byte b = 2;
            Assert.Equal(-1, a - b);
        }

        [UnitTest]
        public static void sbyte_vv_1()
        {
            byte a = 1;
            byte b = 2;
            Assert.Equal(-1, a - b);
        }

        [UnitTest]
        public static void short_vv_1()
        {
            short a = 1;
            short b = 2;
            Assert.Equal(-1, a - b);
        }

        [UnitTest]
        public static void ushort_vv_1()
        {
            ushort a = 1;
            ushort b = 2;
            Assert.Equal(-1, a - b);
        }

        [UnitTest]
        public static void int_vv_1()
        {
            int a = 1;
            int b = 2;
            Assert.Equal(-1, a - b);
        }

        [UnitTest]
        public static void int_vc_1()
        {
            int a = 1;
            Assert.Equal(-1, a - 2);
        }

        [UnitTest]
        public static void int_cv_1()
        {
            int b = 2;
            Assert.Equal(-1, 1 - b);
        }

        [UnitTest]
        public static void long_vv_1()
        {
            long a = 1;
            long b = 2;
            Assert.Equal(-1L, a - b);
        }

        [UnitTest]
        public static void long_vc_1()
        {
            long a = 1;
            Assert.Equal(-1L, a - 2);
        }

        [UnitTest]
        public static void long_cv_1()
        {
            long b = 2;
            Assert.Equal(-1L, 1 - b);
        }

        [UnitTest]
        public static void nint_int_vv_1()
        {
            IntPtr a = new IntPtr(1);
            int b = 2;
            Assert.Equal(new IntPtr(-1), a - b);
        }

        [UnitTest]
        public static void Nuint_int_vv_1()
        {
            UIntPtr a = new UIntPtr(3);
            int b = 2;
            Assert.Equal(new UIntPtr(1), a - b);
        }

        [UnitTest]
        public static void float_vv_1()
        {
            float a = 1f;
            float b = 2f;
            Assert.Equal(-1f, a - b);
        }

        [UnitTest]
        public static void float_vv_inf()
        {
            float a = 2f;
            float b = float.PositiveInfinity;
            Assert.True(float.IsPositiveInfinity(b - a));
        }

        [UnitTest]
        public static void float_vv_minf()
        {
            float a = 2f;
            float b = float.NegativeInfinity;
            Assert.True(float.IsNegativeInfinity(b - a));
        }

        [UnitTest]
        public static void float_vv_NaN()
        {
            float a = 2f;
            float b = float.NaN;
            Assert.True(float.IsNaN(a - b));
        }

        [UnitTest]
        public static void double_vv_1()
        {
            double a = 1.0;
            double b = 2.0;
            Assert.Equal(-1d, a - b);
        }

        [UnitTest]
        public static void double_inf()
        {
            double a = 2d;
            double b = double.PositiveInfinity;
            Assert.Equal(double.PositiveInfinity, b-a);
        }

        [UnitTest]
        public static void double_minf()
        {
            double a = 2;
            double b = double.NegativeInfinity;
            Assert.Equal(double.NegativeInfinity, b-a);
        }

        [UnitTest]
        public static void double_NaN()
        {
            double a = 2;
            double b = double.NaN;
            Assert.Equal(double.NaN, a-b);
        }
    }
}

