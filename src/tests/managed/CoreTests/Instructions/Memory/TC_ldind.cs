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
    class TC_ldind : GeneralTestCaseBase
    {
        [UnitTest]
        public static void i1_1()
        {
            sbyte i = 1;
            ref sbyte j = ref i;
            Assert.Equal(1, j);
        }

        [UnitTest]
        public static void i1_2()
        {
            sbyte i = -1;
            ref sbyte j = ref i;
            Assert.Equal(-1, j);
        }

        [UnitTest]
        public static void u1()
        {
            byte i = 1;
            ref byte j = ref i;
            Assert.Equal(1, j);
        }

        [UnitTest]
        public static void i2()
        {
            short i = 1;
            ref short j = ref i;
            Assert.Equal(1, j);
        }

        [UnitTest]
        public static void i2_2()
        {
            short i = -1;
            ref short j = ref i;
            Assert.Equal(-1, j);
        }

        [UnitTest]
        public static void u2()
        {
            ushort i = 1;
            ref ushort j = ref i;
            Assert.Equal(1, j);
        }

        [UnitTest]
        public static void i4()
        {
            int i = 1;
            ref int j = ref i;
            Assert.Equal(1, j);
        }

        [UnitTest]
        public static void i4_2()
        {
            int i = -1;
            ref int j = ref i;
            Assert.Equal(-1, j);
        }

        [UnitTest]
        public static void u4()
        {
            uint i = 1U;
            ref uint j = ref i;
            Assert.Equal(1U, j);
        }

        [UnitTest]
        public static void i8()
        {
            long i = 1;
            ref long j = ref i;
            Assert.Equal(1, j);
        }

        [UnitTest]
        public static void i8_2()
        {
            long i = -1;
            ref long j = ref i;
            Assert.Equal(-1L, j);
        }

        // ldind.u8 是ldind.i8的别名
        public static void u8()
        {
            ulong i = 1;
            ref ulong j = ref i;
            Assert.Equal(1, j);
        }

        [UnitTest]
        public static void nint_1()
        {
            IntPtr i = new IntPtr(1);
            ref var j = ref i;
            Assert.Equal(i, j);
        }

        [UnitTest]
        public static void nint_2()
        {
            IntPtr i = new IntPtr(-1);
            ref var j = ref i;
            Assert.Equal(new IntPtr(-1), j);
        }

        [UnitTest]
        public static void r4()
        {
            float i = 1f;
            ref float j = ref i;
            Assert.Equal(1f, j);
        }

        [UnitTest]
        public static void r8()
        {
            double i = 1.0;
            ref double j = ref i;
            Assert.Equal(1.0, j);
        }

        [UnitTest]
        public static void obj()
        {
            object o = "a";
            ref object j = ref o;
            Assert.Equal(o, j);
        }


        [UnitTest]
        public static void valuetype1()
        {
            var i = new ValueTypeSize1 { x1 = 1 };
            ref var j = ref i;
            var j2 = j;
            Assert.Equal(1, j2.x1);
        }

        [UnitTest]
        public static void valuetype2()
        {
            var i = new ValueTypeSize2 { x1 = 1 };
            ref var j = ref i;
            var j2 = j;
            Assert.Equal(1, j2.x1);
        }

        [UnitTest]
        public static void valuetype3()
        {
            var i = new ValueTypeSize3 { x1 = 1 };
            ref var j = ref i;
            var j2 = j;
            Assert.Equal(1, j2.x1);
        }

        [UnitTest]
        public static void valuetype4()
        {
            var i = new ValueTypeSize4 { x1 = 1 };
            ref var j = ref i;
            var j2 = j;
            Assert.Equal(1, j2.x1);
        }

        [UnitTest]
        public static void valuetype5()
        {
            var i = new ValueTypeSize5 { x1 = 1 };
            ref var j = ref i;
            var j2 = j;
            Assert.Equal(1, j2.x1);
        }

        [UnitTest]
        public static void valuetype8()
        {
            var i = new ValueTypeSize8 { x1 = 1 };
            ref var j = ref i;
            var j2 = j;
            Assert.Equal(1, j2.x1);
        }

        [UnitTest]
        public static void valuetype9()
        {
            var i = new ValueTypeSize9 { x1 = 1 };
            ref var j = ref i;
            var j2 = j;
            Assert.Equal(1, j2.x1);
        }

        [UnitTest]
        public static void valuetype16()
        {
            var i = new ValueTypeSize16 { x1 = 1 };
            ref var j = ref i;
            var j2 = j;
            Assert.Equal(1, j2.x1);
        }

        [UnitTest]
        public static void valuetype20()
        {
            var i = new ValueTypeSize20 { x1 = 1 };
            ref var j = ref i;
            var j2 = j;
            Assert.Equal(1, j2.x1);
        }
    }
}
