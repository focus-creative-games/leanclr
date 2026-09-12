using System;

namespace Tests.Instruments.Ariths
{
    internal class TC_mul_ovf : TestCaseBase
    {
        [UnitTest]
        public void int_in_range()
        {
            int a = 12;
            int b = 11;
            Assert.Equal(132, checked(a * b));
        }

        [UnitTest]
        public void int_overflow_positive()
        {
            int a = int.MaxValue;
            Assert.ExpectException<OverflowException>(() =>
            {
                int _ = checked(a * 2);
            });
        }

        [UnitTest]
        public void int_overflow_negative()
        {
            int a = int.MinValue;
            Assert.ExpectException<OverflowException>(() =>
            {
                int _ = checked(a * -1);
            });
        }

        [UnitTest]
        public void long_overflow_positive()
        {
            long a = long.MaxValue;
            Assert.ExpectException<OverflowException>(() =>
            {
                long _ = checked(a * 2L);
            });
        }
    }
}
