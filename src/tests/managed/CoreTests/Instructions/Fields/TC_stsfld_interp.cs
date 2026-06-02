
using test;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;
using AOTDefs;

namespace Tests.Instruments.Fields
{
    internal class TC_stsfld_interp : GeneralTestCaseBase
    {
        [UnitTest]
        public void byte_1()
        {
            InterpTypeStaticFields2.x1 = 11;
            Assert.Equal(11, InterpTypeStaticFields2.x1);
            Assert.Equal(2, InterpTypeStaticFields2.x2);
            InterpTypeStaticFields2.x1 = 1;
        }

        [UnitTest]
        public void sbyte_1()
        {
            InterpTypeStaticFields2.x2 = 11;
            Assert.Equal(11, InterpTypeStaticFields2.x2);
            Assert.Equal(1, InterpTypeStaticFields2.x1);
            Assert.True(InterpTypeStaticFields2.x3);
            InterpTypeStaticFields2.x2 = 2;
        }

        [UnitTest]
        public void bool_1()
        {
            InterpTypeStaticFields2.x3 = false;
            Assert.False(InterpTypeStaticFields2.x3);
            Assert.Equal(2, InterpTypeStaticFields2.x2);
            Assert.Equal(4, InterpTypeStaticFields2.x4);
            InterpTypeStaticFields2.x3 = true;
        }

        [UnitTest]
        public void short_1()
        {
            InterpTypeStaticFields2.x5 = 15;
            Assert.Equal(15, InterpTypeStaticFields2.x5);
            Assert.Equal(4, InterpTypeStaticFields2.x4);
            Assert.Equal(6, InterpTypeStaticFields2.x6);
            InterpTypeStaticFields2.x5 = 5;
        }

        [UnitTest]
        public void ushort_1()
        {
            InterpTypeStaticFields2.x6 = 16;
            Assert.Equal(16, InterpTypeStaticFields2.x6);
            Assert.Equal(5, InterpTypeStaticFields2.x5);
            Assert.Equal(7, InterpTypeStaticFields2.x7);
            InterpTypeStaticFields2.x6 = 6;
        }

        [UnitTest]
        public void int_1()
        {
            InterpTypeStaticFields2.x7 = 17;
            Assert.Equal(17, InterpTypeStaticFields2.x7);
            Assert.Equal(6, InterpTypeStaticFields2.x6);
            Assert.Equal(8, InterpTypeStaticFields2.x8);
            InterpTypeStaticFields2.x7 = 7;
        }

        [UnitTest]
        public void uint_1()
        {
            InterpTypeStaticFields2.x8 = 18;
            Assert.Equal(18, InterpTypeStaticFields2.x8);
            Assert.Equal(7, InterpTypeStaticFields2.x7);
            Assert.Equal(9, InterpTypeStaticFields2.x9);
            InterpTypeStaticFields2.x8 = 8;
        }

        [UnitTest]
        public void long_1()
        {
            InterpTypeStaticFields2.x9 = 19;
            Assert.Equal(19, InterpTypeStaticFields2.x9);
            Assert.Equal(8, InterpTypeStaticFields2.x8);
            Assert.Equal(10, InterpTypeStaticFields2.x10);
            InterpTypeStaticFields2.x9 = 9;
        }

        [UnitTest]
        public void ulong_1()
        {
            InterpTypeStaticFields2.x10 = 20;
            Assert.Equal(20, InterpTypeStaticFields2.x10);
            Assert.Equal(9, InterpTypeStaticFields2.x9);
            Assert.Equal(1f, InterpTypeStaticFields2.y1);
            InterpTypeStaticFields2.x10 = 10;
        }

        [UnitTest]
        public void float_1()
        {
            InterpTypeStaticFields2.y1 = 11f;
            Assert.Equal(11f, InterpTypeStaticFields2.y1);
            Assert.Equal(10, InterpTypeStaticFields2.x10);
            Assert.Equal(2f, InterpTypeStaticFields2.y2);
            InterpTypeStaticFields2.y1 = 1f;
        }

