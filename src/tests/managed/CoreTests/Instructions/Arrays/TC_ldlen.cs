using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;

namespace Tests.Instruments.Arrays
{
    internal class TC_ldlen : GeneralTestCaseBase
    {

        [UnitTest]
        public void len_empty()
        {
            var arr = new int[0];
            Assert.Equal(0, arr.Length);
            Assert.Equal(0, arr.LongLength);
        }

        [UnitTest]
        public void len_size2()
        {
            var arr = new int[2];
            Assert.Equal(2, arr.Length);
            Assert.Equal(2, arr.LongLength);
        }

        //[UnitTest]
        //public void nullobj()
        //{
        //    int[] arr = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    int n = arr.Length;
        //    Assert.Fail();
        //}
    }
}
