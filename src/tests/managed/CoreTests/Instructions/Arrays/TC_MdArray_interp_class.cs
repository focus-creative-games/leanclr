
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;

namespace Tests.Instruments.Arrays
{
    internal class TC_MdArray_interp_class : GeneralTestCaseBase
    {
        class A
        {
            public int x;

            public A(int x)
            {
                this.x = x;
            }
        }

        [UnitTest]
        public void ld_1()
        {
            var arr = new A[2,3] { { new A(1), new A(2), new A(3) }, { new A(11), new A(12), new A(13) } };
            var x = arr[1, 1];
            Assert.Equal(12, x.x);
            arr[1, 1].x = 22;
            var x2 = arr[1, 1];
            Assert.Equal(22, x2.x);
            Assert.Equal(1, arr[0, 0].x);
            Assert.Equal(2, arr[0, 1].x);
            Assert.Equal(3, arr[0, 2].x);
            Assert.Equal(11, arr[1, 0].x);
            Assert.Equal(13, arr[1, 2].x);
        }



        [UnitTest]
        public void address()
        {
            var arr = new A[2, 3] { { new A(1), new A(2), new A(3) }, { new A(11), new A(12), new A(13) } };
            ref A x = ref arr[1, 1];
            Assert.Equal(12, x.x);
            x = new A(22);
            var x2 = arr[1, 1];
            Assert.Equal(22, x2.x);
            Assert.Equal(1, arr[0, 0].x);
            Assert.Equal(2, arr[0, 1].x);
            Assert.Equal(3, arr[0, 2].x);
            Assert.Equal(11, arr[1, 0].x);
            Assert.Equal(13, arr[1, 2].x);
        }

        //[UnitTest]
        //public void OutOfRange_lower()
        //{
        //    var arr = new A[2, 3] { { new A(1), new A(2), new A(3) }, { new A(11), new A(12), new A(13) } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[-1, -1];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_lower2()
        //{
        //    var arr = new A[2, 3] { { new A(1), new A(2), new A(3) }, { new A(11), new A(12), new A(13) } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[-1, 1];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_lower3()
        //{
        //    var arr = new A[2, 3] { { new A(1), new A(2), new A(3) }, { new A(11), new A(12), new A(13) } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[1, -1];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper()
        //{
        //    var arr = new A[2, 3] { { new A(1), new A(2), new A(3) }, { new A(11), new A(12), new A(13) } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[2, 0];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper2()
        //{
        //    var arr = new A[2, 3] { { new A(1), new A(2), new A(3) }, { new A(11), new A(12), new A(13) } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[0, 3];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper3()
        //{
        //    var arr = new A[2, 3] { { new A(1), new A(2), new A(3) }, { new A(11), new A(12), new A(13) } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[2, 3];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void NullRef()
        //{
        //    A[,] arr = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    var s = arr[0, 0];
        //    Assert.Fail();
        //}
    }
}
