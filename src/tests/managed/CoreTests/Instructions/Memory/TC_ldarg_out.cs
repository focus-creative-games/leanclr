using System;
using test;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;
using AOTDefs;


namespace Tests.Instruments.Mems
{
    class TC_ldarg_out : GeneralTestCaseBase
    {
        private static void s_bool_1(out bool a)
        {
            a = false;
        }

        [UnitTest]
        public void bool_1()
        {
            bool x = true;
            s_bool_1(out x);
            Assert.False(x);
        }

        private static void s_bool_2(out bool a)
        {
            a = true;
        }

        [UnitTest]
        public void bool_2()
        {
            bool x = false;
            s_bool_2(out x);
            Assert.True(x);
        }

        private static void s_byte_1(out byte a)
        {
            a = 2;
        }

        [UnitTest]
        public void byte_1()
        {
            byte x = 1;
            s_byte_1(out x);
            Assert.Equal(2, x);
        }

        private static void s_sbyte_1(out sbyte a)
        {
            a = -1;
        }

        [UnitTest]
        public void sbyte_1()
        {
            sbyte x = 1;
            s_sbyte_1(out x);
            Assert.Equal(-1, x);
        }

        private static void s_sbyte_2(out sbyte a)
        {
            a = 1;
        }

        [UnitTest]
        public void sbyte_2()
        {
            sbyte x = -1;
            s_sbyte_2(out x);
            Assert.Equal(1, x);
        }

        private static void s_short_1(out short a)
        {
            a = -1;
        }

        [UnitTest]
        public void short_1()
        {
            short x = 1;
            s_short_1(out x);
            Assert.Equal(-1, x);
        }

        private static void s_short_2(out short a)
        {
            a = 1;
        }

        [UnitTest]
        public void short_2()
        {
            short x = -1;
            s_short_2(out x);
            Assert.Equal(1, x);
        }

        private static void s_char_1(out char a)
        {
            a = 'b';
        }

        [UnitTest]
        public void char_1()
        {
            char x = 'a';
            s_char_1(out x);
            Assert.Equal('b', x);
        }

        private static void s_int_1(out int a)
        {
            a = -1;
        }

        [UnitTest]
        public void int_1()
        {
            int x = 1;
            s_int_1(out x);
            Assert.Equal(-1, x);
        }

        private static void s_int_2(out int a)
        {
            a = 1;
        }

        [UnitTest]
        public void int_2()
        {
            int x = -1;
            s_int_2(out x);
            Assert.Equal(1, x);
        }

        private static void s_uint_1(out uint a)
        {
            a = 2;
        }

        [UnitTest]
        public void uint_1()
        {
            uint x = 1;
            s_uint_1(out x);
            Assert.Equal(2, x);
        }


        private static void s_long_1(out long a)
        {
            a = -1;
        }

        [UnitTest]
        public void long_1()
        {
            long x = 1;
            s_long_1(out x);
            Assert.Equal(-1, x);
        }

        private static void s_long_2(out long a)
        {
            a = 1;
        }

        [UnitTest]
        public void long_2()
        {
            long x = -1;
            s_long_2(out x);
            Assert.Equal(1, x);
        }

        private static void s_ulong_1(out ulong a)
        {
            a = 2;
        }

        [UnitTest]
        public void ulong_1()
        {
            ulong x = 1;
            s_ulong_1(out x);
            Assert.Equal(2, x);
        }

        private static void s_float_1(out float a)
        {
            a = -1f;
        }

        [UnitTest]
        public void float_1()
        {
            float x = 1f;
            s_float_1(out x);
            Assert.Equal(-1f, x);
        }

        private static void s_double_1(out double a)
        {
            a = -1.0;
        }

        [UnitTest]
        public void double_1()
        {
            double x = 1.0;
            s_double_1(out x);
            Assert.Equal(-1.0, x);
        }


        private static void s_valuetype_1(out ValueTypeSize1 a)
        {
            a.x1 = 11;
        }

        [UnitTest]
        public void valuetype_1()
        {
            var x = new ValueTypeSize1 { x1 = 1 };
            s_valuetype_1(out x);
            Assert.Equal(11, x.x1);
        }

        private static void s_valuetype_2(out ValueTypeSize2 a)
        {
            a.x1 = 11;
            a.x2 = 12;
        }

        [UnitTest]
        public void valuetype_2()
        {
            var x = new ValueTypeSize2 { x1 = 1, x2 = 2 };
            s_valuetype_2(out x);
            Assert.Equal(11, x.x1);
        }


        private static void s_valuetype_3(out ValueTypeSize3 a)
        {
            a.x1 = 11;
            a.x2 = 12;
            a.x3 = 13;
        }

        [UnitTest]
        public void valuetype_3()
        {
            var x = new ValueTypeSize3 { x1 = 1, x2 = 2, x3 = 3 };
            s_valuetype_3(out x);
            Assert.Equal(11, x.x1);
        }


        private static void s_valuetype_4(out ValueTypeSize4 a)
        {
            a.x1 = 11;
        }

