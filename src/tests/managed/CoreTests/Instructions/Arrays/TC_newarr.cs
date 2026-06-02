using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;

namespace Tests.Instruments.Arrays
{
    internal class TC_newarr : GeneralTestCaseBase
    {

        [UnitTest]
        public void new_int()
        {
            var arr = new int[2];
            Assert.Equal(2, arr.Length);
            Assert.Equal(0, arr[0]);
            Assert.Equal(0, arr[1]);
        }

        [UnitTest]
        public void new_str()
        {
            var arr = new string[2];
            Assert.Equal(2, arr.Length);
            Assert.Null(arr[0]);
            Assert.Null(arr[1]);
        }

        //[UnitTest]
        //public void overflow()
        //{
        //    Assert.ExpectException<OverflowException>();
        //    int a = -1;
        //    var arr = new string[a];
        //}
    }
}
