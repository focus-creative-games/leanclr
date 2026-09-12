using System;

namespace Tests.Instruments.Ariths
{
    internal class TC_sub_ovf : TestCaseBase
    {
        [UnitTest]
        public void int_in_range()
        {
            int a = 100;
            int b = 23;
            Assert.Equal(77, checked(a - b));
        }

        [UnitTest]
        public void int_overflow_positive()
        {
            int a = int.MaxValue;
            Assert.ExpectException<OverflowException>(() =>
            {
                int _ = checked(a - -1);
            });
        }

        [UnitTest]
        public void int_overflow_negative()
        {
            int a = int.MinValue;
            Assert.ExpectException<OverflowException>(() =>
            {
                int _ = checked(a - 1);
            });
        }

        [UnitTest]
        public void long_overflow_negative()
        {
            long a = long.MinValue;
            Assert.ExpectException<OverflowException>(() =>
            {
                long _ = checked(a - 1L);
            });
        }
    }
}
