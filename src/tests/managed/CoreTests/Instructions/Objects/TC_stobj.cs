
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Test;
using AOTDefs;

namespace Tests.Instruments.Objs
{
    internal class TC_stobj : GeneralTestCaseBase
    {
        public static void SetObj<T>(ref T a, T b)
        {
            a = b;
        }

        [UnitTest]
        public void SetBool()
        {
            bool a = false;
            bool b = true;
            bool c = false;
            SetObj(ref a, b);
            Assert.True(a);
            Assert.False(c);
        }

        [UnitTest]
        public void SetSbyte()
        {
            sbyte a = 0;
            sbyte b = 1;
            sbyte c = 2;
            SetObj(ref a, b);
            Assert.Equal(1, a);
            Assert.Equal(2, c);
        }
        
        [UnitTest]
        public void SetByte()
        {
            byte a = 0;
            byte b = 1;
            byte c = 2;
            SetObj(ref a, b);
            Assert.Equal(1, a);
            Assert.Equal(2, c);
        }

        [UnitTest]
        public void SetShort()
        {
            short a = 0;
            short b = 1;
            short c = 2;
            SetObj(ref a, b);
            Assert.Equal(1, a);
            Assert.Equal(2, c);
        }

        [UnitTest]
        public void SetUshort()
        {
            ushort a = 0;
            ushort b = 1;
            ushort c = 2;
            SetObj(ref a, b);
            Assert.Equal(1, a);
            Assert.Equal(2, c);
        }

        [UnitTest]
        public void SetInt()
        {
            int a = 0;
            int b = 1;
            int c = 2;
            SetObj(ref a, b);
            Assert.Equal(1, a);
            Assert.Equal(2, c);
        }

        [UnitTest]
        public void SetUint()
        {
            uint a = 0;
            uint b = 1;
            uint c = 2;
            SetObj(ref a, b);
            Assert.Equal(1, a);
            Assert.Equal(2, c);
        }

        [UnitTest]
        public void SetLong()
        {
            long a = 0;
            long b = 1;
            long c = 2;
            SetObj(ref a, b);
            Assert.Equal(1, a);
            Assert.Equal(2, c);
        }

        [UnitTest]
        public void SetUlong()
        {
            ulong a = 0;
            ulong b = 1;
            ulong c = 2;
            SetObj(ref a, b);
            Assert.Equal(1, a);
            Assert.Equal(2, c);
        }

        [UnitTest]
        public void SetFloat()
        {
            float a = 0;
            float b = 1;
            float c = 2;
            SetObj(ref a, b);
            Assert.Equal(1f, a);
            Assert.Equal(2f, c);
        }

        [UnitTest]
        public void SetDouble()
        {
            double a = 0;
            double b = 1;
            double c = 2;
            SetObj(ref a, b);
            Assert.Equal(1.0, a);
            Assert.Equal(2.0, c);
        }

        [UnitTest]
        public void SetRef()
        {
            string a = null;
            string b = "abc";
            string c = "def";
            SetObj(ref a, b);
            Assert.Equal("abc", a);
            Assert.Equal("def", c);
        }

        [UnitTest]
        public void SetStruct1()
        {
            ValueTypeSize1 a = default;
            ValueTypeSize1 b = new ValueTypeSize1 { x1 = 1 };
            ValueTypeSize1 c = new ValueTypeSize1 { x1 = 2 };
            SetObj(ref a, b);
            Assert.Equal(1, a.x1);
            Assert.Equal(2, c.x1);
        }

        [UnitTest]
        public void SetStruct2()
        {
            ValueTypeSize2 a = default;
            ValueTypeSize2 b = new ValueTypeSize2 { x1 = 1, x2 = 2 };
            ValueTypeSize2 c = new ValueTypeSize2 { x1 = 3, x2 = 4 };
            SetObj(ref a, b);
            Assert.Equal(1, a.x1);
            Assert.Equal(2, a.x2);
            Assert.Equal(3, c.x1);
            Assert.Equal(4, c.x2);
        }

        [UnitTest]
        public void SetStruct9()
        {
            ValueTypeSize9 a = default;
            ValueTypeSize9 b = new ValueTypeSize9 { x1 = 1, x9 = 9 };
            ValueTypeSize9 c = new ValueTypeSize9 { x1 = 11, x9 = 19 };
            SetObj(ref a, b);
            Assert.Equal(1, a.x1);
            Assert.Equal(9, a.x9);
            Assert.Equal(11, c.x1);
            Assert.Equal(19, c.x9);
        }

        [UnitTest]
        public void SetStruct20()
        {
            ValueTypeSize20 a = default;
            ValueTypeSize20 b = new ValueTypeSize20 { x1 = 1, x5 = 9 };
            ValueTypeSize20 c = new ValueTypeSize20 { x1 = 11, x5 = 19 };
            SetObj(ref a, b);
            Assert.Equal(1, a.x1);
            Assert.Equal(9, a.x5);
            Assert.Equal(11, c.x1);
            Assert.Equal(19, c.x5);
        }
    }
}
