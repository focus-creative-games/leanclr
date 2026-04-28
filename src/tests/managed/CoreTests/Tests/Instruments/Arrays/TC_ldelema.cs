using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Arrays
{
    internal class TC_ldelema : GeneralTestCaseBase
    {

        [UnitTest]
        public void ld_1()
        {
            var arr = new int[] { -1, 1 };
            ref int x = ref arr[0];
            Assert.Equal(-1, x);
            ref int y = ref arr[1];
            Assert.Equal(1, y);
        }

        //[UnitTest]
        //public void OutOfRange_lower()
        //{
        //    var arr = new int[] { -1, 1 };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    ref int s = ref arr[-1];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void OutOfRange_upper()
        //{
        //    var arr = new int[] { -1, 1 };
        //    Assert.ExpectException<IndexOutOfRangeException>();
        //    ref int s = ref arr[2];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void NullRef()
        //{
        //    int[] arr = null;
        //    Assert.ExpectException<NullReferenceException>();
        //    ref int s = ref arr[0];
        //    Assert.Fail();
        //}

        //[UnitTest]
        //public void LoadCovariantElement()
        //{
        //    Assert.ExpectException<System.ArrayTypeMismatchException>();
        //    var arr = new string[] { "a", "b" };
        //    object[] arr2 = arr;
        //    ref object s = ref arr2[0];
        //    s = new object();
        //}


        interface ITest
        {
            int Test();
        }

        static int CallArrayMemberFunction<T>(T[] arr) where T: ITest
        {
            return arr[0].Test();
        }

        struct TestStruct : ITest
        {
            public int x;

            public int Test()
            {
                return x;
            }
        }

        class TestClass : ITest
        {
            public int x;

            public int Test()
            {
                return x;
            }
        }

        [UnitTest]
        public void ReadOnlyPrefixStructArray()
        {
            TestStruct[] arr = new TestStruct[] { new TestStruct { x = 1 } };
            Assert.Equal(1, CallArrayMemberFunction(arr));
        }

        [UnitTest]
        public void ReadOnlyPrefixStructClass()
        {
            TestClass[] arr = new TestClass[] { new TestClass { x = 1 } };
            ITest[] arr2 = arr;
            Assert.Equal(1, CallArrayMemberFunction(arr2));
        }

        //[UnitTest]
        //public void UncheckedForStructArray1()
        //{
        //    int[] arr = new int[1] { 1 };
        //    uint[] arr2 = System.Runtime.com<int[], uint[]>(ref arr);
        //    ref int x = ref arr[0];
        //    Assert.Equal(1, x);
        //}

        //[UnitTest]
        //public void UncheckedForStructArray2()
        //{
        //    TestStruct2[] arr = new TestStruct2[1] { new TestStruct2 { x = 1 } };
        //    TestStruct[] arr2 = UnsafeUtility.As<TestStruct2[], TestStruct[]>(ref arr);
        //    ref TestStruct x = ref arr2[0];
        //    Assert.Equal(1, x.x);
        //}
    }
}
