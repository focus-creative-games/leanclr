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
    class TC_ldarg_ref : GeneralTestCaseBase
    {
        private static void s_bool_1(ref bool a)
        {
            Assert.Equal(true, a);
            a = false;
        }

        [UnitTest]
        public void bool_1()
        {
            bool x = true;
            s_bool_1(ref x);
            Assert.False(x);
        }

        private static void s_bool_2(ref bool a)
        {
            Assert.Equal(false, a);
            a = true;
        }

        [UnitTest]
        public void bool_2()
        {
            bool x = false;
            s_bool_2(ref x);
            Assert.True(x);
        }

        private static void s_byte_1(ref byte a)
        {
            Assert.Equal(1, a);
            a = 2;
        }

        [UnitTest]
        public void byte_1()
        {
            byte x = 1;
            s_byte_1(ref x);
            Assert.Equal(2, x);
        }

        private static void s_sbyte_1(ref sbyte a)
        {
            Assert.Equal(1, a);
            a = -1;
        }

        [UnitTest]
        public void sbyte_1()
        {
            sbyte x = 1;
            s_sbyte_1(ref x);
            Assert.Equal(-1, x);
        }

        private static void s_sbyte_2(ref sbyte a)
        {
            Assert.Equal(-1, a);
            a = 1;
        }

        [UnitTest]
        public void sbyte_2()
        {
            sbyte x = -1;
            s_sbyte_2(ref x);
            Assert.Equal(1, x);
        }

        private static void s_short_1(ref short a)
        {
            Assert.Equal(1, a);
            a = -1;
        }

        [UnitTest]
        public void short_1()
        {
            short x = 1;
            s_short_1(ref x);
            Assert.Equal(-1, x);
        }

        private static void s_short_2(ref short a)
        {
            Assert.Equal(-1, a);
            a = 1;
        }

        [UnitTest]
        public void short_2()
        {
            short x = -1;
            s_short_2(ref x);
            Assert.Equal(1, x);
        }

        private static void s_char_1(ref char a)
        {
            Assert.Equal('a', a);
            a = 'b';
        }

        [UnitTest]
        public void char_1()
        {
            char x = 'a';
            s_char_1(ref x);
            Assert.Equal('b', x);
        }

        private static void s_int_1(ref int a)
        {
            Assert.Equal(1, a);
            a = -1;
        }

        [UnitTest]
        public void int_1()
        {
            int x = 1;
            s_int_1(ref x);
            Assert.Equal(-1, x);
        }

        private static void s_int_2(ref int a)
        {
            Assert.Equal(-1, a);
            a = 1;
        }

        [UnitTest]
        public void int_2()
        {
            int x = -1;
            s_int_2(ref x);
            Assert.Equal(1, x);
        }

        private static void s_uint_1(ref uint a)
        {
            Assert.Equal(1, a);
            a = 2;
        }

        [UnitTest]
        public void uint_1()
        {
            uint x = 1;
            s_uint_1(ref x);
            Assert.Equal(2, x);
        }


        private static void s_long_1(ref long a)
        {
            Assert.Equal(1, a);
            a = -1;
        }

        [UnitTest]
        public void long_1()
        {
            long x = 1;
            s_long_1(ref x);
            Assert.Equal(-1, x);
        }

        private static void s_long_2(ref long a)
        {
            Assert.Equal(-1, a);
            a = 1;
        }

        [UnitTest]
        public void long_2()
        {
            long x = -1;
            s_long_2(ref x);
            Assert.Equal(1, x);
        }

        private static void s_ulong_1(ref ulong a)
        {
            Assert.Equal(1, a);
            a = 2;
        }

        [UnitTest]
        public void ulong_1()
        {
            ulong x = 1;
            s_ulong_1(ref x);
            Assert.Equal(2, x);
        }

        private static void s_float_1(ref float a)
        {
            Assert.Equal(1f, a);
            a = -1f;
        }

        [UnitTest]
        public void float_1()
        {
            float x = 1f;
            s_float_1(ref x);
            Assert.Equal(-1f, x);
        }

        private static void s_double_1(ref double a)
        {
            Assert.Equal(1.0, a);
            a = -1.0;
        }

        [UnitTest]
        public void double_1()
        {
            double x = 1.0;
            s_double_1(ref x);
            Assert.Equal(-1.0, x);
        }


        private static void s_valuetype_1(ref ValueTypeSize1 a)
        {
            Assert.Equal(1, a.x1);
            a.x1 = 11;
        }

        [UnitTest]
        public void valuetype_1()
        {
            var x = new ValueTypeSize1 { x1 = 1 };
            s_valuetype_1(ref x);
            Assert.Equal(11, x.x1);
        }

        private static void s_valuetype_2(ref ValueTypeSize2 a)
        {
            Assert.Equal(1, a.x1);
            Assert.Equal(2, a.x2);
            a.x1 = 11;
        }

        [UnitTest]
        public void valuetype_2()
        {
            var x = new ValueTypeSize2 { x1 = 1, x2 = 2 };
            s_valuetype_2(ref x);
            Assert.Equal(11, x.x1);
        }


        private static void s_valuetype_3(ref ValueTypeSize3 a)
        {
            Assert.Equal(1, a.x1);
            Assert.Equal(2, a.x2);
            Assert.Equal(3, a.x3);
            a.x1 = 11;
        }

        [UnitTest]
        public void valuetype_3()
        {
            var x = new ValueTypeSize3 { x1 = 1, x2 = 2, x3 = 3 };
            s_valuetype_3(ref x);
            Assert.Equal(11, x.x1);
        }


        private static void s_valuetype_4(ref ValueTypeSize4 a)
        {
            Assert.Equal(1, a.x1);
            a.x1 = 11;
        }

        [UnitTest]
        public void valuetype_4()
        {
            var x = new ValueTypeSize4 { x1 = 1 };
            s_valuetype_4(ref x);
            Assert.Equal(11, x.x1);
        }

        private static void s_valuetype_5(ref ValueTypeSize5 a)
        {
            Assert.Equal(1, a.x1);
            Assert.Equal(2, a.x2);
            Assert.Equal(3, a.x3);
            Assert.Equal(4, a.x4);
            Assert.Equal(5, a.x5);
            a.x1 = 11;
        }

        [UnitTest]
        public void valuetype_5()
        {
            var x = new ValueTypeSize5 { x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5 };
            s_valuetype_5(ref x);
            Assert.Equal(11, x.x1);
        }

        private static void s_valuetype_8(ref ValueTypeSize8 a)
        {
            Assert.Equal(1, a.x1);
            a.x1 = 11;
        }

        [UnitTest]
        public void valuetype_8()
        {
            var x = new ValueTypeSize8 { x1 = 1, };
            s_valuetype_8(ref x);
            Assert.Equal(11, x.x1);
        }

        private static void s_valuetype_9(ref ValueTypeSize9 a)
        {
            Assert.Equal(1, a.x1);
            Assert.Equal(2, a.x2);
            Assert.Equal(3, a.x3);
            Assert.Equal(4, a.x4);
            Assert.Equal(5, a.x5);
            Assert.Equal(6, a.x6);
            Assert.Equal(7, a.x7);
            Assert.Equal(8, a.x8);
            Assert.Equal(9, a.x9);
            a.x1 = 11;
        }

        [UnitTest]
        public void valuetype_9()
        {
            var x = new ValueTypeSize9 { x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5, x6 = 6, x7 = 7, x8 = 8, x9 = 9 };
            s_valuetype_9(ref x);
            Assert.Equal(11, x.x1);
        }

        private static void s_valuetype_16(ref ValueTypeSize16 a)
        {
            Assert.Equal(1, a.x1);
            Assert.Equal(2, a.x2);
            a.x1 = 11;
        }

        [UnitTest]
        public void valuetype_16()
        {
            var x = new ValueTypeSize16 { x1 = 1, x2 = 2 };
            s_valuetype_16(ref x);
            Assert.Equal(11, x.x1);
        }

        private static void s_valuetype_20(ref ValueTypeSize20 a)
        {
            Assert.Equal(1, a.x1);
            Assert.Equal(2, a.x2);
            Assert.Equal(3, a.x3);
            Assert.Equal(4, a.x4);
            Assert.Equal(5, a.x5);
            a.x1 = 11;
        }

        [UnitTest]
        public void valuetype_20()
        {
            var x = new ValueTypeSize20 { x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5 };
            s_valuetype_20(ref x);
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

        private static void s_multi_args2(ref ValueTypeSize1 a1, ref ValueTypeSize2 a2, ref ValueTypeSize3 a3, ref ValueTypeSize4 a4, ref ValueTypeSize5 a5,
            ref ValueTypeSize8 a8, ref ValueTypeSize9 a9, ref ValueTypeSize16 b1, ref ValueTypeSize20 b2)
        {
            Assert.Equal(1, a1.x1);
            Assert.Equal(2, a2.x1);
            Assert.Equal(3, a3.x1);
            Assert.Equal(4, a4.x1);
            Assert.Equal(5, a5.x1);
            Assert.Equal(8, a8.x1);
            Assert.Equal(9, a9.x1);
            Assert.Equal(16, b1.x1);
            Assert.Equal(20, b2.x1);
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
                ref a1,
                ref a2,
                ref a3,
                ref a4,
                ref a5,
                ref a8,
                ref a9,
                ref b1,
                ref b2
                );
        }
    }
}
