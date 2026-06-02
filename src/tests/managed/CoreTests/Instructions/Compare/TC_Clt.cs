using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Cmps
{
    internal class TC_Clt : GeneralTestCaseBase
    {
        [UnitTest]
        public void int_true()
        {
            int b = 2;
            int c = 3;
            Assert.True(b < c);
        }

        [UnitTest]
        public void int_false()
        {
            int b = 4;
            int c = 3;
            Assert.False(b < c);
        }

        [UnitTest]
        public void long_true()
        {
            long b = 3L;
            long c = 4L;
            Assert.True(b < c);
        }

        [UnitTest]
        public void long_false()
        {
            long b = 4L;
            long c = 3L;
            Assert.False(b < c);
        }

        [UnitTest]
        public unsafe void nint_true()
        {
            int* b = (int*)2;
            int* c = (int*)3;
            Assert.True(b < c);
        }

        [UnitTest]
        public unsafe void nint_false()
        {
            int* b = (int*)4;
            int* c = (int*)3;
            Assert.False(b < c);
        }

        [UnitTest]
        public void float_true()
        {
            float b = 3f;
            float c = 12f;
            Assert.True(b < c);
        }

        [UnitTest]
        public void float_false()
        {
            float b = 12f;
            float c = 3f;
            Assert.False(b < c);
        }

        [UnitTest]
        public void float_false_1()
        {
            float b = float.NaN;
            float c = 3f;
            Assert.False(b < c);
        }

        [UnitTest]
        public void float_false_2()
        {
            float b = 12f;
            float c = float.NaN;
            Assert.False(b < c);
        }

        [UnitTest]
        public void float_false_3()
        {
            float b = float.NaN;
            float c = float.NaN;
            Assert.False(b < c);
        }

        [UnitTest]
        public void float_false_4()
        {
            float b = float.PositiveInfinity;
            float c = float.NegativeInfinity;
            Assert.False(b < c);
        }

        [UnitTest]
        public void float_false_5()
        {
            float b = 1f;
            float c = float.NegativeInfinity;
            Assert.False(b < c);
        }

        [UnitTest]
        public void float_double_false()
        {
            float b = 12f;
            double c = 3d;
            Assert.False(b < c);
        }

        [UnitTest]
        public void double_true()
        {
            double b = 3d;
            double c = 12d;
            Assert.True(b < c);
        }

        [UnitTest]
        public void double_false()
        {
            double b = 12d;
            double c = 3d;
            Assert.False(b < c);
        }

        [UnitTest]
        public void double_false_1()
        {
            double b = double.NaN;
            double c = 3d;
            Assert.False(b < c);
        }

        [UnitTest]
        public void double_false_2()
        {
            double b = 12d;
            double c = double.NaN;
            Assert.False(b < c);
        }

        [UnitTest]
        public void double_false_3()
        {
            double b = double.NaN;
            double c = double.NaN;
            Assert.False(b < c);
        }

        [UnitTest]
        public void double_false_4()
        {
            double b = double.PositiveInfinity;
            double c = double.NegativeInfinity;
            Assert.False(b < c);
        }

        [UnitTest]
        public void double_false_5()
        {
            double b = 1d;
            double c = double.NegativeInfinity;
            Assert.False(b < c);
        }
    }
}
