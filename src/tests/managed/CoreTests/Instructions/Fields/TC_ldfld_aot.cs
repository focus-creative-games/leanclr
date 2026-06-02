
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
    internal class TC_ldfld_aot : GeneralTestCaseBase
    {
        [UnitTest]
        public void byte_1()
        {
            var t = new TypeFields();
            Assert.Equal(1, t.x1);
        }

        [UnitTest]
        public void sbyte_1()
        {
            var t = new TypeFields();
            Assert.Equal(2, t.x2);
        }

        [UnitTest]
        public void bool_1()
        {
            var t = new TypeFields();
            Assert.True(t.x3);
        }

        [UnitTest]
        public void short_1()
        {
            var t = new TypeFields();
            Assert.Equal(5, t.x5);
        }

        [UnitTest]
        public void ushort_1()
        {
            var t = new TypeFields();
            Assert.Equal(6, t.x6);
        }

        [UnitTest]
        public void int_1()
        {
            var t = new TypeFields();
            Assert.Equal(7, t.x7);
        }

        [UnitTest]
        public void uint_1()
        {
            var t = new TypeFields();
            Assert.Equal(8, t.x8);
        }

        [UnitTest]
        public void long_1()
        {
            var t = new TypeFields();
            Assert.Equal(9, t.x9);
        }

        [UnitTest]
        public void ulong_1()
        {
            var t = new TypeFields();
            Assert.Equal(10, t.x10);
        }

        [UnitTest]
        public void float_1()
        {
            var t = new TypeFields();
            Assert.Equal(1f, t.y1);
        }

        [UnitTest]
        public void double_1()
        {
            var t = new TypeFields();
            Assert.Equal(3, t.y3);
        }

        [UnitTest]
        public void str_1()
        {
            var t = new TypeFields();
            Assert.Equal("a", t.y4);
        }

        [UnitTest]
        public void enum_byte_1()
        {
            var t = new TypeFields();
            Assert.Equal(AOT_Enum_byte.A, t.e1);
        }

        [UnitTest]
        public void enum_sbyte_1()
        {
            var t = new TypeFields();
            Assert.Equal(AOT_Enum_sbyte.B, t.e2);
        }

        [UnitTest]
        public void enum_short_1()
        {
            var t = new TypeFields();
            Assert.Equal(AOT_Enum_short.A, t.e3);
        }

        [UnitTest]
        public void enum_ushort_1()
        {
            var t = new TypeFields();
            Assert.Equal(AOT_Enum_ushort.A, t.e4);
        }

        [UnitTest]
        public void enum_int_1()
        {
            var t = new TypeFields();
            Assert.Equal(AOT_Enum_int.B, t.e5);
        }

        [UnitTest]
        public void enum_uint_1()
        {
            var t = new TypeFields();
            Assert.Equal(AOT_Enum_uint.A, t.e6);
        }

        [UnitTest]
        public void enum_long_1()
        {
            var t = new TypeFields();
            Assert.Equal(AOT_Enum_long.B, t.e7);
        }

        [UnitTest]
        public void enum_ulong_1()
        {
            var t = new TypeFields();
            Assert.Equal(AOT_Enum_ulong.A, t.e8);
        }

        [UnitTest]
        public void valuetypesize_1()
        {
            var t = new TypeFields();
            Assert.Equal(1, t.s1.x1);
        }

        [UnitTest]
        public void valuetypesize_2()
        {
            var t = new TypeFields();
            Assert.Equal(2, t.s2.x1);
        }

        [UnitTest]
        public void valuetypesize_3()
        {
            var t = new TypeFields();
            Assert.Equal(3, t.s3.x1);
        }

        [UnitTest]
        public void valuetypesize_4()
        {
            var t = new TypeFields();
            Assert.Equal(4, t.s4.x1);
        }

        [UnitTest]
        public void valuetypesize_5()
        {
            var t = new TypeFields();
            Assert.Equal(5, t.s5.x1);
        }

        [UnitTest]
        public void valuetypesize_8()
        {
            var t = new TypeFields();
            Assert.Equal(6, t.s8.x1);
        }

        [UnitTest]
        public void valuetypesize_9()
        {
            var t = new TypeFields();
            Assert.Equal(7, t.s9.x1);
        }

        [UnitTest]
        public void valuetypesize_16()
        {
            var t = new TypeFields();
            Assert.Equal(8, t.s16.x1);
        }
    }
}
