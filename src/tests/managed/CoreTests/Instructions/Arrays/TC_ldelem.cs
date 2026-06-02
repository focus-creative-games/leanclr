
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using AOTDefs;

namespace Tests.Instruments.Arrays
{
    internal class TC_ldelem : GeneralTestCaseBase
    {

        [UnitTest]
        public void ld_1()
        {
            var arr = new ValueTypeSize1[] { new ValueTypeSize1 { x1 = 1 }, new ValueTypeSize1 { x1 = 2 } };

            var x = arr[0];
            Assert.Equal(1, x.x1);
            var y = arr[1];
            Assert.Equal(2, y.x1);
        }


        [UnitTest]
        public void ld_4()
        {
            var arr = new ValueTypeSize4[] { new ValueTypeSize4 { x1 = 1 }, new ValueTypeSize4 { x1 = 2 } };

            var x = arr[0];
            Assert.Equal(1, x.x1);
            var y = arr[1];
            Assert.Equal(2, y.x1);

        }


        [UnitTest]
        public void ld_8()
        {
            var arr = new ValueTypeSize8[] { new ValueTypeSize8 { x1 = 1 }, new ValueTypeSize8 { x1 = 2 } };

            var x = arr[0];
            Assert.Equal(1, x.x1);
            var y = arr[1];
            Assert.Equal(2, y.x1);

        }


        [UnitTest]
        public void ld_9()
        {
            var arr = new ValueTypeSize9[] { new ValueTypeSize9 { x1 = 1 }, new ValueTypeSize9 { x1 = 2 } };

            var x = arr[0];
            Assert.Equal(1, x.x1);
            var y = arr[1];
            Assert.Equal(2, y.x1);

        }

        [UnitTest]
        public void ld_16()
        {
            var arr = new ValueTypeSize16[] { new ValueTypeSize16 { x1 = 1 }, new ValueTypeSize16 { x1 = 2 } };

            var x = arr[0];
            Assert.Equal(1, x.x1);
            var y = arr[1];
            Assert.Equal(2, y.x1);

        }

        [UnitTest]
        public void ld_20()
        {
            var arr = new ValueTypeSize20[] { new ValueTypeSize20 { x1 = 1 }, new ValueTypeSize20 { x1 = 2 } };

            var x = arr[0];
            Assert.Equal(1, x.x1);
            var y = arr[1];
            Assert.Equal(2, y.x1);

        }

        //[UnitTest]
        //public void OutOfRange_lower()
        //{
        //    var arr = new ValueTypeSize4[] { new ValueTypeSize4 { x1 = 1 }, new ValueTypeSize4 { x1 = 2 } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[-1];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper()
        //{
        //    var arr = new ValueTypeSize4[] { new ValueTypeSize4 { x1 = 1 }, new ValueTypeSize4 { x1 = 2 } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[2];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void NullRef()
        //{
        //    ValueTypeSize4[] arr = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    var s = arr[0];
        //    Assert.Fail();
        //}
    }
}
