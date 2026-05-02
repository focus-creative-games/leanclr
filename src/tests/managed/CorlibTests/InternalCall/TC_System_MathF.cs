using System;

namespace CorlibTests.InternalCall
{
    internal class TC_System_MathF : GeneralTestCaseBase
    {
        private static void AssertNearlyEqual(float expected, float actual, float epsilon = 1e-6f)
        {
            if (float.IsNaN(expected) || float.IsNaN(actual))
            {
                Assert.Equal(expected, actual);
                return;
            }
            if (float.IsInfinity(expected) || float.IsInfinity(actual))
            {
                Assert.Equal(expected, actual);
                return;
            }
            Assert.IsTrue(MathF.Abs(expected - actual) <= epsilon);
        }

        [UnitTest]
        public void Acos1()
        {
            Assert.Equal(0f, MathF.Acos(1f));
        }

        [UnitTest]
        public void Acosh1()
        {
            Assert.Equal(0f, MathF.Acosh(1f));
        }

        [UnitTest]
        public void Asin1()
        {
            Assert.Equal(0f, MathF.Asin(0f));
        }

        [UnitTest]
        public void Asinh1()
        {
            Assert.Equal(0f, MathF.Asinh(0f));
        }

        [UnitTest]
        public void Atan1()
        {
            Assert.Equal(0f, MathF.Atan(0f));
        }

        [UnitTest]
        public void Atan21()
        {
            Assert.Equal(0f, MathF.Atan2(0f, 1f));
        }

        [UnitTest]
        public void Atanh1()
        {
            Assert.Equal(0f, MathF.Atanh(0f));
        }

        [UnitTest]
        public void Cbrt1()
        {
            AssertNearlyEqual(2f, MathF.Cbrt(8f));
        }

        [UnitTest]
        public void Ceiling1()
        {
            Assert.Equal(2f, MathF.Ceiling(1.2f));
        }

        [UnitTest]
        public void Cos1()
        {
            Assert.Equal(1f, MathF.Cos(0f));
        }

        [UnitTest]
        public void Cosh1()
        {
            Assert.Equal(1f, MathF.Cosh(0f));
        }

        [UnitTest]
        public void Exp1()
        {
            Assert.Equal(1f, MathF.Exp(0f));
        }

        [UnitTest]
        public void Floor1()
        {
            Assert.Equal(1f, MathF.Floor(1.8f));
        }

        [UnitTest]
        public void Round1()
        {
            Assert.Equal(2f, MathF.Round(1.6f));
        }

        [UnitTest]
        public void RoundAwayFromZero()
        {
            Assert.Equal(3f, MathF.Round(2.51f, 0, MidpointRounding.AwayFromZero));
        }

        [UnitTest]
        public void RoundToEvent()
        {
            Assert.Equal(4f, MathF.Round(3.5f, 0, MidpointRounding.ToEven));
        }

        [UnitTest]
        public void Truncate1()
        {
            Assert.Equal(1f, MathF.Truncate(1.8f));
            Assert.Equal(-1f, MathF.Truncate(-1.8f));
        }

        [UnitTest]
        public void Log1()
        {
            Assert.Equal(0f, MathF.Log(1f));
        }

        [UnitTest]
        public void Log101()
        {
            AssertNearlyEqual(2f, MathF.Log10(100f));
        }

        [UnitTest]
        public void Pow1()
        {
            Assert.Equal(8f, MathF.Pow(2f, 3f));
        }

        [UnitTest]
        public void Sin1()
        {
            Assert.Equal(0f, MathF.Sin(0f));
        }

        [UnitTest]
        public void Sinh1()
        {
            Assert.Equal(0f, MathF.Sinh(0f));
        }

        [UnitTest]
        public void Sqrt1()
        {
            Assert.Equal(3f, MathF.Sqrt(9f));
        }

        [UnitTest]
        public void Tan1()
        {
            Assert.Equal(0f, MathF.Tan(0f));
        }

        [UnitTest]
        public void Tanh1()
        {
            Assert.Equal(0f, MathF.Tanh(0f));
        }

        // [UnitTest]
        // public void FMod1()
        // {
        //     AssertNearlyEqual(1f, MathF.FMod(7f, 3f));
        // }

        // [UnitTest]
        // public void ModF1()
        // {
        //     float intPart;
        //     float frac = MathF.ModF(3.75f, out intPart);
        //     AssertNearlyEqual(0.75f, frac);
        //     AssertNearlyEqual(3f, intPart);
        // }
    }
}
