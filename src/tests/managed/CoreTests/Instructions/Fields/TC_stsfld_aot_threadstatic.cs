
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
//    internal class TC_stsfld_aot_threadstatic : GeneralTestCaseBase
//    {
//        public override void SetUp()
//        {
//            base.SetUp();
//            TypeThreadStaticFields2.Init();
//        }

//        [UnitTest]
//        public void byte_1()
//        {
//            TypeThreadStaticFields2.x1 = 11;
//            Assert.Equal(11, TypeThreadStaticFields2.x1);
//            Assert.Equal(2, TypeThreadStaticFields2.x2);
//            TypeThreadStaticFields2.x1 = 1;
//        }

//        [UnitTest]
//        public void sbyte_1()
//        {
//            TypeThreadStaticFields2.x2 = 11;
//            Assert.Equal(11, TypeThreadStaticFields2.x2);
//            Assert.Equal(1, TypeThreadStaticFields2.x1);
//            Assert.True(TypeThreadStaticFields2.x3);
//            TypeThreadStaticFields2.x2 = 2;
//        }

//        [UnitTest]
//        public void bool_1()
//        {
//            TypeThreadStaticFields2.x3 = false;
//            Assert.False(TypeThreadStaticFields2.x3);
//            Assert.Equal(2, TypeThreadStaticFields2.x2);
//            Assert.Equal(4, TypeThreadStaticFields2.x4);
//            TypeThreadStaticFields2.x3 = true;
//        }

//        [UnitTest]
//        public void short_1()
//        {
//            TypeThreadStaticFields2.x5 = 15;
//            Assert.Equal(15, TypeThreadStaticFields2.x5);
//            Assert.Equal(4, TypeThreadStaticFields2.x4);
//            Assert.Equal(6, TypeThreadStaticFields2.x6);
//            TypeThreadStaticFields2.x5 = 5;
//        }

//        [UnitTest]
//        public void ushort_1()
//        {
//            TypeThreadStaticFields2.x6 = 16;
//            Assert.Equal(16, TypeThreadStaticFields2.x6);
//            Assert.Equal(5, TypeThreadStaticFields2.x5);
//            Assert.Equal(7, TypeThreadStaticFields2.x7);
//            TypeThreadStaticFields2.x6 = 6;
//        }

//        [UnitTest]
//        public void int_1()
//        {
//            TypeThreadStaticFields2.x7 = 17;
//            Assert.Equal(17, TypeThreadStaticFields2.x7);
//            Assert.Equal(6, TypeThreadStaticFields2.x6);
//            Assert.Equal(8, TypeThreadStaticFields2.x8);
//            TypeThreadStaticFields2.x7 = 7;
//        }

//        [UnitTest]
//        public void uint_1()
//        {
//            TypeThreadStaticFields2.x8 = 18;
//            Assert.Equal(18, TypeThreadStaticFields2.x8);
//            Assert.Equal(7, TypeThreadStaticFields2.x7);
//            Assert.Equal(9, TypeThreadStaticFields2.x9);
//            TypeThreadStaticFields2.x8 = 8;
//        }

//        [UnitTest]
//        public void long_1()
//        {
//            TypeThreadStaticFields2.x9 = 19;
//            Assert.Equal(19, TypeThreadStaticFields2.x9);
//            Assert.Equal(8, TypeThreadStaticFields2.x8);
//            Assert.Equal(10, TypeThreadStaticFields2.x10);
//            TypeThreadStaticFields2.x9 = 9;
//        }

//        [UnitTest]
//        public void ulong_1()
//        {
//            TypeThreadStaticFields2.x10 = 20;
//            Assert.Equal(20, TypeThreadStaticFields2.x10);
//            Assert.Equal(9, TypeThreadStaticFields2.x9);
//            Assert.Equal(1f, TypeThreadStaticFields2.y1);
//            TypeThreadStaticFields2.x10 = 10;
//        }

//        [UnitTest]
//        public void float_1()
//        {
//            TypeThreadStaticFields2.y1 = 11f;
//            Assert.Equal(11f, TypeThreadStaticFields2.y1);
//            Assert.Equal(10, TypeThreadStaticFields2.x10);
//            Assert.Equal(2f, TypeThreadStaticFields2.y2);
//            TypeThreadStaticFields2.y1 = 1f;
//        }

