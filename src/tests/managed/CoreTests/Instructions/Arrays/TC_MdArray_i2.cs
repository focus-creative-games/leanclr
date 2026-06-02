
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;

namespace Tests.Instruments.Arrays
{
    internal class TC_MdArray_i2 : GeneralTestCaseBase
    {

        [UnitTest]
        public void ld_1()
        {
            var arr = new short[2,3] { { 1, 2, 3 }, { 11, 12, 13 } };
            var x = arr[1, 1];
            Assert.Equal(12, x);
            arr[1, 1] = 22;
            var x2 = arr[1, 1];
            Assert.Equal(22, x2);
            Assert.Equal(1, arr[0, 0]);
            Assert.Equal(2, arr[0, 1]);
            Assert.Equal(3, arr[0, 2]);
            Assert.Equal(11, arr[1, 0]);
            Assert.Equal(13, arr[1, 2]);
        }

        //[UnitTest]
        //public void OutOfRange_lower()
        //{
        //    var arr = new short[2, 3] { { 1, 2, 3 }, { 11, 12, 13 } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[-1, -1];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_lower2()
        //{
        //    var arr = new short[2, 3] { { 1, 2, 3 }, { 11, 12, 13 } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[-1, 1];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_lower3()
        //{
        //    var arr = new short[2, 3] { { 1, 2, 3 }, { 11, 12, 13 } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[1, -1];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper()
        //{
        //    var arr = new short[2, 3] { { 1, 2, 3 }, { 11, 12, 13 } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[2, 0];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper2()
        //{
        //    var arr = new short[2, 3] { { 1, 2, 3 }, { 11, 12, 13 } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[0, 3];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper3()
        //{
        //    var arr = new short[2, 3] { { 1, 2, 3 }, { 11, 12, 13 } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[2, 3];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void NullRef()
        //{
        //    short[,] arr = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    var s = arr[0, 0];
        //    Assert.Fail();
        //}
    }
}
