using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    internal class Bug_2022_9_22 : GeneralTestCaseBase
    {
        /// <summary>
        /// i2与u2的桥接签名并不等效，在release编译情况下，如果将u2的参数传递给int变量，
        /// arm32 abi下，由于abi要求char无符号扩展到int，导致int变量直接使用了参数的值，
        /// 而未进行无符号扩展，导致以i2形式传传u2变量时，出现负数的情况
        /// </summary>
        [UnitTest]
        public void arg_char()
        {
            char c = '陈';
            bool r = char.IsWhiteSpace(c);
            Assert.False(r);
        }
    }
}
