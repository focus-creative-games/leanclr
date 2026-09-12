using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Arrays
{
    internal class TC_ldelem_ref : TestCaseBase
    {

        [UnitTest]
        public void string_1()
        {
            var arr = new string[] {"1", "2", "3"};
            var s = arr[1];
            Assert.Equal("2", s);
            var x2 = ArrayVerifyUtil.Get(arr, 1);
            Assert.Equal("2", x2);
        }

        [UnitTest]
        public void string_OutOfRange_lower()
        {
            var arr = new string[] { "1", "2", "3" };
            Assert.ExpectException<IndexOutOfRangeException>(() =>
            {
            var s = arr[-1];
            });
        }

        [UnitTest]
        public void string_OutOfRange_upper()
        {
            var arr = new string[] { "1", "2", "3" };
            Assert.ExpectException<IndexOutOfRangeException>(() =>
            {
            var s = arr[3];
            });
        }

        [UnitTest]
        public void string_NullRef()
        {
            string[] arr = null;
            Assert.ExpectException<NullReferenceException>(() =>
            {
            var s = arr[0];
            });
        }
    }
}
