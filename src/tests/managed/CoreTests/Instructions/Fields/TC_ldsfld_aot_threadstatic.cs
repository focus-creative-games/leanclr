
//using test;
//using System;
//using System.Collections;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;
//using Test;

//namespace Tests.Instruments.Fields
//{
//    internal class TC_ldsfld_aot_threadstatic : GeneralTestCaseBase
//    {
//        public override void SetUp()
//        {
//            base.SetUp();
//            TypeThreadStaticFields.Init();
//        }

//        [UnitTest]
//        public void byte_1()
//        {
//            Assert.Equal(1, TypeThreadStaticFields.x1);
//        }

//        [UnitTest]
//        public void sbyte_1()
//        {
//            Assert.Equal(2, TypeThreadStaticFields.x2);
//        }

//        [UnitTest]
//        public void bool_1()
//        {
//            Assert.True(TypeThreadStaticFields.x3);
//        }

//        [UnitTest]
//        public void short_1()
//        {
//            Assert.Equal(5, TypeThreadStaticFields.x5);
//        }

//        [UnitTest]
//        public void ushort_1()
//        {
//            Assert.Equal(6, TypeThreadStaticFields.x6);
//        }

//        [UnitTest]
//        public void int_1()
//        {
//            Assert.Equal(7, TypeThreadStaticFields.x7);
//        }

//        [UnitTest]
//        public void uint_1()
//        {
//            Assert.Equal(8, TypeThreadStaticFields.x8);
//        }

//        [UnitTest]
//        public void long_1()
//        {
//            Assert.Equal(9, TypeThreadStaticFields.x9);
//        }

//        [UnitTest]
//        public void ulong_1()
//        {
//            Assert.Equal(10, TypeThreadStaticFields.x10);
//        }

//        [UnitTest]
//        public void float_1()
//        {
//            Assert.Equal(1f, TypeThreadStaticFields.y1);
//        }

//        [UnitTest]
//        public void double_1()
//        {
//            Assert.Equal(3, TypeThreadStaticFields.y3);
//        }

//        [UnitTest]
//        public void str_1()
//        {
//            Assert.Equal("a", TypeThreadStaticFields.y4);
//        }

//        [UnitTest]
//        public void enum_byte_1()
//        {
//            Assert.Equal(AOT_Enum_byte.A, TypeThreadStaticFields.e1);
//        }

//        [UnitTest]
//        public void enum_sbyte_1()
//        {
//            Assert.Equal(AOT_Enum_sbyte.B, TypeThreadStaticFields.e2);
//        }

//        [UnitTest]
//        public void enum_short_1()
//        {
//            Assert.Equal(AOT_Enum_short.A, TypeThreadStaticFields.e3);
//        }

//        [UnitTest]
//        public void enum_ushort_1()
//        {
//            Assert.Equal(AOT_Enum_ushort.A, TypeThreadStaticFields.e4);
//        }

//        [UnitTest]
//        public void enum_int_1()
//        {
//            Assert.Equal(AOT_Enum_int.B, TypeThreadStaticFields.e5);
//        }

//        [UnitTest]
//        public void enum_uint_1()
//        {
//            Assert.Equal(AOT_Enum_uint.A, TypeThreadStaticFields.e6);
//        }

//        [UnitTest]
//        public void enum_long_1()
//        {
//            Assert.Equal(AOT_Enum_long.B, TypeThreadStaticFields.e7);
//        }

//        [UnitTest]
//        public void enum_ulong_1()
//        {
//            Assert.Equal(AOT_Enum_ulong.A, TypeThreadStaticFields.e8);
//        }

//        [UnitTest]
//        public void valuetypesize_1()
//        {
//            Assert.Equal(1, TypeThreadStaticFields.s1.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_2()
//        {
//            Assert.Equal(2, TypeThreadStaticFields.s2.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_3()
//        {
//            Assert.Equal(3, TypeThreadStaticFields.s3.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_4()
//        {
//            Assert.Equal(4, TypeThreadStaticFields.s4.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_5()
//        {
//            Assert.Equal(5, TypeThreadStaticFields.s5.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_8()
//        {
//            Assert.Equal(6, TypeThreadStaticFields.s8.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_9()
//        {
//            Assert.Equal(7, TypeThreadStaticFields.s9.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_16()
//        {
//            Assert.Equal(8, TypeThreadStaticFields.s16.x1);
//        }
//    }
//}
