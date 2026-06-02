using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;

namespace Tests.Instruments.Objs
{
    internal class TC_ldstr : GeneralTestCaseBase
    {

        [UnitTest]
        public void str_1()
        {
            string s = "abc";
            Assert.Equal(3, s.Length);
            Assert.Equal('a', s[0]);
            Assert.Equal('b', s[1]);
            Assert.Equal('c', s[2]);
        }



        [UnitTest]
        public void str_2()
        {
            Assert.IsTrue(CharEqual('a', 'a'));
        }

        private bool CharEqual<T>(T x, T y)
        {
            return Object.Equals(x, y);
        }
    }
}
