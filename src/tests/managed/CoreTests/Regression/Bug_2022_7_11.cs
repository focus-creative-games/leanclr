using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{

    internal class Bug_2022_7_11 : GeneralTestCaseBase
    {

        [UnitTest]
        public void test7()
        {
            Action<string> mOnValueChanged = v => { };
            Assert.NotNull(mOnValueChanged);
        }
    }
}
