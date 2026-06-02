using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.CSharp
{
    internal class TC_foreach : GeneralTestCaseBase
    {
        [UnitTest]
        public void sum()
        {
            int[] arr = new int[] { 1, 2, 3, 4 };
            int sum = 0;
            foreach(int i in arr)
            {
                sum += i;
            }
            Assert.Equal(10, sum);
        }
    }
}