//        [UnitTest]
//        public void double_1()
//        {
//            TypeThreadStaticFields2.y3 = 13;
//            Assert.Equal(13, TypeThreadStaticFields2.y3);
//            Assert.Equal(2f, TypeThreadStaticFields2.y2);
//            Assert.Equal("a", TypeThreadStaticFields2.y4);
//            TypeThreadStaticFields2.y3 = 3f;
//        }

//        [UnitTest]
//        public void str_1()
//        {
//            TypeThreadStaticFields2.y4 = "b";
//            Assert.Equal("b", TypeThreadStaticFields2.y4);
//            Assert.Equal(3.0, TypeThreadStaticFields2.y3);
//            TypeThreadStaticFields2.y4 = "a";
//        }

//        [UnitTest]
//        public void enum_byte_1()
//        {
//            TypeThreadStaticFields2.e1 = default;
//            Assert.Equal(default(AOT_Enum_byte), TypeThreadStaticFields2.e1);
//            Assert.Equal(AOT_Enum_sbyte.B, TypeThreadStaticFields2.e2);
//            TypeThreadStaticFields2.e1 = AOT_Enum_byte.A;
//        }

//        [UnitTest]
//        public void enum_sbyte_1()
//        {
//            TypeThreadStaticFields2.e2 = AOT_Enum_sbyte.A;
//            Assert.Equal(AOT_Enum_sbyte.A, TypeThreadStaticFields2.e2);
//            Assert.Equal(AOT_Enum_byte.A, TypeThreadStaticFields2.e1);
//            Assert.Equal(AOT_Enum_short.A, TypeThreadStaticFields2.e3);
//            TypeThreadStaticFields2.e2 = AOT_Enum_sbyte.B;
//        }

//        [UnitTest]
//        public void enum_short_1()
//        {
//            TypeThreadStaticFields2.e3 = AOT_Enum_short.B;
//            Assert.Equal(AOT_Enum_short.B, TypeThreadStaticFields2.e3);
//            Assert.Equal(AOT_Enum_sbyte.B, TypeThreadStaticFields2.e2);
//            Assert.Equal(AOT_Enum_ushort.A, TypeThreadStaticFields2.e4);
//            TypeThreadStaticFields2.e3 = AOT_Enum_short.A;
//        }

//        [UnitTest]
//        public void enum_ushort_1()
//        {
//            TypeThreadStaticFields2.e4 = default;
//            Assert.Equal(default(AOT_Enum_ushort), TypeThreadStaticFields2.e4);
//            Assert.Equal(AOT_Enum_short.A, TypeThreadStaticFields2.e3);
//            Assert.Equal(AOT_Enum_int.B, TypeThreadStaticFields2.e5);
//            TypeThreadStaticFields2.e4 = AOT_Enum_ushort.A;
//        }

//        [UnitTest]
//        public void enum_int_1()
//        {
//            TypeThreadStaticFields2.e5 = AOT_Enum_int.A;
//            Assert.Equal(AOT_Enum_int.A, TypeThreadStaticFields2.e5);
//            Assert.Equal(AOT_Enum_ushort.A, TypeThreadStaticFields2.e4);
//            Assert.Equal(AOT_Enum_uint.A, TypeThreadStaticFields2.e6);
//            TypeThreadStaticFields2.e5 = AOT_Enum_int.B;
//        }

//        [UnitTest]
//        public void enum_uint_1()
//        {
//            TypeThreadStaticFields2.e6 = default;
//            Assert.Equal(default(AOT_Enum_uint), TypeThreadStaticFields2.e6);
//            Assert.Equal(AOT_Enum_int.B, TypeThreadStaticFields2.e5);
//            Assert.Equal(AOT_Enum_long.B, TypeThreadStaticFields2.e7);
//            TypeThreadStaticFields2.e6 = AOT_Enum_uint.A;
//        }

//        [UnitTest]
//        public void enum_long_1()
//        {
//            TypeThreadStaticFields2.e7 = AOT_Enum_long.A;
//            Assert.Equal(AOT_Enum_long.A, TypeThreadStaticFields2.e7);
//            Assert.Equal(AOT_Enum_uint.A, TypeThreadStaticFields2.e6);
//            Assert.Equal(AOT_Enum_ulong.A, TypeThreadStaticFields2.e8);
//            TypeThreadStaticFields2.e7 = AOT_Enum_long.B;
//        }

