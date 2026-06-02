using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Arrays
{
    internal class TC_stelem_i4 : GeneralTestCaseBase
    {

        [UnitTest]
        public void st_1()
        {
            var arr = new int[2];
            arr[0] = -1;
            arr[1] = 1;

            var x = arr[0];
            Assert.Equal(-1, x);
            var y = arr[1];
            Assert.Equal(1, y);
        }

        //[UnitTest]
        //public void OutOfRange_lower()
        //{
        //    var arr = new int[2];
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    int i = -1;
        //    arr[i] = 1;
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper()
        //{
        //    var arr = new int[2];
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    arr[2] = 1;
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void NullRef()
        //{
        //    int[] arr = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    arr[0] = 1;
        //    Assert.Fail();
        //}
    }
}
