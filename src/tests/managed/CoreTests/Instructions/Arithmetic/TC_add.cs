using test;

namespace Tests.Instruments.Ariths
{
    public class TC_add : GeneralTestCaseBase
    {

        [UnitTest]
        public void byte_vv_1()
        {
            byte a = 0x10;
            byte b = 0x01;
            Assert.Equal(0x11, a + b);
        }

        [UnitTest]
        public void byte_vc_1()
        {
            byte a = 0x10;
            Assert.Equal(0x11, a + 0x01);
        }

        [UnitTest]
        public void byte_cv_1()
        {
            byte a = 0x10;
            Assert.Equal(0x11, 0x01 + a);
        }

        [UnitTest]
        public void sbyte_vv_1()
        {
            sbyte a = 0x10;
            sbyte b = -0x01;
            Assert.Equal(0x0F, a + b);
        }

        [UnitTest]
        public void sbyte_vc_1()
        {
            sbyte a = 0x10;
            Assert.Equal(0x0F, a + -0x01);
        }

        [UnitTest]
        public void sbyte_cv_1()
        {
            sbyte a = 0x10;
            Assert.Equal(0x0F, -0x01 + a);
        }

        [UnitTest]
        public void short_vv_1()
        {
            short a = 0x1010;
            short b = -0x0101;
            Assert.Equal(0x0F0F, a + b);
        }

        [UnitTest]
        public void short_vc_1()
        {
            short a = 0x1010;
            Assert.Equal(0x0F0F, a + -0x0101);
        }

        [UnitTest]
        public void short_cv_1()
        {
            short b = -0x0101;
            Assert.Equal(0x0F0F, 0x1010 + b);
        }

        [UnitTest]
        public void ushort_1()
        {
            ushort a = 0x1010;
            ushort b = 0x0101;
            Assert.Equal(0x1111, a + b);
        }

        [UnitTest]
        public void int_vv_1()
        {
            int a = 0x10101010;
            int b = -0x01010101;
            Assert.Equal(0x0F0F0F0F, a + b);
        }

        [UnitTest]
        public void int_vc_1()
        {
            int a = 0x10101010;
            Assert.Equal(0x0F0F0F0F, a + -0x01010101);
        }

        [UnitTest]
        public void int_cv_1()
        {
            int a = 0x10101010;
            Assert.Equal(0x0F0F0F0F, -0x01010101 + a);
        }

        /// <summary>
        /// overflow. not exception
        /// </summary>
        [UnitTest]
        public void int_vv_OverflowNotException()
        {
            int a = 0x7FFFFFFF;
            int b = 4;
            Assert.Equal(-0x7FFFFFFD, a + b);
        }

        [UnitTest]
        public void uint_vv_1()
        {
            uint a = 0x10203040u;
            uint b = 0x01020304u;
            Assert.Equal(0x11223344u, a + b);
        }

        /// <summary>
        /// overflow. not exception
        /// </summary>
        [UnitTest]
        public void uint_vv_OverflowNotException()
        {
            uint a = 0xFFFFFFFF;
            uint b = 1;
            Assert.Equal(0x0u, a + b);
        }

        [UnitTest]
        public void long_vv_1()
        {
            long a = 0x1020304050607080;
            long b = 0x0102030405060708;
            Assert.Equal(0x1122334455667788L, a + b);
        }

        [UnitTest]
        public void long_vc_1()
        {
            long a = 0x1020304050607080;
            Assert.Equal(0x1122334455667788L, a + 0x0102030405060708);
        }

        [UnitTest]
        public void long_cv_1()
        {
            long b = 0x0102030405060708;
            Assert.Equal(0x1122334455667788L, 0x1020304050607080 + b);
        }

        /// <summary>
        /// overflow not excpetion
        /// </summary>
        [UnitTest]
        public void long_vv_OverflowNotException()
        {
            long a = 0x7FFFFFFF_FFFFFFFF;
            long b = 1;
            Assert.Equal(-0x80000000_00000000L, a + b);
        }

        [UnitTest]
        public void ulong_vv_1()
        {
            ulong a = 0x1020304050607080;
            ulong b = 0x0102030405060708;
            Assert.Equal(0x1122334455667788UL, a + b);
        }

        /// <summary>
        /// overflow not excpetion
        /// </summary>
        [UnitTest]
        public void ulong_vv_OverflowNotException()
        {
            ulong a = 0xFFFFFFFF_FFFFFFFFUL;
            ulong b = 1UL;
            Assert.Equal(0UL, a + b);
        }

        [UnitTest]
        public void nint_int_vv_1()
        {
            System.IntPtr a = new System.IntPtr(0x10101010);
            int b = - 0x01010101;
            Assert.Equal(0x0F0F0F0F, (a + b).ToInt32());
        }

        [UnitTest]
        public void float_vv_1()
        {
            float a = 3f;
            float b = 4f;
            Assert.Equal(7f, a + b);
        }

        [UnitTest]
        public void float_vc_1()
        {
            float a = 3f;
            Assert.Equal(7f, a + 4f);
        }

        [UnitTest]
        public void float_cv_1()
        {
            float b = 4f;
            Assert.Equal(7f, 3f + b);
        }

        [UnitTest]
        public void float_vv_inf()
        {
            float a = float.PositiveInfinity;
            float b = 1;
            Assert.True(float.IsPositiveInfinity(a + b));
        }

        [UnitTest]
        public void float_vv_minf()
        {
            float a = float.NegativeInfinity;
            float b = 1;
            Assert.True(float.IsNegativeInfinity(a + b));
        }

        [UnitTest]
        public void float_vv_NaN()
        {
            float a = float.NaN;
            float b = 1;
            Assert.True(float.IsNaN(a + b));
        }

        [UnitTest]
        public void double_vv_1()
        {
            double a = 3.0;
            double b = 4.0;
            Assert.Equal(7.0, a + b);
        }

        [UnitTest]
        public void double_vc_1()
        {
            double a = 3.0;
            Assert.Equal(7.0, a + 4.0);
        }

        [UnitTest]
        public void double_cv_1()
        {
            double b = 4.0;
            Assert.Equal(7.0, 3.0 + b);
        }

        [UnitTest]
        public void double_inf()
        {
            double a = double.PositiveInfinity;
            double b = 1;
            Assert.True(double.IsInfinity(a + b));
        }

        [UnitTest]
        public void double_minf()
        {
            double a = double.NegativeInfinity;
            double b = 1;
            Assert.Equal(double.NegativeInfinity, a + b);
        }

        [UnitTest]
        public void double_NaN()
        {
            double a = double.NaN;
            double b = 1;
            Assert.Equal(double.NaN, a + b);
        }
    }
}