        [UnitTest]
        public void double_1()
        {
            InterpTypeStaticFields2.y3 = 13;
            Assert.Equal(13, InterpTypeStaticFields2.y3);
            Assert.Equal(2f, InterpTypeStaticFields2.y2);
            Assert.Equal("a", InterpTypeStaticFields2.y4);
            InterpTypeStaticFields2.y3 = 3f;
        }

        [UnitTest]
        public void str_1()
        {
            InterpTypeStaticFields2.y4 = "b";
            Assert.Equal("b", InterpTypeStaticFields2.y4);
            Assert.Equal(3.0, InterpTypeStaticFields2.y3);
            InterpTypeStaticFields2.y4 = "a";
        }

        [UnitTest]
        public void enum_byte_1()
        {
            InterpTypeStaticFields2.e1 = default;
            Assert.Equal(default(AOT_Enum_byte), InterpTypeStaticFields2.e1);
            Assert.Equal(AOT_Enum_sbyte.B, InterpTypeStaticFields2.e2);
            InterpTypeStaticFields2.e1 = AOT_Enum_byte.A;
        }

        [UnitTest]
        public void enum_sbyte_1()
        {
            InterpTypeStaticFields2.e2 = AOT_Enum_sbyte.A;
            Assert.Equal(AOT_Enum_sbyte.A, InterpTypeStaticFields2.e2);
            Assert.Equal(AOT_Enum_byte.A, InterpTypeStaticFields2.e1);
            Assert.Equal(AOT_Enum_short.A, InterpTypeStaticFields2.e3);
            InterpTypeStaticFields2.e2 = AOT_Enum_sbyte.B;
        }

        [UnitTest]
        public void enum_short_1()
        {
            InterpTypeStaticFields2.e3 = AOT_Enum_short.B;
            Assert.Equal(AOT_Enum_short.B, InterpTypeStaticFields2.e3);
            Assert.Equal(AOT_Enum_sbyte.B, InterpTypeStaticFields2.e2);
            Assert.Equal(AOT_Enum_ushort.A, InterpTypeStaticFields2.e4);
            InterpTypeStaticFields2.e3 = AOT_Enum_short.A;
        }

        [UnitTest]
        public void enum_ushort_1()
        {
            InterpTypeStaticFields2.e4 = default;
            Assert.Equal(default(AOT_Enum_ushort), InterpTypeStaticFields2.e4);
            Assert.Equal(AOT_Enum_short.A, InterpTypeStaticFields2.e3);
            Assert.Equal(AOT_Enum_int.B, InterpTypeStaticFields2.e5);
            InterpTypeStaticFields2.e4 = AOT_Enum_ushort.A;
        }

        [UnitTest]
        public void enum_int_1()
        {
            InterpTypeStaticFields2.e5 = AOT_Enum_int.A;
            Assert.Equal(AOT_Enum_int.A, InterpTypeStaticFields2.e5);
            Assert.Equal(AOT_Enum_ushort.A, InterpTypeStaticFields2.e4);
            Assert.Equal(AOT_Enum_uint.A, InterpTypeStaticFields2.e6);
            InterpTypeStaticFields2.e5 = AOT_Enum_int.B;
        }

        [UnitTest]
        public void enum_uint_1()
        {
            InterpTypeStaticFields2.e6 = default;
            Assert.Equal(default(AOT_Enum_uint), InterpTypeStaticFields2.e6);
            Assert.Equal(AOT_Enum_int.B, InterpTypeStaticFields2.e5);
            Assert.Equal(AOT_Enum_long.B, InterpTypeStaticFields2.e7);
            InterpTypeStaticFields2.e6 = AOT_Enum_uint.A;
        }

        [UnitTest]
        public void enum_long_1()
        {
            InterpTypeStaticFields2.e7 = AOT_Enum_long.A;
            Assert.Equal(AOT_Enum_long.A, InterpTypeStaticFields2.e7);
            Assert.Equal(AOT_Enum_uint.A, InterpTypeStaticFields2.e6);
            Assert.Equal(AOT_Enum_ulong.A, InterpTypeStaticFields2.e8);
            InterpTypeStaticFields2.e7 = AOT_Enum_long.B;
        }

        [UnitTest]
        public void enum_ulong_1()
        {
            InterpTypeStaticFields2.e8 = default;
            Assert.Equal(default(AOT_Enum_ulong), InterpTypeStaticFields2.e8);
            InterpTypeStaticFields2.e8 = AOT_Enum_ulong.A;
        }

