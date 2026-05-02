using System;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Math : GeneralTestCaseBase
    {
        private static void AssertNearlyEqual(double expected, double actual, double epsilon = 1e-12)
        {
            if (double.IsNaN(expected) || double.IsNaN(actual))
            {
                Assert.Equal(expected, actual);
                return;
            }
            if (double.IsInfinity(expected) || double.IsInfinity(actual))
            {
                Assert.Equal(expected, actual);
                return;
            }
            Assert.IsTrue(Math.Abs(expected - actual) <= epsilon);
        }

        [UnitTest]
        public void Round1()
        {
            Assert.Equal(1.0, Math.Round(1.2));
        }

        [UnitTest]
        public void RoundAwayFromZero()
        {
            Assert.Equal(2.0, Math.Round(1.51, 0, MidpointRounding.AwayFromZero));
        }

        [UnitTest]
        public void RoundToEvent()
        {
            Assert.Equal(4.0, Math.Round(3.5, 0, MidpointRounding.ToEven));
        }

        [UnitTest]
        public void AbsF321()
        {
            float a = -5.5f;
            Assert.Equal(5.5, Math.Abs(a));
        }

        [UnitTest]
        public void AbsF641()
        {
            double a = -5.5;
            Assert.Equal(5.5, Math.Abs(a));
        }

        [UnitTest]
        public void Acos1()
        {
            Assert.Equal(0.0, Math.Acos(1.0));
        }

        [UnitTest]
        public void Acosh1()
        {
            Assert.Equal(0.0, Math.Acosh(1.0));
        }

        [UnitTest]
        public void Asin1()
        {
            Assert.Equal(0.0, Math.Asin(0.0));
        }

        [UnitTest]
        public void Asinh1()
        {
            Assert.Equal(0.0, Math.Asinh(0.0));
        }

        [UnitTest]
        public void Atan1()
        {
            Assert.Equal(0.0, Math.Atan(0.0));
        }

        [UnitTest]
        public void Atan21()
        {
            Assert.Equal(0.0, Math.Atan2(0.0, 1.0));
        }

        [UnitTest]
        public void Atanh1()
        {
            Assert.Equal(0.0, Math.Atanh(0.0));
        }

        [UnitTest]
        public void Cbrt1()
        {
            AssertNearlyEqual(2.0, Math.Cbrt(8.0));
        }

        [UnitTest]
        public void Ceiling()
        {
            Assert.Equal(2.0, Math.Ceiling(1.2));
        }

        [UnitTest]
        public void Cos1()
        {
            Assert.Equal(1.0, Math.Cos(0.0));
        }

        [UnitTest]
        public void Cosh1()
        {
            Assert.Equal(1.0, Math.Cosh(0.0));
        }

        [UnitTest]
        public void Exp1()
        {
            Assert.Equal(1.0, Math.Exp(0.0));
        }

        [UnitTest]
        public void Floor1()
        {
            Assert.Equal(1.0, Math.Floor(1.8));
        }

        [UnitTest]
        public void Truncate1()
        {
            Assert.Equal(1.0, Math.Truncate(1.8));
            Assert.Equal(-1.0, Math.Truncate(-1.8));
        }

        [UnitTest]
        public void Log1()
        {
            Assert.Equal(0.0, Math.Log(1.0));
        }

        [UnitTest]
        public void Log101()
        {
            AssertNearlyEqual(2.0, Math.Log10(100.0));
        }

        [UnitTest]
        public void Pow1()
        {
            Assert.Equal(8.0, Math.Pow(2.0, 3.0));
        }

        [UnitTest]
        public void Sin1()
        {
            Assert.Equal(0.0, Math.Sin(0.0));
        }

        [UnitTest]
        public void Sinh1()
        {
            Assert.Equal(0.0, Math.Sinh(0.0));
        }

        [UnitTest]
        public void Sqrt1()
        {
            Assert.Equal(3.0, Math.Sqrt(9.0));
        }

        [UnitTest]
        public void Tan1()
        {
            Assert.Equal(0.0, Math.Tan(0.0));
        }

        [UnitTest]
        public void Tanh1()
        {
            Assert.Equal(0.0, Math.Tanh(0.0));
        }

        // [UnitTest]
        // public void FMod1()
        // {
        //     AssertNearlyEqual(1.0, Math.FMod(7.0, 3.0));
        // }

        // [UnitTest]
        // public void ModF1()
        // {
        //     double intPart;
        //     double frac = Math.ModF(3.75, out intPart);
        //     AssertNearlyEqual(0.75, frac);
        //     AssertNearlyEqual(3.0, intPart);
        // }
    }
}
