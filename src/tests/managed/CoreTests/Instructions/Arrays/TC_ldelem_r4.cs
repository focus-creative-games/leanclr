using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Arrays
{
    internal class TC_ldelem_r4 : GeneralTestCaseBase
    {

        [UnitTest]
        public void ld_1()
        {
            var arr = new float[] {1, 2};
            var x = arr[0];
            Assert.Equal(1f, x);
            var y = arr[1];
            Assert.Equal(2f, y);
            //var x2 = ArrayVerifyUtil.Get(arr, 0);
            //Assert.Equal(1f, x2);
            //var y2 = ArrayVerifyUtil.Get(arr, 1);
            //Assert.Equal(2f, y2);
        }

        //[UnitTest]
        //public void OutOfRange_lower()
        //{
        //    var arr = new float[] { 1, 2 };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[-1];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper()
        //{
        //    var arr = new float[] { 1, 2 };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[2];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void NullRef()
        //{
        //    float[] arr = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    var s = arr[0];
        //    Assert.Fail();
        //}
    }
}
