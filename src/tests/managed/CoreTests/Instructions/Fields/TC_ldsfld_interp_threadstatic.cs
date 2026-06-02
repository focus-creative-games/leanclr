
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
//    internal class TC_ldsfld_interp_threadstatic : GeneralTestCaseBase
//    {
//        public override void SetUp()
//        {
//            base.SetUp();
//            InterpTypeThreadStaticFields.Init();
//        }

//        [UnitTest]
//        public void byte_1()
//        {
//            Assert.Equal(1, InterpTypeThreadStaticFields.x1);
//        }

//        [UnitTest]
//        public void sbyte_1()
//        {
//            Assert.Equal(2, InterpTypeThreadStaticFields.x2);
//        }

//        [UnitTest]
//        public void bool_1()
//        {
//            Assert.True(InterpTypeThreadStaticFields.x3);
//        }

//        [UnitTest]
//        public void short_1()
//        {
//            Assert.Equal(5, InterpTypeThreadStaticFields.x5);
//        }

//        [UnitTest]
//        public void ushort_1()
//        {
//            Assert.Equal(6, InterpTypeThreadStaticFields.x6);
//        }

//        [UnitTest]
//        public void int_1()
//        {
//            Assert.Equal(7, InterpTypeThreadStaticFields.x7);
//        }

//        [UnitTest]
//        public void uint_1()
//        {
//            Assert.Equal(8, InterpTypeThreadStaticFields.x8);
//        }

//        [UnitTest]
//        public void long_1()
//        {
//            Assert.Equal(9, InterpTypeThreadStaticFields.x9);
//        }

//        [UnitTest]
//        public void ulong_1()
//        {
//            Assert.Equal(10, InterpTypeThreadStaticFields.x10);
//        }

//        [UnitTest]
//        public void float_1()
//        {
//            Assert.Equal(1f, InterpTypeThreadStaticFields.y1);
//        }

//        [UnitTest]
//        public void double_1()
//        {
//            Assert.Equal(3, InterpTypeThreadStaticFields.y3);
//        }

//        [UnitTest]
//        public void str_1()
//        {
//            Assert.Equal("a", InterpTypeThreadStaticFields.y4);
//        }

//        [UnitTest]
//        public void enum_byte_1()
//        {
//            Assert.Equal(AOT_Enum_byte.A, InterpTypeThreadStaticFields.e1);
//        }

//        [UnitTest]
//        public void enum_sbyte_1()
//        {
//            Assert.Equal(AOT_Enum_sbyte.B, InterpTypeThreadStaticFields.e2);
//        }

//        [UnitTest]
//        public void enum_short_1()
//        {
//            Assert.Equal(AOT_Enum_short.A, InterpTypeThreadStaticFields.e3);
//        }

//        [UnitTest]
//        public void enum_ushort_1()
//        {
//            Assert.Equal(AOT_Enum_ushort.A, InterpTypeThreadStaticFields.e4);
//        }

//        [UnitTest]
//        public void enum_int_1()
//        {
//            Assert.Equal(AOT_Enum_int.B, InterpTypeThreadStaticFields.e5);
//        }

//        [UnitTest]
//        public void enum_uint_1()
//        {
//            Assert.Equal(AOT_Enum_uint.A, InterpTypeThreadStaticFields.e6);
//        }

//        [UnitTest]
//        public void enum_long_1()
//        {
//            Assert.Equal(AOT_Enum_long.B, InterpTypeThreadStaticFields.e7);
//        }

//        [UnitTest]
//        public void enum_ulong_1()
//        {
//            Assert.Equal(AOT_Enum_ulong.A, InterpTypeThreadStaticFields.e8);
//        }

//        [UnitTest]
//        public void valuetypesize_1()
//        {
//            Assert.Equal(1, InterpTypeThreadStaticFields.s1.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_2()
//        {
//            Assert.Equal(2, InterpTypeThreadStaticFields.s2.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_3()
//        {
//            Assert.Equal(3, InterpTypeThreadStaticFields.s3.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_4()
//        {
//            Assert.Equal(4, InterpTypeThreadStaticFields.s4.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_5()
//        {
//            Assert.Equal(5, InterpTypeThreadStaticFields.s5.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_8()
//        {
//            Assert.Equal(6, InterpTypeThreadStaticFields.s8.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_9()
//        {
//            Assert.Equal(7, InterpTypeThreadStaticFields.s9.x1);
//        }

//        [UnitTest]
//        public void valuetypesize_16()
//        {
//            Assert.Equal(8, InterpTypeThreadStaticFields.s16.x1);
//        }
//    }
//}
