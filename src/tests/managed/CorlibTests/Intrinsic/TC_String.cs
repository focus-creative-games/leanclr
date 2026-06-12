using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Intrinsic
{
    internal class TC_String : TestCaseBase
    {
        [CoversIcall("System.String::get_Chars")]
        [CoversIcall("System.String::get_Length")]
        [UnitTest]
        public void GetChars1()
        {
            var s = "abc";
            Assert.Equal('a', s[0]);
            Assert.Equal('b', s[1]);
            Assert.Equal('c', s[2]);
        }

        [CoversIcall("System.String::get_Chars")]
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
