using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Cmps
{
    internal class TC_Ceq : GeneralTestCaseBase
    {
        [UnitTest]
        public void int_1()
        {
            int b = 3;
            int c = 3;
            Assert.True(b == c);
        }

        [UnitTest]
        public void int_2()
        {
            int b = 3;
            int c = 4;
            Assert.False(b == c);
        }

        [UnitTest]
        public void long_1()
        {
            long b = 3;
            long c = 3;
            Assert.True(b == c);
        }

        [UnitTest]
        public void long_2()
        {
            long b = 3;
            long c = 4;
            Assert.False(b == c);
        }

        [UnitTest]
        public void nint_1()
        {
            IntPtr b = new IntPtr(3);
            IntPtr c = new IntPtr(3);
            Assert.True(b == c);
        }

        [UnitTest]
        public void nint_2()
        {
            IntPtr b = new IntPtr(3);
            IntPtr c = new IntPtr(4);
            Assert.False(b == c);
        }

        [UnitTest]
        public void float_1()
        {
            float b = 3;
            float c = 3;
            Assert.True(b == c);
        }

        [UnitTest]
        public void float_2()
        {
            float b = 3;
            float c = 4;
            Assert.False(b == c);
        }


        [UnitTest]
        public void double_1()
        {
            double b = 3;
            double c = 3;
            Assert.True(b == c);
        }

        [UnitTest]
        public void double_2()
        {
            double b = 3;
            double c = 4;
            Assert.False(b == c);
        }


        [UnitTest]
        public unsafe void ref_1()
        {
            int* b = (int*)3;
            int* c = (int*)3;
            Assert.True(b == c);
        }

        [UnitTest]
        public unsafe void ref_2()
        {
            int* b = (int*)3;
            int* c = (int*)4;
            Assert.False(b == c);
        }

        [UnitTest]
        public void object_true()
        {
            object b = "a";
            object c = b;
            Assert.True(b == c);
        }

        [UnitTest]
        public void object_false()
        {
            object b = "a";
            object c = null;
            Assert.False(b == c);
        }
    }
}
