using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Mems
{
    internal class TC_ldloc: GeneralTestCaseBase
    {
        [UnitTest]
        public void byte_1()
        {
            byte a = 1;
            Assert.Equal(1, a);
        }

        [UnitTest]
        public void sbyte_1()
        {
            sbyte a = 1;
            Assert.Equal(1, a);
        }

        [UnitTest]
        public void sbyte_2()
        {
            sbyte a = -1;
            Assert.Equal(-1, a);
        }

        [UnitTest]
        public void ushort_1()
        {
            ushort a = 1;
            Assert.Equal(1, a);
        }

        [UnitTest]
        public void short_1()
        {
            short a = 1;
            Assert.Equal(1, a);
        }

        [UnitTest]
        public void short_2()
        {
            short a = -1;
            Assert.Equal(-1, a);
        }

        [UnitTest]
        public void uint_1()
        {
            uint a = 1;
            Assert.Equal(1, a);
        }

        [UnitTest]
        public void int_1()
        {
            int a = 1;
            Assert.Equal(1, a);
        }

        [UnitTest]
        public void int_2()
        {
            int a = -1;
            Assert.Equal(-1, a);
        }


        [UnitTest]
        public void ulong_1()
        {
            ulong a = 1;
            Assert.Equal(1, a);
        }

        [UnitTest]
        public void long_1()
        {
            long a = 1;
            Assert.Equal(1, a);
        }

        [UnitTest]
        public void long_2()
        {
            long a = -1;
            Assert.Equal(-1, a);
        }

        [UnitTest]
        public void unint_1()
        {
            UIntPtr a = new UIntPtr(1);
            Assert.Equal(new UIntPtr(1), a);
        }

        [UnitTest]
        public void nint_1()
        {
            var a = new IntPtr(1);
            Assert.Equal(new IntPtr(1), a);
        }

        [UnitTest]
        public void nint_2()
        {
            var a = new IntPtr(-1);
            Assert.Equal(new IntPtr(-1), a);
        }

        [UnitTest]
        public void float_1()
        {
            float a = 1f;
            Assert.Equal(1f, a);
        }

        [UnitTest]
        public void double_1()
        {
            double a = 1.0;
            Assert.Equal(1.0, a);
        }

        [UnitTest]
        public void p_0()
        {
            int a = 1;
            int b = 2;
            int c = 3;
            int d = 4;
            Assert.Equal(1, a);
        }

        [UnitTest]
        public void p_1()
        {
            int a = 1;
            int b = 2;
            int c = 3;
            int d = 4;
            Assert.Equal(2, b);
        }

        [UnitTest]
        public void p_2()
        {
            int a = 1;
            int b = 2;
            int c = 3;
            int d = 4;
            Assert.Equal(3, c);
        }

        [UnitTest]
        public void p_3()
        {
            int a = 1;
            int b = 2;
            int c = 3;
            int d = 4;
            Assert.Equal(4, d);
        }

        [UnitTest]
        public void p_s()
        {
            int a = 1;
            int b = 2;
            int c = 3;
            int d = 4;
            int e = 5;
            Assert.Equal(5, e);
        }

        [UnitTest]
        public void s_253()
        {
            int a253 = 253;
            Assert.Equal(253, a253);
        }

        [UnitTest]
        public void p_255()
        {
            int a255 = 255;
            Assert.Equal(255, a255);
        }
    }
}
