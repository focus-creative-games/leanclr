using test;
using System;

namespace Tests.Instruments.Cmps
{
    internal class TC_Cgt : GeneralTestCaseBase
    {
        [UnitTest]
        public void int_true()
        {
            int b = 12;
            int c = 3;
            Assert.True(b > c);
        }

        [UnitTest]
        public void int_false()
        {
            int b = 3;
            int c = 12;
            Assert.False(b > c);
        }

        [UnitTest]
        public void long_true()
        {
            long b = 12L;
            long c = 3L;
            Assert.True(b > c);
        }

        [UnitTest]
        public void long_false()
        {
            long b = 3L;
            long c = 12L;
            Assert.False(b > c);
        }

        [UnitTest]
        public unsafe void nint_true()
        {
            int* b = (int*)3;
            int* c = (int*)2;
            Assert.True(b > c);
        }

        [UnitTest]
        public unsafe void nint_false()
        {
            int* b = (int*)3;
            int* c = (int*)4;
            Assert.False(b > c);
        }

        [UnitTest]
        public void float_true()
        {
            float b = 12f;
            float c = 3f;
            Assert.True(b > c);
        }

        [UnitTest]
        public void float_false()
        {
            float b = 3f;
            float c = 12f;
            Assert.False(b > c);
        }

        [UnitTest]
        public void float_NaN()
        {
            float b = float.NaN;
            float c = 3f;
            Assert.False(b > c);
            Assert.False(c > b);
        }

        [UnitTest]
        public void float_NInf()
        {
            float b = float.NegativeInfinity;
            float c = 12f;
            Assert.False(b > c);
        }

        [UnitTest]
        public void float_PInf()
        {
            float b = 3f;
            float c = float.PositiveInfinity;
            Assert.False(b > c);
        }

        [UnitTest]
        public void double_true()
        {
            double b = 12d;
            double c = 3d;
            Assert.True(b > c);
        }

        [UnitTest]
        public void double_false()
        {
            double b = 3d;
            double c = 12d;
            Assert.False(b > c);
        }

        [UnitTest]
        public void double_NaN()
        {
            double b = double.NaN;
            double c = 3d;
            Assert.False(b > c);
            Assert.False(c > b);
        }

        [UnitTest]
        public void double_NInf()
        {
            double b = double.NegativeInfinity;
            double c = 12d;
            Assert.False(b > c);
        }

        [UnitTest]
        public void double_PInf()
        {
            double b = 3d;
            double c = double.PositiveInfinity;
            Assert.False(b > c);
        }
    }
}
