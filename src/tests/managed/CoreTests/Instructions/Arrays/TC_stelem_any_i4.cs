using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Arrays
{
    internal class TC_stelem_any_i4 : GeneralTestCaseBase
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
        public void st_1()
        {
            var arr = new int[3] {1, 1, 1};
            SetEle(arr, 1, -1);

            var x = GetEle(arr, 0);
            Assert.Equal(1, x);
            var y = GetEle(arr, 1);
            Assert.Equal(-1, y);
            var z = GetEle(arr, 2);
            Assert.Equal(1, z);
        }

        //[UnitTest]
        //public void OutOfRange_lower()
        //{
        //    var arr = new int[2];
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    SetEle(arr, -1, 1);
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper()
        //{
        //    var arr = new int[2];
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    SetEle(arr, 2, 1);
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void NullRef()
        //{
        //    int[] arr = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    SetEle(arr, 0, 1);
        //    Assert.Fail();
        //}
    }
}
