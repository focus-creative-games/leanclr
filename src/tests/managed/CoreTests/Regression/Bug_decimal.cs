using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    internal class Bug_decimal : GeneralTestCaseBase
    {
        [UnitTest]
        public void RoundOfDecimal()
        {
            decimal x = 30m;
            decimal y = Math.Round(x, 2, MidpointRounding.AwayFromZero);
            Assert.Equal(30m, y);
        }
    }
}
