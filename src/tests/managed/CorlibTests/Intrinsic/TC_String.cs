using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Intrinsic
{
    internal class TC_String : TestCaseBase
    {
        [UnitTest]
        public void GetChars1()
        {
            var s = "abc";
            Assert.Equal('a', s[0]);
            Assert.Equal('b', s[1]);
            Assert.Equal('c', s[2]);
        }

        [UnitTest]
        public unsafe void GetStringDataOffset()
        {
            var s = "abc";
            fixed(char* ptr = s)
            {
                Assert.Equal('a', *ptr);
            }
        }
    }
}
