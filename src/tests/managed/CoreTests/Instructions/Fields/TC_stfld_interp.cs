
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
    internal class TC_stfld_interp : GeneralTestCaseBase
    {
        [UnitTest]
        public void byte_1()
        {
            var t = new InterpTypeFields();
            t.x1 = 11;
            Assert.Equal(11, t.x1);
            Assert.Equal(2, t.x2);
        }

        [UnitTest]
        public void sbyte_1()
        {
            var t = new InterpTypeFields();
            t.x2 = 11;
            Assert.Equal(11, t.x2);
            Assert.Equal(1, t.x1);
            Assert.True(t.x3);
        }

        [UnitTest]
        public void bool_1()
        {
            var t = new InterpTypeFields();
            t.x3 = false;
            Assert.False(t.x3);
            Assert.Equal(2, t.x2);
            Assert.Equal(4, t.x4);
        }

        [UnitTest]
        public void short_1()
        {
            var t = new InterpTypeFields();
            t.x5 = 15;
            Assert.Equal(15, t.x5);
            Assert.Equal(4, t.x4);
            Assert.Equal(6, t.x6);
        }

        [UnitTest]
        public void ushort_1()
        {
            var t = new InterpTypeFields();
            t.x6 = 16;
            Assert.Equal(16, t.x6);
            Assert.Equal(5, t.x5);
            Assert.Equal(7, t.x7);
        }

        [UnitTest]
        public void int_1()
        {
            var t = new InterpTypeFields();
            t.x7 = 17;
            Assert.Equal(17, t.x7);
            Assert.Equal(6, t.x6);
            Assert.Equal(8, t.x8);
        }

        [UnitTest]
        public void uint_1()
        {
            var t = new InterpTypeFields();
            t.x8 = 18;
            Assert.Equal(18, t.x8);
            Assert.Equal(7, t.x7);
            Assert.Equal(9, t.x9);
        }

        [UnitTest]
        public void long_1()
        {
            var t = new InterpTypeFields();
            t.x9 = 19;
            Assert.Equal(19, t.x9);
            Assert.Equal(8, t.x8);
            Assert.Equal(10, t.x10);
        }

        [UnitTest]
        public void ulong_1()
        {
            var t = new InterpTypeFields();
            t.x10 = 20;
            Assert.Equal(20, t.x10);
            Assert.Equal(9, t.x9);
            Assert.Equal(1f, t.y1);
        }

        [UnitTest]
        public void float_1()
        {
            var t = new InterpTypeFields();
            t.y1 = 11f;
            Assert.Equal(11f, t.y1);
            Assert.Equal(10, t.x10);
            Assert.Equal(2f, t.y2);
        }

        [UnitTest]
        public void double_1()
        {
            var t = new InterpTypeFields();
            t.y3 = 13;
            Assert.Equal(13, t.y3);
            Assert.Equal(2f, t.y2);
            Assert.Equal("a", t.y4);
        }

        [UnitTest]
        public void str_1()
        {
            var t = new InterpTypeFields();
            t.y4 = "b";
            Assert.Equal("b", t.y4);
            Assert.Equal(3.0, t.y3);
        }

        [UnitTest]
        public void enum_byte_1()
        {
            var t = new InterpTypeFields();
            t.e1 = default;
            Assert.Equal(default(AOT_Enum_byte), t.e1);
            Assert.Equal(AOT_Enum_sbyte.B, t.e2);
        }

        [UnitTest]
        public void enum_sbyte_1()
        {
            var t = new InterpTypeFields();
            t.e2 = AOT_Enum_sbyte.A;
            Assert.Equal(AOT_Enum_sbyte.A, t.e2);
            Assert.Equal(AOT_Enum_byte.A, t.e1);
            Assert.Equal(AOT_Enum_short.A, t.e3);
        }

        [UnitTest]
        public void enum_short_1()
        {
            var t = new InterpTypeFields();
            t.e3 = AOT_Enum_short.B;
            Assert.Equal(AOT_Enum_short.B, t.e3);
            Assert.Equal(AOT_Enum_sbyte.B, t.e2);
            Assert.Equal(AOT_Enum_ushort.A, t.e4);
        }

        [UnitTest]
        public void enum_ushort_1()
        {
            var t = new InterpTypeFields();
            t.e4 = default;
            Assert.Equal(default(AOT_Enum_ushort), t.e4);
            Assert.Equal(AOT_Enum_short.A, t.e3);
            Assert.Equal(AOT_Enum_int.B, t.e5);
        }

        [UnitTest]
        public void enum_int_1()
        {
            var t = new InterpTypeFields();
            t.e5 = AOT_Enum_int.A;
            Assert.Equal(AOT_Enum_int.A, t.e5);
            Assert.Equal(AOT_Enum_ushort.A, t.e4);
            Assert.Equal(AOT_Enum_uint.A, t.e6);
        }

        [UnitTest]
        public void enum_uint_1()
        {
            var t = new InterpTypeFields();
            t.e6 = default;
            Assert.Equal(default(AOT_Enum_uint), t.e6);
            Assert.Equal(AOT_Enum_int.B, t.e5);
            Assert.Equal(AOT_Enum_long.B, t.e7);
        }

        [UnitTest]
        public void enum_long_1()
        {
            var t = new InterpTypeFields();
            t.e7 = AOT_Enum_long.A;
            Assert.Equal(AOT_Enum_long.A, t.e7);
            Assert.Equal(AOT_Enum_uint.A, t.e6);
            Assert.Equal(AOT_Enum_ulong.A, t.e8);
        }

        [UnitTest]
        public void enum_ulong_1()
        {
            var t = new InterpTypeFields();
            Assert.Equal(AOT_Enum_ulong.A, t.e8);
        }

        [UnitTest]
        public void valuetypesize_1()
        {
            var t = new InterpTypeFields();
            t.s1 = new ValueTypeSize1 { x1 = 10 };
            Assert.Equal(10, t.s1.x1);
            Assert.Equal("a", t.y4);
            Assert.Equal(2, t.s2.x1);
        }

        [UnitTest]
        public void valuetypesize_2()
        {
            var t = new InterpTypeFields();
            t.s2 = new ValueTypeSize2 { x1 = 10 };
            Assert.Equal(10, t.s2.x1);
            Assert.Equal(1, t.s1.x1);
            Assert.Equal(3, t.s3.x1);
        }

        [UnitTest]
        public void valuetypesize_3()
        {
            var t = new InterpTypeFields();
            t.s3 = new ValueTypeSize3 { x1 = 10 };
            Assert.Equal(10, t.s3.x1);
            Assert.Equal(2, t.s2.x1);
            Assert.Equal(4, t.s4.x1);
        }

        [UnitTest]
        public void valuetypesize_4()
        {
            var t = new InterpTypeFields();
            t.s4 = new ValueTypeSize4 { x1 = 10 };
            Assert.Equal(10, t.s4.x1);
            Assert.Equal(3, t.s3.x1);
            Assert.Equal(5, t.s5.x1);
        }

        [UnitTest]
        public void valuetypesize_5()
        {
            var t = new InterpTypeFields();
            t.s5 = new ValueTypeSize5 { x1 = 10 };
            Assert.Equal(10, t.s5.x1);
            Assert.Equal(4, t.s4.x1);
            Assert.Equal(6, t.s8.x1);
        }

        [UnitTest]
        public void valuetypesize_8()
        {
            var t = new InterpTypeFields();
            t.s8 = new ValueTypeSize8 { x1 = 10 };
            Assert.Equal(10, t.s8.x1);
            Assert.Equal(5, t.s5.x1);
            Assert.Equal(7, t.s9.x1);
        }

        [UnitTest]
        public void valuetypesize_9()
        {
            var t = new InterpTypeFields();
            t.s9 = new ValueTypeSize9 { x1 = 10 };
            Assert.Equal(10, t.s9.x1);
            Assert.Equal(6, t.s8.x1);
            Assert.Equal(8, t.s16.x1);
        }

        [UnitTest]
        public void valuetypesize_16()
        {
            var t = new InterpTypeFields();
            t.s16 = new ValueTypeSize16 { x1 = 10 };
            Assert.Equal(10, t.s16.x1);
            Assert.Equal(7, t.s9.x1);
            Assert.Equal(AOT_Enum_byte.A, t.e1);
        }
    }
}