        [UnitTest]
        public void valuetypesize_1()
        {
            InterpTypeStaticFields2.s1 = new ValueTypeSize1 { x1 = 10 };
            Assert.Equal(10, InterpTypeStaticFields2.s1.x1);
            Assert.Equal("a", InterpTypeStaticFields2.y4);
            Assert.Equal(2, InterpTypeStaticFields2.s2.x1);
            InterpTypeStaticFields2.s1 = new ValueTypeSize1 { x1 = 1 };
        }

        [UnitTest]
        public void valuetypesize_2()
        {
            InterpTypeStaticFields2.s2 = new ValueTypeSize2 { x1 = 10 };
            Assert.Equal(10, InterpTypeStaticFields2.s2.x1);
            Assert.Equal(1, InterpTypeStaticFields2.s1.x1);
            Assert.Equal(3, InterpTypeStaticFields2.s3.x1);
            InterpTypeStaticFields2.s2 = new ValueTypeSize2 { x1 = 2 };
        }

        [UnitTest]
        public void valuetypesize_3()
        {
            InterpTypeStaticFields2.s3 = new ValueTypeSize3 { x1 = 10 };
            Assert.Equal(10, InterpTypeStaticFields2.s3.x1);
            Assert.Equal(2, InterpTypeStaticFields2.s2.x1);
            Assert.Equal(4, InterpTypeStaticFields2.s4.x1);
            InterpTypeStaticFields2.s3 = new ValueTypeSize3 { x1 = 3 };
        }

        [UnitTest]
        public void valuetypesize_4()
        {
            InterpTypeStaticFields2.s4 = new ValueTypeSize4 { x1 = 10 };
            Assert.Equal(10, InterpTypeStaticFields2.s4.x1);
            Assert.Equal(3, InterpTypeStaticFields2.s3.x1);
            Assert.Equal(5, InterpTypeStaticFields2.s5.x1);
            InterpTypeStaticFields2.s4 = new ValueTypeSize4 { x1 = 4 };
        }

        [UnitTest]
        public void valuetypesize_5()
        {
            InterpTypeStaticFields2.s5 = new ValueTypeSize5 { x1 = 10 };
            Assert.Equal(10, InterpTypeStaticFields2.s5.x1);
            Assert.Equal(4, InterpTypeStaticFields2.s4.x1);
            Assert.Equal(6, InterpTypeStaticFields2.s8.x1);
            InterpTypeStaticFields2.s5 = new ValueTypeSize5 { x1 = 5 };
        }

        [UnitTest]
        public void valuetypesize_8()
        {
            InterpTypeStaticFields2.s8 = new ValueTypeSize8 { x1 = 10 };
            Assert.Equal(10, InterpTypeStaticFields2.s8.x1);
            Assert.Equal(5, InterpTypeStaticFields2.s5.x1);
            Assert.Equal(7, InterpTypeStaticFields2.s9.x1);
            InterpTypeStaticFields2.s8 = new ValueTypeSize8 { x1 = 6 };
        }

        [UnitTest]
        public void valuetypesize_9()
        {
            InterpTypeStaticFields2.s9 = new ValueTypeSize9 { x1 = 10 };
            Assert.Equal(10, InterpTypeStaticFields2.s9.x1);
            Assert.Equal(6, InterpTypeStaticFields2.s8.x1);
            Assert.Equal(8, InterpTypeStaticFields2.s16.x1);
            InterpTypeStaticFields2.s9 = new ValueTypeSize9 { x1 = 7 };
        }

        [UnitTest]
        public void valuetypesize_16()
        {
            InterpTypeStaticFields2.s16 = new ValueTypeSize16 { x1 = 10 };
            Assert.Equal(10, InterpTypeStaticFields2.s16.x1);
            Assert.Equal(7, InterpTypeStaticFields2.s9.x1);
            Assert.Equal(AOT_Enum_byte.A, InterpTypeStaticFields2.e1);
            InterpTypeStaticFields2.s16 = new ValueTypeSize16 { x1 = 8 };
        }
    }
}
