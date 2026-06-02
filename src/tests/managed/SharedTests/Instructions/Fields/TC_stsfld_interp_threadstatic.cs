
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
//    internal class TC_stsfld_interp_threadstatic : GeneralTestCaseBase
//    {
//        public override void SetUp()
//        {
//            base.SetUp();
//            InterpTypeThreadStaticFields2.Init();
//        }

//        [UnitTest]
//        public void byte_1()
//        {
//            InterpTypeThreadStaticFields2.x1 = 11;
//            Assert.Equal(11, InterpTypeThreadStaticFields2.x1);
//            Assert.Equal(2, InterpTypeThreadStaticFields2.x2);
//            InterpTypeThreadStaticFields2.x1 = 1;
//        }

//        [UnitTest]
//        public void sbyte_1()
//        {
//            InterpTypeThreadStaticFields2.x2 = 11;
//            Assert.Equal(11, InterpTypeThreadStaticFields2.x2);
//            Assert.Equal(1, InterpTypeThreadStaticFields2.x1);
//            Assert.True(InterpTypeThreadStaticFields2.x3);
//            InterpTypeThreadStaticFields2.x2 = 2;
//        }

//        [UnitTest]
//        public void bool_1()
//        {
//            InterpTypeThreadStaticFields2.x3 = false;
//            Assert.False(InterpTypeThreadStaticFields2.x3);
//            Assert.Equal(2, InterpTypeThreadStaticFields2.x2);
//            Assert.Equal(4, InterpTypeThreadStaticFields2.x4);
//            InterpTypeThreadStaticFields2.x3 = true;
//        }

//        [UnitTest]
//        public void short_1()
//        {
//            InterpTypeThreadStaticFields2.x5 = 15;
//            Assert.Equal(15, InterpTypeThreadStaticFields2.x5);
//            Assert.Equal(4, InterpTypeThreadStaticFields2.x4);
//            Assert.Equal(6, InterpTypeThreadStaticFields2.x6);
//            InterpTypeThreadStaticFields2.x5 = 5;
//        }

//        [UnitTest]
//        public void ushort_1()
//        {
//            InterpTypeThreadStaticFields2.x6 = 16;
//            Assert.Equal(16, InterpTypeThreadStaticFields2.x6);
//            Assert.Equal(5, InterpTypeThreadStaticFields2.x5);
//            Assert.Equal(7, InterpTypeThreadStaticFields2.x7);
//            InterpTypeThreadStaticFields2.x6 = 6;
//        }

//        [UnitTest]
//        public void int_1()
//        {
//            InterpTypeThreadStaticFields2.x7 = 17;
//            Assert.Equal(17, InterpTypeThreadStaticFields2.x7);
//            Assert.Equal(6, InterpTypeThreadStaticFields2.x6);
//            Assert.Equal(8, InterpTypeThreadStaticFields2.x8);
//            InterpTypeThreadStaticFields2.x7 = 7;
//        }

//        [UnitTest]
//        public void uint_1()
//        {
//            InterpTypeThreadStaticFields2.x8 = 18;
//            Assert.Equal(18, InterpTypeThreadStaticFields2.x8);
//            Assert.Equal(7, InterpTypeThreadStaticFields2.x7);
//            Assert.Equal(9, InterpTypeThreadStaticFields2.x9);
//            InterpTypeThreadStaticFields2.x8 = 8;
//        }

//        [UnitTest]
//        public void long_1()
//        {
//            InterpTypeThreadStaticFields2.x9 = 19;
//            Assert.Equal(19, InterpTypeThreadStaticFields2.x9);
//            Assert.Equal(8, InterpTypeThreadStaticFields2.x8);
//            Assert.Equal(10, InterpTypeThreadStaticFields2.x10);
//            InterpTypeThreadStaticFields2.x9 = 9;
//        }

//        [UnitTest]
//        public void ulong_1()
//        {
//            InterpTypeThreadStaticFields2.x10 = 20;
//            Assert.Equal(20, InterpTypeThreadStaticFields2.x10);
//            Assert.Equal(9, InterpTypeThreadStaticFields2.x9);
//            Assert.Equal(1f, InterpTypeThreadStaticFields2.y1);
//            InterpTypeThreadStaticFields2.x10 = 10;
//        }

//        [UnitTest]
//        public void float_1()
//        {
//            InterpTypeThreadStaticFields2.y1 = 11f;
//            Assert.Equal(11f, InterpTypeThreadStaticFields2.y1);
//            Assert.Equal(10, InterpTypeThreadStaticFields2.x10);
//            Assert.Equal(2f, InterpTypeThreadStaticFields2.y2);
//            InterpTypeThreadStaticFields2.y1 = 1f;
//        }

