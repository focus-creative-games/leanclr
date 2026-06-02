using System;

namespace Tests.Instructions.Converts
{
    class TC_conv_ovf_il : GeneralTestCaseBase
    {
        [UnitTest]
        public void conv_ovf_i1_in_range()
        {
            Assert.Equal(127, TestConvOvf.TestConvOvfI1(127));
        }

        [UnitTest]
        public void conv_ovf_i1_negative_in_range()
        {
            Assert.Equal(-128, TestConvOvf.TestConvOvfI1(-128));
        }

        [UnitTest]
        public void conv_ovf_i1_overflow_positive()
        {
            Assert.ExpectException<OverflowException>(() => TestConvOvf.TestConvOvfI1(128));
        }

        [UnitTest]
        public void conv_ovf_i1_overflow_negative()
        {
            Assert.ExpectException<OverflowException>(() => TestConvOvf.TestConvOvfI1(-129));
        }

        [UnitTest]
        public void conv_ovf_u1_un_in_range()
        {
            Assert.Equal(255, TestConvOvf.TestConvOvfU1Un(255));
        }

        [UnitTest]
        public void conv_ovf_u1_un_zero()
        {
            Assert.Equal(0, TestConvOvf.TestConvOvfU1Un(0));
        }

        [UnitTest]
        public void conv_ovf_u1_un_overflow()
        {
            Assert.ExpectException<OverflowException>(() => TestConvOvf.TestConvOvfU1Un(256));
        }

        [UnitTest]
        public void conv_ovf_u1_un_negative_input()
        {
            Assert.ExpectException<OverflowException>(() => TestConvOvf.TestConvOvfU1Un(-1));
        }
    }
}
