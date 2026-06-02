using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Arrays
{
    internal class TC_ldelem_any_u8 : GeneralTestCaseBase
    {

        public static T GetEle<T>(T[] arr, int index)
        {
            return arr[index];
        }


        public static void SetEle<T>(T[] arr, int index, T value)
        {
            arr[index] = value;
        }

        [UnitTest]
        public void ld_1()
        {
            var arr = new ulong[] {1, 2};
            var x = GetEle(arr, 0);
            Assert.Equal(1, x);
            var y = GetEle(arr, 1);
            Assert.Equal(2, y);
        }

        //[UnitTest]
        //public void OutOfRange_lower()
        //{
        //    var arr = new ulong[] { 1, 2 };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = GetEle(arr, -1);
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper()
        //{
        //    var arr = new ulong[] { 1, 2 };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = GetEle(arr, 2);
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void NullRef()
        //{
        //    ulong[] arr = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    var s = GetEle(arr, 0);
        //    Assert.Fail();
        //}
    }
}