//        [UnitTest]
//        public void double_1()
//        {
//            InterpTypeThreadStaticFields2.y3 = 13;
//            Assert.Equal(13, InterpTypeThreadStaticFields2.y3);
//            Assert.Equal(2f, InterpTypeThreadStaticFields2.y2);
//            Assert.Equal("a", InterpTypeThreadStaticFields2.y4);
//            InterpTypeThreadStaticFields2.y3 = 3f;
//        }

//        [UnitTest]
//        public void str_1()
//        {
//            InterpTypeThreadStaticFields2.y4 = "b";
//            Assert.Equal("b", InterpTypeThreadStaticFields2.y4);
//            Assert.Equal(3.0, InterpTypeThreadStaticFields2.y3);
//            InterpTypeThreadStaticFields2.y4 = "a";
//        }

//        [UnitTest]
//        public void enum_byte_1()
//        {
//            InterpTypeThreadStaticFields2.e1 = default;
//            Assert.Equal(default(AOT_Enum_byte), InterpTypeThreadStaticFields2.e1);
//            Assert.Equal(AOT_Enum_sbyte.B, InterpTypeThreadStaticFields2.e2);
//            InterpTypeThreadStaticFields2.e1 = AOT_Enum_byte.A;
//        }

//        [UnitTest]
//        public void enum_sbyte_1()
//        {
//            InterpTypeThreadStaticFields2.e2 = AOT_Enum_sbyte.A;
//            Assert.Equal(AOT_Enum_sbyte.A, InterpTypeThreadStaticFields2.e2);
//            Assert.Equal(AOT_Enum_byte.A, InterpTypeThreadStaticFields2.e1);
//            Assert.Equal(AOT_Enum_short.A, InterpTypeThreadStaticFields2.e3);
//            InterpTypeThreadStaticFields2.e2 = AOT_Enum_sbyte.B;
//        }

//        [UnitTest]
//        public void enum_short_1()
//        {
//            InterpTypeThreadStaticFields2.e3 = AOT_Enum_short.B;
//            Assert.Equal(AOT_Enum_short.B, InterpTypeThreadStaticFields2.e3);
//            Assert.Equal(AOT_Enum_sbyte.B, InterpTypeThreadStaticFields2.e2);
//            Assert.Equal(AOT_Enum_ushort.A, InterpTypeThreadStaticFields2.e4);
//            InterpTypeThreadStaticFields2.e3 = AOT_Enum_short.A;
//        }

//        [UnitTest]
//        public void enum_ushort_1()
//        {
//            InterpTypeThreadStaticFields2.e4 = default;
//            Assert.Equal(default(AOT_Enum_ushort), InterpTypeThreadStaticFields2.e4);
//            Assert.Equal(AOT_Enum_short.A, InterpTypeThreadStaticFields2.e3);
//            Assert.Equal(AOT_Enum_int.B, InterpTypeThreadStaticFields2.e5);
//            InterpTypeThreadStaticFields2.e4 = AOT_Enum_ushort.A;
//        }

//        [UnitTest]
//        public void enum_int_1()
//        {
//            InterpTypeThreadStaticFields2.e5 = AOT_Enum_int.A;
//            Assert.Equal(AOT_Enum_int.A, InterpTypeThreadStaticFields2.e5);
//            Assert.Equal(AOT_Enum_ushort.A, InterpTypeThreadStaticFields2.e4);
//            Assert.Equal(AOT_Enum_uint.A, InterpTypeThreadStaticFields2.e6);
//            InterpTypeThreadStaticFields2.e5 = AOT_Enum_int.B;
//        }

//        [UnitTest]
//        public void enum_uint_1()
//        {
//            InterpTypeThreadStaticFields2.e6 = default;
//            Assert.Equal(default(AOT_Enum_uint), InterpTypeThreadStaticFields2.e6);
//            Assert.Equal(AOT_Enum_int.B, InterpTypeThreadStaticFields2.e5);
//            Assert.Equal(AOT_Enum_long.B, InterpTypeThreadStaticFields2.e7);
//            InterpTypeThreadStaticFields2.e6 = AOT_Enum_uint.A;
//        }

//        [UnitTest]
//        public void enum_long_1()
//        {
//            InterpTypeThreadStaticFields2.e7 = AOT_Enum_long.A;
//            Assert.Equal(AOT_Enum_long.A, InterpTypeThreadStaticFields2.e7);
//            Assert.Equal(AOT_Enum_uint.A, InterpTypeThreadStaticFields2.e6);
//            Assert.Equal(AOT_Enum_ulong.A, InterpTypeThreadStaticFields2.e8);
//            InterpTypeThreadStaticFields2.e7 = AOT_Enum_long.B;
//        }

