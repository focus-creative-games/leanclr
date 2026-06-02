using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Ariths
{
    internal class TC_neg : GeneralTestCaseBase
    {
        [UnitTest]
        public void sbyte_1()
        {
            sbyte b = 12;
            Assert.Equal(-12, -b);
        }

        [UnitTest]
        public void byte_1()
        {
            byte b = 12;
            Assert.Equal(-12, -b);
        }

        [UnitTest]
        public void short_1()
        {
            short b = 12;
            Assert.Equal(-12, -b);
        }

        [UnitTest]
        public void ushort_1()
        {
            ushort b = 12;
            Assert.Equal(-12, -b);
        }

        [UnitTest]
        public void int_1()
        {
            int b = 12;
            Assert.Equal(-12, -b);
        }

        [UnitTest]
        public void long_1()
        {
            long b = 1;
            Assert.Equal(-1L, -b);
        }

        [UnitTest]
        public void float_1()
        {
            float b = 3f;
            Assert.Equal(-3f, -b);
        }

        [UnitTest]
        public void float_inf()
        {
            float b = float.NegativeInfinity;
            Assert.True(float.IsInfinity(-b));
        }

        [UnitTest]
        public void float_NaN()
        {
            float b = float.NaN;
            Assert.True(float.IsNaN(-b));
        }

        [UnitTest]
        public void double_1()
        {
            double b = 3.0;
            Assert.Equal(-3.0, -b);
        }

        [UnitTest]
        public void double_inf()
        {
            double b = double.NegativeInfinity;
            Assert.Equal(double.PositiveInfinity, -b);
        }

        [UnitTest]
        public void double_NaN()
        {
            double b = double.NaN;
            Assert.True(double.IsNaN(-b));
        }
    }
}
