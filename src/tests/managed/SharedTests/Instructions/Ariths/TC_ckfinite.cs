using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Ariths
{
    internal class TC_ckfinite : TestCaseBase
    {
        [UnitTest]
        public void double_finite_value()
        {
            Assert.Equal(12.5, TestInstructionOpcodes.CkfiniteDouble(12.5));
        }

        [UnitTest]
        public void double_infinity_throws()
        {
            Assert.ExpectException<ArithmeticException>(() =>
            {
                TestInstructionOpcodes.CkfiniteDouble(double.PositiveInfinity);
            });
        }

        [UnitTest]
        public void float_finite_value()
        {
            Assert.Equal(3.5f, TestInstructionOpcodes.CkfiniteFloat(3.5f));
        }

        [UnitTest]
        public void float_nan_throws()
        {
            Assert.ExpectException<ArithmeticException>(() =>
            {
                TestInstructionOpcodes.CkfiniteFloat(float.NaN);
            });
        }
    }
}