//        [UnitTest]
//        public void enum_ulong_1()
//        {
//            TypeThreadStaticFields2.e8 = default;
//            Assert.Equal(default(AOT_Enum_ulong), TypeThreadStaticFields2.e8);
//            TypeThreadStaticFields2.e8 = AOT_Enum_ulong.A;
//        }

//        [UnitTest]
//        public void valuetypesize_1()
//        {
//            TypeThreadStaticFields2.s1 = new ValueTypeSize1 { x1 = 10 };
//            Assert.Equal(10, TypeThreadStaticFields2.s1.x1);
//            Assert.Equal("a", TypeThreadStaticFields2.y4);
//            Assert.Equal(2, TypeThreadStaticFields2.s2.x1);
//            TypeThreadStaticFields2.s1 = new ValueTypeSize1 { x1 = 1 };
//        }

//        [UnitTest]
//        public void valuetypesize_2()
//        {
//            TypeThreadStaticFields2.s2 = new ValueTypeSize2 { x1 = 10 };
//            Assert.Equal(10, TypeThreadStaticFields2.s2.x1);
//            Assert.Equal(1, TypeThreadStaticFields2.s1.x1);
//            Assert.Equal(3, TypeThreadStaticFields2.s3.x1);
//            TypeThreadStaticFields2.s2 = new ValueTypeSize2 { x1 = 2 };
//        }

//        [UnitTest]
//        public void valuetypesize_3()
//        {
//            TypeThreadStaticFields2.s3 = new ValueTypeSize3 { x1 = 10 };
//            Assert.Equal(10, TypeThreadStaticFields2.s3.x1);
//            Assert.Equal(2, TypeThreadStaticFields2.s2.x1);
//            Assert.Equal(4, TypeThreadStaticFields2.s4.x1);
//            TypeThreadStaticFields2.s3 = new ValueTypeSize3 { x1 = 3 };
//        }

//        [UnitTest]
//        public void valuetypesize_4()
//        {
//            TypeThreadStaticFields2.s4 = new ValueTypeSize4 { x1 = 10 };
//            Assert.Equal(10, TypeThreadStaticFields2.s4.x1);
//            Assert.Equal(3, TypeThreadStaticFields2.s3.x1);
//            Assert.Equal(5, TypeThreadStaticFields2.s5.x1);
//            TypeThreadStaticFields2.s4 = new ValueTypeSize4 { x1 = 4 };
//        }

//        [UnitTest]
//        public void valuetypesize_5()
//        {
//            TypeThreadStaticFields2.s5 = new ValueTypeSize5 { x1 = 10 };
//            Assert.Equal(10, TypeThreadStaticFields2.s5.x1);
//            Assert.Equal(4, TypeThreadStaticFields2.s4.x1);
//            Assert.Equal(6, TypeThreadStaticFields2.s8.x1);
//            TypeThreadStaticFields2.s5 = new ValueTypeSize5 { x1 = 5 };
//        }

//        [UnitTest]
//        public void valuetypesize_8()
//        {
//            TypeThreadStaticFields2.s8 = new ValueTypeSize8 { x1 = 10 };
//            Assert.Equal(10, TypeThreadStaticFields2.s8.x1);
//            Assert.Equal(5, TypeThreadStaticFields2.s5.x1);
//            Assert.Equal(7, TypeThreadStaticFields2.s9.x1);
//            TypeThreadStaticFields2.s8 = new ValueTypeSize8 { x1 = 6 };
//        }

//        [UnitTest]
//        public void valuetypesize_9()
//        {
//            TypeThreadStaticFields2.s9 = new ValueTypeSize9 { x1 = 10 };
//            Assert.Equal(10, TypeThreadStaticFields2.s9.x1);
//            Assert.Equal(6, TypeThreadStaticFields2.s8.x1);
//            Assert.Equal(8, TypeThreadStaticFields2.s16.x1);
//            TypeThreadStaticFields2.s9 = new ValueTypeSize9 { x1 = 7 };
//        }

//        [UnitTest]
//        public void valuetypesize_16()
//        {
//            TypeThreadStaticFields2.s16 = new ValueTypeSize16 { x1 = 10 };
//            Assert.Equal(10, TypeThreadStaticFields2.s16.x1);
//            Assert.Equal(7, TypeThreadStaticFields2.s9.x1);
//            Assert.Equal(AOT_Enum_byte.A, TypeThreadStaticFields2.e1);
//            TypeThreadStaticFields2.s16 = new ValueTypeSize16 { x1 = 8 };
//        }
//    }
//}