        [UnitTest]
        public void valuetype_4()
        {
            var x = new ValueTypeSize4 { x1 = 1 };
            s_valuetype_4(out x);
            Assert.Equal(11, x.x1);
        }

        private static void s_valuetype_5(out ValueTypeSize5 a)
        {
            a.x1 = 11;
            a.x2 = 12;
            a.x3 = 13;
            a.x4 = 14;
            a.x5 = 15;
        }

        [UnitTest]
        public void valuetype_5()
        {
            var x = new ValueTypeSize5 { x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5 };
            s_valuetype_5(out x);
            Assert.Equal(11, x.x1);
        }

        private static void s_valuetype_8(out ValueTypeSize8 a)
        {
            a.x1 = 11;
        }

        [UnitTest]
        public void valuetype_8()
        {
            var x = new ValueTypeSize8 { x1 = 1, };
            s_valuetype_8(out x);
            Assert.Equal(11, x.x1);
        }

        private static void s_valuetype_9(out ValueTypeSize9 a)
        {
            a.x1 = 11;
            a.x2 = 12;
            a.x3 = 13;
            a.x4 = 14;
            a.x5 = 15;
            a.x6 = 16;
            a.x7 = 17;
            a.x8 = 18;
            a.x9 = 19;
        }

        [UnitTest]
        public void valuetype_9()
        {
            var x = new ValueTypeSize9 { x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5, x6 = 6, x7 = 7, x8 = 8, x9 = 9 };
            s_valuetype_9(out x);
            Assert.Equal(11, x.x1);
        }

        private static void s_valuetype_16(out ValueTypeSize16 a)
        {
            a.x1 = 11;
            a.x2 = 12;
        }

        [UnitTest]
        public void valuetype_16()
        {
            var x = new ValueTypeSize16 { x1 = 1, x2 = 2 };
            s_valuetype_16(out x);
            Assert.Equal(11, x.x1);
        }

        private static void s_valuetype_20(out ValueTypeSize20 a)
        {
            a.x1 = 11;
            a.x2 = 12;
            a.x3 = 13;
            a.x4 = 14;
            a.x5 = 15;
        }

        [UnitTest]
        public void valuetype_20()
        {
            var x = new ValueTypeSize20 { x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5 };
            s_valuetype_20(out x);
            Assert.Equal(11, x.x1);
        }


        private static void s_multi_args(bool a1, byte a2, sbyte a3, short a4, ushort a5, char a6, int a7, uint a8, long a9, ulong a10,
            float b1, double b2, object b3)
        {
            Assert.True(a1);
            Assert.Equal(2, a2);
            Assert.Equal(3, a3);
            Assert.Equal(4, a4);
            Assert.Equal(5, a5);
            Assert.Equal('a', a6);
            Assert.Equal(7, a7);
            Assert.Equal(8, a8);
            Assert.Equal(9, a9);
            Assert.Equal(10, a10);
            Assert.Equal(11f, b1);
            Assert.Equal(12.0, b2);
            Assert.Equal("abc", b3);
        }

        [UnitTest]
        public void multi_args()
        {
            s_multi_args(true, 2, 3, 4, 5, 'a', 7, 8, 9, 10, 11f, 12.0, "abc");
        }

        private static void s_multi_args2(out ValueTypeSize1 a1, out ValueTypeSize2 a2, out ValueTypeSize3 a3, out ValueTypeSize4 a4, out ValueTypeSize5 a5,
            out ValueTypeSize8 a8, out ValueTypeSize9 a9, out ValueTypeSize16 b1, out ValueTypeSize20 b2)
        {
            a1 = new ValueTypeSize1 { x1 = 1 };
            a2 = new ValueTypeSize2 { x1 = 2 };
            a3 = new ValueTypeSize3 { x1 = 3 };
            a4 = new ValueTypeSize4 { x1 = 4 };
            a5 = new ValueTypeSize5 { x1 = 5 };
            a8 = new ValueTypeSize8 { x1 = 8 };
            a9 = new ValueTypeSize9 { x1 = 9 };
            b1 = new ValueTypeSize16 { x1 = 16 };
            b2 = new ValueTypeSize20 { x1 = 20 };
        }

        [UnitTest]
        public void multi_args2()
        {
            var a1 = new ValueTypeSize1 { x1 = 1 };
            var a2 = new ValueTypeSize2 { x1 = 2 };
            var a3 = new ValueTypeSize3 { x1 = 3 };
            var a4 = new ValueTypeSize4 { x1 = 4 };
            var a5 = new ValueTypeSize5 { x1 = 5 };
            var a8 = new ValueTypeSize8 { x1 = 8 };
            var a9 = new ValueTypeSize9 { x1 = 9 };
            var b1 = new ValueTypeSize16 { x1 = 16 };
            var b2 = new ValueTypeSize20 { x1 = 20 };

            s_multi_args2(
                out a1,
                out a2,
                out a3,
                out a4,
                out a5,
                out a8,
                out a9,
                out b1,
                out b2
                );
        }
    }
}
