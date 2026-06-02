
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
    internal class TC_ldsfld_interp : GeneralTestCaseBase
    {
        [UnitTest]
        public void byte_1()
        {
            Assert.Equal(1, TypeStaticFields.x1);
        }

        [UnitTest]
        public void sbyte_1()
        {
            Assert.Equal(2, TypeStaticFields.x2);
        }

        [UnitTest]
        public void bool_1()
        {
            Assert.True(TypeStaticFields.x3);
        }

        [UnitTest]
        public void short_1()
        {
            Assert.Equal(5, TypeStaticFields.x5);
        }

        [UnitTest]
        public void ushort_1()
        {
            Assert.Equal(6, TypeStaticFields.x6);
        }

        [UnitTest]
        public void int_1()
        {
            Assert.Equal(7, TypeStaticFields.x7);
        }

        [UnitTest]
        public void uint_1()
        {
            Assert.Equal(8, TypeStaticFields.x8);
        }

        [UnitTest]
        public void long_1()
        {
            Assert.Equal(9, TypeStaticFields.x9);
        }

        [UnitTest]
        public void ulong_1()
        {
            Assert.Equal(10, TypeStaticFields.x10);
        }

        [UnitTest]
        public void float_1()
        {
            Assert.Equal(1f, TypeStaticFields.y1);
        }

        [UnitTest]
        public void double_1()
        {
            Assert.Equal(3, TypeStaticFields.y3);
        }

        [UnitTest]
        public void str_1()
        {
            Assert.Equal("a", TypeStaticFields.y4);
        }

        [UnitTest]
        public void enum_byte_1()
        {
            Assert.Equal(AOT_Enum_byte.A, TypeStaticFields.e1);
        }

        [UnitTest]
        public void enum_sbyte_1()
        {
            Assert.Equal(AOT_Enum_sbyte.B, TypeStaticFields.e2);
        }

        [UnitTest]
        public void enum_short_1()
        {
            Assert.Equal(AOT_Enum_short.A, TypeStaticFields.e3);
        }

        [UnitTest]
        public void enum_ushort_1()
        {
            Assert.Equal(AOT_Enum_ushort.A, TypeStaticFields.e4);
        }

        [UnitTest]
        public void enum_int_1()
        {
            Assert.Equal(AOT_Enum_int.B, TypeStaticFields.e5);
        }

        [UnitTest]
        public void enum_uint_1()
        {
            Assert.Equal(AOT_Enum_uint.A, TypeStaticFields.e6);
        }

        [UnitTest]
        public void enum_long_1()
        {
            Assert.Equal(AOT_Enum_long.B, TypeStaticFields.e7);
        }

        [UnitTest]
        public void enum_ulong_1()
        {
            Assert.Equal(AOT_Enum_ulong.A, TypeStaticFields.e8);
        }

        [UnitTest]
        public void valuetypesize_1()
        {
            Assert.Equal(1, TypeStaticFields.s1.x1);
        }

        [UnitTest]
        public void valuetypesize_2()
        {
            Assert.Equal(2, TypeStaticFields.s2.x1);
        }

        [UnitTest]
        public void valuetypesize_3()
        {
            Assert.Equal(3, TypeStaticFields.s3.x1);
        }

        [UnitTest]
        public void valuetypesize_4()
        {
            Assert.Equal(4, TypeStaticFields.s4.x1);
        }

        [UnitTest]
        public void valuetypesize_5()
        {
            Assert.Equal(5, TypeStaticFields.s5.x1);
        }

        [UnitTest]
        public void valuetypesize_8()
        {
            Assert.Equal(6, TypeStaticFields.s8.x1);
        }

        [UnitTest]
        public void valuetypesize_9()
        {
            Assert.Equal(7, TypeStaticFields.s9.x1);
        }

        [UnitTest]
        public void valuetypesize_16()
        {
            Assert.Equal(8, TypeStaticFields.s16.x1);
        }
    }
}
