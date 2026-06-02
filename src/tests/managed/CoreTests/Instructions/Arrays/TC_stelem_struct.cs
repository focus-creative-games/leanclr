
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;
using AOTDefs;

namespace Tests.Instruments.Arrays
{
    internal class TC_stelem_struct : GeneralTestCaseBase
    {

        [UnitTest]
        public void st_1()
        {
            var arr = new ValueTypeSize4[2];
            arr[0] = new ValueTypeSize4 { x1 = 1 };
            arr[1] = new ValueTypeSize4 { x1 = 2 };
            var x = arr[0];
            Assert.Equal(1, x.x1);
            var y = arr[1];
            Assert.Equal(2, y.x1);
        }

        //[UnitTest]
        //public void OutOfRange_lower()
        //{
        //    var arr = new ValueTypeSize4[2];
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    int i = -1;
        //    arr[i] = new ValueTypeSize4 { x1 = 1 };
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper()
        //{
        //    var arr = new ValueTypeSize4[2];
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    arr[2] = new ValueTypeSize4();
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void NullRef()
        //{
        //    ValueTypeSize4[] arr = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    arr[0] = new ValueTypeSize4();
        //    Assert.Fail();
        //}
    }
}
