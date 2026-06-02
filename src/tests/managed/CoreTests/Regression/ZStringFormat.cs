using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    internal class ZStringFormat : GeneralTestCaseBase
    {
        [UnitTest]
        public void Test4()
        {
            //string s = ZString.Format("{0}-{1}-{2}", "abc", 1, 2);
            //Assert.Equals(s, "abc-1-2");
        }
    }
}
