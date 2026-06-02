using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Arrays
{
    internal class TC_stelem_i : GeneralTestCaseBase
    {

        [UnitTest]
        public void st_1()
        {
            var arr = new IntPtr[2];
            arr[0] = (IntPtr)1;
            arr[1] = (IntPtr)2;

            var x = arr[0];
            Assert.Equal((IntPtr)1, x);
            var y = arr[1];
            Assert.Equal((IntPtr)2, y);
        }

        //[UnitTest]
        //public void OutOfRange_lower()
        //{
        //    var arr = new IntPtr[2];
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    arr[-1] = (IntPtr)1;
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper()
        //{
        //    var arr = new IntPtr[2];
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    arr[2] = (IntPtr)1;
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void NullRef()
        //{
        //    IntPtr[] arr = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    arr[0] = (IntPtr)1;
        //    Assert.Fail();
        //}
    }
}
