using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_Interop_Sys : TestCaseBase
    {
        // DoubleToString
        [UnitTest]
        public void DoubleToString1()
        {
            double d = 12345;
            string s = d.ToString();
            Assert.Equal("12345", s);
        }


        [UnitTest]
        public void DoubleToString2()
        {
            double d = 12345.6782;
            string s = d.ToString("F3");
            Assert.Equal("12345.678", s);
        }
    }
}
