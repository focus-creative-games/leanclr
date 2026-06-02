
using test;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Test;
using Tests.Instruments.Objs;
using AOTDefs;

namespace Tests.Instruments.Fields
{
    internal class TC_ldflda_interp : GeneralTestCaseBase
    {
        [UnitTest]
        public void byte_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.x1;
            Assert.Equal(1, x);
        }

        [UnitTest]
        public void sbyte_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.x2;
            Assert.Equal(2, x);
        }

        [UnitTest]
        public void bool_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.x3;
            Assert.True(x);
        }

        [UnitTest]
        public void short_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.x5;
            Assert.Equal(5, x);
        }

        [UnitTest]
        public void ushort_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.x6;
            Assert.Equal(6, x);
        }

        [UnitTest]
        public void int_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.x7;
            Assert.Equal(7, x);
        }

        [UnitTest]
        public void uint_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.x8;
            Assert.Equal(8, x);
        }

        [UnitTest]
        public void long_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.x9;
            Assert.Equal(9, x);
        }

        [UnitTest]
        public void ulong_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.x10;
            Assert.Equal(10, x);
        }

        [UnitTest]
        public void float_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.y1;
            Assert.Equal(1f, x);
        }

        [UnitTest]
        public void double_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.y3;
            Assert.Equal(3, x);
        }

        [UnitTest]
        public void str_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.y4;
            Assert.Equal("a", x);
        }

        [UnitTest]
        public void enum_byte_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.e1;
            Assert.Equal(AOT_Enum_byte.A, x);
        }

        [UnitTest]
        public void enum_sbyte_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.e2;
            Assert.Equal(AOT_Enum_sbyte.B, x);
        }

        [UnitTest]
        public void enum_short_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.e3;
            Assert.Equal(AOT_Enum_short.A, x);
        }

        [UnitTest]
        public void enum_ushort_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.e4;
            Assert.Equal(AOT_Enum_ushort.A, x);
        }

        [UnitTest]
        public void enum_int_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.e5;
            Assert.Equal(AOT_Enum_int.B, x);
        }

        [UnitTest]
        public void enum_uint_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.e6;
            Assert.Equal(AOT_Enum_uint.A, x);
        }

        [UnitTest]
        public void enum_long_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.e7;
            Assert.Equal(AOT_Enum_long.B, x);
        }

        [UnitTest]
        public void enum_ulong_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.e8;
            Assert.Equal(AOT_Enum_ulong.A, x);
        }

        [UnitTest]
        public void valuetypesize_1()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.s1;
            Assert.Equal(1, x.x1);
        }

        [UnitTest]
        public void valuetypesize_2()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.s2;
            Assert.Equal(2, x.x1);
        }

        [UnitTest]
        public void valuetypesize_3()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.s3;
            Assert.Equal(3, x.x1);
        }

        [UnitTest]
        public void valuetypesize_4()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.s4;
            Assert.Equal(4, x.x1);
        }

        [UnitTest]
        public void valuetypesize_5()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.s5;
            Assert.Equal(5, x.x1);
        }

        [UnitTest]
        public void valuetypesize_8()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.s8;
            Assert.Equal(6, x.x1);
        }

        [UnitTest]
        public void valuetypesize_9()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.s9;
            Assert.Equal(7, x.x1);
        }

        [UnitTest]
        public void valuetypesize_16()
        {
            var t = new InterpTypeFields();
            ref var x = ref t.s16;
            Assert.Equal(8, x.x1);
        }

        [UnitTest]
        public unsafe void valuetypesize_large()
        {
            var lenval = sizeof(CSDT_DESK_PLAYERINFO);
            IntPtr buffPtr = Marshal.AllocHGlobal(lenval);
            byte* ptr = (byte*)buffPtr.ToPointer();
            CSDT_DESK_PLAYERINFO* info = (CSDT_DESK_PLAYERINFO*)ptr;
            info->dwPlayerNum = 123456;

            int* ptrPlayerNum = (int*)(ptr + 919808);
            Assert.Equal(123456, *ptrPlayerNum);
            Assert.Equal(123456, info->dwPlayerNum);

            uint* ptrPlayNum2 = (uint*)(&info->dwPlayerNum);
            Assert.Equal(123456, *ptrPlayNum2);
        }
    }
}
