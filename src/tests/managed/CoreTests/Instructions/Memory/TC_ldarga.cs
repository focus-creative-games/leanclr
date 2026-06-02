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
    class TC_ldarga : GeneralTestCaseBase
    {
        private static void s_bool_1(bool a)
        {
            ref bool r = ref a;
            Assert.True(r);
            r = false;
            Assert.False(a);
        }

        [UnitTest]
        public void bool_1()
        {
            s_bool_1(true);
        }

        private static void s_byte_1(byte a)
        {
            ref byte r = ref a;
            Assert.Equal(1, r);
            r = 2;
            Assert.Equal(2, a);
        }

        [UnitTest]
        public void byte_1()
        {
            s_byte_1(1);
        }

        private static void s_sbyte_1(sbyte a)
        {
            ref sbyte r = ref a;
            Assert.Equal(1, r);
            r = -1;
            Assert.Equal(-1, a);
        }

        [UnitTest]
        public void sbyte_1()
        {
            s_sbyte_1(1);
        }

        private static void s_short_1(short a)
        {
            ref short r = ref a;
            Assert.Equal(1, r);
            r = -1;
            Assert.Equal(-1, a);
        }

        [UnitTest]
        public void short_1()
        {
            s_short_1(1);
        }

        private static void s_char_1(char a)
        {
            ref char r = ref a;
            Assert.Equal('a', r);
            r = 'b';
            Assert.Equal('b', a);
        }

        [UnitTest]
        public void char_1()
        {
            s_char_1('a');
        }

        private static void s_int_1(int a)
        {
            ref int r = ref a;
            Assert.Equal(1, r);
            r = -1;
            Assert.Equal(-1, a);
        }

        [UnitTest]
        public void int_1()
        {
            s_int_1(1);
        }

        private static void s_uint_1(uint a)
        {
            ref uint r = ref a;
            Assert.Equal(1, r);
            r = 2;
            Assert.Equal(2, a);
        }

        [UnitTest]
        public void uint_1()
        {
            s_uint_1(1);
        }


        private static void s_long_1(long a)
        {
            ref long r = ref a;
            Assert.Equal(1, r);
            r = -1;
            Assert.Equal(-1, a);
        }

        [UnitTest]
        public void long_1()
        {
            s_long_1(1);
        }

        private static void s_ulong_1(ulong a)
        {
            ref ulong r = ref a;
            Assert.Equal(1, r);
            r = 2;
            Assert.Equal(2, a);
        }

        [UnitTest]
        public void ulong_1()
        {
            s_ulong_1(1);
        }

        private static void s_float_1(float a)
        {
            ref float r = ref a;
            Assert.Equal(1, r);
            r = -1f;
            Assert.Equal(-1f, a);
        }

        [UnitTest]
        public void float_1()
        {
            s_float_1(1f);
        }

        private static void s_double_1(double a)
        {
            ref double r = ref a;
            Assert.Equal(1.0, r);
            r = -1.0;
            Assert.Equal(-1.0, a);
        }

        [UnitTest]
        public void double_1()
        {
            s_double_1(1.0);
        }


        private static void s_valuetype_1(ValueTypeSize1 a)
        {
            ref ValueTypeSize1 r = ref a;
            Assert.Equal(1, r.x1);
            r.x1 = 2;
            Assert.Equal(2, a.x1);
        }

        [UnitTest]
        public void valuetype_1()
        {
            s_valuetype_1(new ValueTypeSize1 { x1 = 1 });
        }

        private static void s_valuetype_2(ValueTypeSize2 a)
        {
            ref ValueTypeSize2 r = ref a;
            Assert.Equal(1, r.x1);
            r.x1 = 2;
            Assert.Equal(2, a.x1);
        }

        [UnitTest]
        public void valuetype_2()
        {
            s_valuetype_2(new ValueTypeSize2 { x1 = 1, x2 = 2 });
        }


        private static void s_valuetype_3(ValueTypeSize3 a)
        {
            ref ValueTypeSize3 r = ref a;
            Assert.Equal(1, r.x1);
            r.x1 = 2;
            Assert.Equal(2, a.x1);
        }

        [UnitTest]
        public void valuetype_3()
        {
            s_valuetype_3(new ValueTypeSize3 { x1 = 1, x2 = 2, x3 = 3 });
        }


        private static void s_valuetype_4(ValueTypeSize4 a)
        {
            ref ValueTypeSize4 r = ref a;
            Assert.Equal(1, r.x1);
            r.x1 = 2;
            Assert.Equal(2, a.x1);
        }

        [UnitTest]
        public void valuetype_4()
        {
            s_valuetype_4(new ValueTypeSize4 { x1 = 1 });
        }

        private static void s_valuetype_5(ValueTypeSize5 a)
        {
            ref ValueTypeSize5 r = ref a;
            Assert.Equal(1, r.x1);
            r.x1 = 2;
            Assert.Equal(2, a.x1);
        }

        [UnitTest]
        public void valuetype_5()
        {
            s_valuetype_5(new ValueTypeSize5 { x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5 });
        }

        private static void s_valuetype_8(ValueTypeSize8 a)
        {
            ref ValueTypeSize8 r = ref a;
            Assert.Equal(1, r.x1);
            r.x1 = 2;
            Assert.Equal(2, a.x1);
        }

        [UnitTest]
        public void valuetype_8()
        {
            s_valuetype_8(new ValueTypeSize8 { x1 = 1, });
        }

        private static void s_valuetype_9(ValueTypeSize9 a)
        {
            ref ValueTypeSize9 r = ref a;
            Assert.Equal(1, r.x1);
            r.x1 = 2;
            Assert.Equal(2, a.x1);
        }

        [UnitTest]
        public void valuetype_9()
        {
            s_valuetype_9(new ValueTypeSize9 { x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5, x6 = 6, x7 = 7, x8 = 8, x9 = 9 });
        }

        private static void s_valuetype_16(ValueTypeSize16 a)
        {
            ref ValueTypeSize16 r = ref a;
            Assert.Equal(1, r.x1);
            r.x1 = 2;
            Assert.Equal(2, a.x1);
        }

        [UnitTest]
        public void valuetype_16()
        {
            s_valuetype_16(new ValueTypeSize16 { x1 = 1, x2 = 2});
        }

        private static void s_valuetype_20(ValueTypeSize20 a)
        {
            ref ValueTypeSize20 r = ref a;
            Assert.Equal(1, r.x1);
            r.x1 = 2;
            Assert.Equal(2, a.x1);
        }

        [UnitTest]
        public void valuetype_20()
        {
            s_valuetype_20(new ValueTypeSize20 { x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5 });
        }

        private static void s_multi_args(bool a1, byte a2, sbyte a3, short a4, ushort a5, char a6, int a7, uint a8, long a9, ulong a10,
            float b1, double b2, object b3)
        {
            ref bool r1 = ref a1;
            Assert.True(r1);
            r1 = false;
            Assert.False(a1);
            
            ref byte r2 = ref a2;
            Assert.Equal(2, r2);
            r2 = 12;
            Assert.Equal(12, a2);

            ref sbyte r3 = ref a3;
            Assert.Equal(3, r3);
            r3 = 13;
            Assert.Equal(13, a3);

            ref short r4 = ref a4;
            Assert.Equal(4, r4);
            r4 = 14;
            Assert.Equal(14, a4);

            ref ushort r5 = ref a5;
            Assert.Equal(5, r5);
            r5 = 15;
            Assert.Equal(15, a5);

            ref char r6 = ref a6;
            Assert.Equal('a', r6);
            r6 = 'b';
            Assert.Equal('b', a6);

            ref int r7 = ref a7;
            Assert.Equal(7, r7);
            r7 = 17;
            Assert.Equal(17, a7);

            ref uint r8 = ref a8;
            Assert.Equal(8, r8);
            r8 = 18;
            Assert.Equal(18, a8);

            ref long r9 = ref a9;
            Assert.Equal(9, r9);
            r9 = 19;
            Assert.Equal(19, a9);

            ref ulong r10 = ref a10;
            Assert.Equal(10, r10);
            r10 = 20;
            Assert.Equal(20, a10);

            ref float s1 = ref b1;
            Assert.Equal(11f, s1);
            s1 = 21f;
            Assert.Equal(21f, b1);

            ref double s2 = ref b2;
            Assert.Equal(12.0, s2);
            s2 = 22.0;
            Assert.Equal(22.0, b2);

            ref object s3 = ref b3;
            Assert.Equal("abc", s3);
            s3 = null;
            Assert.Null(b3);
        }

        [UnitTest]
        public void multi_args()
        {
            s_multi_args(true, 2, 3, 4, 5, 'a', 7, 8, 9, 10, 11f, 12.0, "abc");
        }

        private static void s_multi_args2(ValueTypeSize1 a1, ValueTypeSize2 a2, ValueTypeSize3 a3, ValueTypeSize4 a4, ValueTypeSize5 a5,
            ValueTypeSize8 a8, ValueTypeSize9 a9, ValueTypeSize16 b1, ValueTypeSize20 b2)
        {
            ref ValueTypeSize1 s1 = ref a1;
            ref ValueTypeSize2 s2 = ref a2;
            ref ValueTypeSize3 s3 = ref a3;
            ref ValueTypeSize4 s4 = ref a4;
            ref ValueTypeSize5 s5 = ref a5;
            ref ValueTypeSize8 s8 = ref a8;
            ref ValueTypeSize9 s9 = ref a9;
            ref ValueTypeSize16 t1 = ref b1;
            ref ValueTypeSize20 t2 = ref b2;
            Assert.Equal(1, s1.x1);
            Assert.Equal(2, s2.x1);
            Assert.Equal(3, s3.x1);
            Assert.Equal(4, s4.x1);
            Assert.Equal(5, s5.x1);
            Assert.Equal(8, s8.x1);
            Assert.Equal(9, s9.x1);
            Assert.Equal(16, t1.x1);
            Assert.Equal(20, t2.x1);
        }

        [UnitTest]
        public void multi_args2()
        {
            s_multi_args2(
                new ValueTypeSize1 { x1 = 1},
                new ValueTypeSize2 { x1 = 2},
                new ValueTypeSize3 { x1 = 3},
                new ValueTypeSize4 { x1 = 4},
                new ValueTypeSize5 { x1 = 5},
                new ValueTypeSize8 { x1 = 8},
                new ValueTypeSize9 { x1 = 9},
                new ValueTypeSize16 { x1 = 16},
                new ValueTypeSize20 { x1 = 20}
                );
        }
    }
}