//        [UnitTest]
//        public void enum_ulong_1()
//        {
//            InterpTypeThreadStaticFields2.e8 = default;
//            Assert.Equal(default(AOT_Enum_ulong), InterpTypeThreadStaticFields2.e8);
//            InterpTypeThreadStaticFields2.e8 = AOT_Enum_ulong.A;
//        }

//        [UnitTest]
//        public void valuetypesize_1()
//        {
//            InterpTypeThreadStaticFields2.s1 = new ValueTypeSize1 { x1 = 10 };
//            Assert.Equal(10, InterpTypeThreadStaticFields2.s1.x1);
//            Assert.Equal("a", InterpTypeThreadStaticFields2.y4);
//            Assert.Equal(2, InterpTypeThreadStaticFields2.s2.x1);
//            InterpTypeThreadStaticFields2.s1 = new ValueTypeSize1 { x1 = 1 };
//        }

//        [UnitTest]
//        public void valuetypesize_2()
//        {
//            InterpTypeThreadStaticFields2.s2 = new ValueTypeSize2 { x1 = 10 };
//            Assert.Equal(10, InterpTypeThreadStaticFields2.s2.x1);
//            Assert.Equal(1, InterpTypeThreadStaticFields2.s1.x1);
//            Assert.Equal(3, InterpTypeThreadStaticFields2.s3.x1);
//            InterpTypeThreadStaticFields2.s2 = new ValueTypeSize2 { x1 = 2 };
//        }

//        [UnitTest]
//        public void valuetypesize_3()
//        {
//            InterpTypeThreadStaticFields2.s3 = new ValueTypeSize3 { x1 = 10 };
//            Assert.Equal(10, InterpTypeThreadStaticFields2.s3.x1);
//            Assert.Equal(2, InterpTypeThreadStaticFields2.s2.x1);
//            Assert.Equal(4, InterpTypeThreadStaticFields2.s4.x1);
//            InterpTypeThreadStaticFields2.s3 = new ValueTypeSize3 { x1 = 3 };
//        }

//        [UnitTest]
//        public void valuetypesize_4()
//        {
//            InterpTypeThreadStaticFields2.s4 = new ValueTypeSize4 { x1 = 10 };
//            Assert.Equal(10, InterpTypeThreadStaticFields2.s4.x1);
//            Assert.Equal(3, InterpTypeThreadStaticFields2.s3.x1);
//            Assert.Equal(5, InterpTypeThreadStaticFields2.s5.x1);
//            InterpTypeThreadStaticFields2.s4 = new ValueTypeSize4 { x1 = 4 };
//        }

//        [UnitTest]
//        public void valuetypesize_5()
//        {
//            InterpTypeThreadStaticFields2.s5 = new ValueTypeSize5 { x1 = 10 };
//            Assert.Equal(10, InterpTypeThreadStaticFields2.s5.x1);
//            Assert.Equal(4, InterpTypeThreadStaticFields2.s4.x1);
//            Assert.Equal(6, InterpTypeThreadStaticFields2.s8.x1);
//            InterpTypeThreadStaticFields2.s5 = new ValueTypeSize5 { x1 = 5 };
//        }

//        [UnitTest]
//        public void valuetypesize_8()
//        {
//            InterpTypeThreadStaticFields2.s8 = new ValueTypeSize8 { x1 = 10 };
//            Assert.Equal(10, InterpTypeThreadStaticFields2.s8.x1);
//            Assert.Equal(5, InterpTypeThreadStaticFields2.s5.x1);
//            Assert.Equal(7, InterpTypeThreadStaticFields2.s9.x1);
//            InterpTypeThreadStaticFields2.s8 = new ValueTypeSize8 { x1 = 6 };
//        }

//        [UnitTest]
//        public void valuetypesize_9()
//        {
//            InterpTypeThreadStaticFields2.s9 = new ValueTypeSize9 { x1 = 10 };
//            Assert.Equal(10, InterpTypeThreadStaticFields2.s9.x1);
//            Assert.Equal(6, InterpTypeThreadStaticFields2.s8.x1);
//            Assert.Equal(8, InterpTypeThreadStaticFields2.s16.x1);
//            InterpTypeThreadStaticFields2.s9 = new ValueTypeSize9 { x1 = 7 };
//        }

//        [UnitTest]
//        public void valuetypesize_16()
//        {
//            InterpTypeThreadStaticFields2.s16 = new ValueTypeSize16 { x1 = 10 };
//            Assert.Equal(10, InterpTypeThreadStaticFields2.s16.x1);
//            Assert.Equal(7, InterpTypeThreadStaticFields2.s9.x1);
//            Assert.Equal(AOT_Enum_byte.A, InterpTypeThreadStaticFields2.e1);
//            InterpTypeThreadStaticFields2.s16 = new ValueTypeSize16 { x1 = 8 };
//        }
//    }
//}
