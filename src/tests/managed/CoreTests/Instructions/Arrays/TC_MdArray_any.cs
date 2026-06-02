
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
    internal class TC_MdArray_any : GeneralTestCaseBase
    {

        [UnitTest]
        public void NotRefStruct()
        {
            var arr = new ValueTypeSize1[2,3] { 
                { 
                    new ValueTypeSize1 { x1 = 1 },
                    new ValueTypeSize1 { x1 = 2 },
                    new ValueTypeSize1 { x1 = 3 },
                },
                { 
                    new ValueTypeSize1 { x1 = 11 },
                    new ValueTypeSize1 { x1 = 12 },
                    new ValueTypeSize1 { x1 = 13 },
                } };
            var x = arr[1, 1];
            Assert.Equal(12, x.x1);
            arr[1, 1] = new ValueTypeSize1 { x1 = 22 };
            var x2 = arr[1, 1];
            Assert.Equal(22, x2.x1);
            Assert.Equal(1, arr[0, 0].x1);
            Assert.Equal(2, arr[0, 1].x1);
            Assert.Equal(3, arr[0, 2].x1);
            Assert.Equal(11, arr[1, 0].x1);
            Assert.Equal(13, arr[1, 2].x1);
        }

        struct StructWithRef
        {
            public int x1;
            public string x2;

            static public string GetX2(StructWithRef x)
            {
                return x.x2;
            }
        }

        [UnitTest]
        public void RefStruct()
        {
            var arr = new StructWithRef[2, 3] {
                {
                    new StructWithRef { x1 = 1 },
                    new StructWithRef { x1 = 2 },
                    new StructWithRef { x1 = 3 },
                },
                {
                    new StructWithRef { x1 = 11 },
                    new StructWithRef { x1 = 12 },
                    new StructWithRef { x1 = 13 },
                } };
            var x = arr[1, 1];
            Assert.Equal(12, x.x1);
            arr[1, 1] = new StructWithRef { x1 = 22 };
            var x2 = arr[1, 1];
            Assert.Equal(22, x2.x1);
            Assert.Equal(1, arr[0, 0].x1);
            Assert.Equal(2, arr[0, 1].x1);
            Assert.Equal(3, arr[0, 2].x1);
            Assert.Equal(11, arr[1, 0].x1);
            Assert.Equal(13, arr[1, 2].x1);
        }

        //[UnitTest]
        //public void OutOfRange_lower()
        //{
        //    var arr = new ValueTypeSize1[2, 3] {
        //        {
        //            new ValueTypeSize1 { x1 = 1 },
        //            new ValueTypeSize1 { x1 = 2 },
        //            new ValueTypeSize1 { x1 = 3 },
        //        },
        //        {
        //            new ValueTypeSize1 { x1 = 11 },
        //            new ValueTypeSize1 { x1 = 12 },
        //            new ValueTypeSize1 { x1 = 13 },
        //        } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[-1, -1];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_lower2()
        //{
        //    var arr = new ValueTypeSize1[2, 3] {
        //        {
        //            new ValueTypeSize1 { x1 = 1 },
        //            new ValueTypeSize1 { x1 = 2 },
        //            new ValueTypeSize1 { x1 = 3 },
        //        },
        //        {
        //            new ValueTypeSize1 { x1 = 11 },
        //            new ValueTypeSize1 { x1 = 12 },
        //            new ValueTypeSize1 { x1 = 13 },
        //        } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[-1, 1];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_lower3()
        //{
        //    var arr = new ValueTypeSize1[2, 3] {
        //        {
        //            new ValueTypeSize1 { x1 = 1 },
        //            new ValueTypeSize1 { x1 = 2 },
        //            new ValueTypeSize1 { x1 = 3 },
        //        },
        //        {
        //            new ValueTypeSize1 { x1 = 11 },
        //            new ValueTypeSize1 { x1 = 12 },
        //            new ValueTypeSize1 { x1 = 13 },
        //        } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[1, -1];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper()
        //{
        //    var arr = new ValueTypeSize1[2, 3] {
        //        {
        //            new ValueTypeSize1 { x1 = 1 },
        //            new ValueTypeSize1 { x1 = 2 },
        //            new ValueTypeSize1 { x1 = 3 },
        //        },
        //        {
        //            new ValueTypeSize1 { x1 = 11 },
        //            new ValueTypeSize1 { x1 = 12 },
        //            new ValueTypeSize1 { x1 = 13 },
        //        } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[2, 0];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper2()
        //{
        //    var arr = new ValueTypeSize1[2, 3] {
        //        {
        //            new ValueTypeSize1 { x1 = 1 },
        //            new ValueTypeSize1 { x1 = 2 },
        //            new ValueTypeSize1 { x1 = 3 },
        //        },
        //        {
        //            new ValueTypeSize1 { x1 = 11 },
        //            new ValueTypeSize1 { x1 = 12 },
        //            new ValueTypeSize1 { x1 = 13 },
        //        } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[0, 3];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper3()
        //{
        //    var arr = new ValueTypeSize1[2, 3] {
        //        {
        //            new ValueTypeSize1 { x1 = 1 },
        //            new ValueTypeSize1 { x1 = 2 },
        //            new ValueTypeSize1 { x1 = 3 },
        //        },
        //        {
        //            new ValueTypeSize1 { x1 = 11 },
        //            new ValueTypeSize1 { x1 = 12 },
        //            new ValueTypeSize1 { x1 = 13 },
        //        } };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    var s = arr[2, 3];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void NullRef()
        //{
        //    ValueTypeSize1[,] arr = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    var s = arr[0, 0];
        //    Assert.Fail();
        //}
    }
}
