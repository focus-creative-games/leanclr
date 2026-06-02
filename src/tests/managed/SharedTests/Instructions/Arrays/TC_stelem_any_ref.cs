
using System;

namespace Tests.Instruments.Arrays
{
    internal class TC_stelem_any_ref : GeneralTestCaseBase
    {
        class Animal { }
        class Dog : Animal { }
        class Cat : Animal { }

        public static T GetEle<T>(T[] arr, int index)
        {
            return arr[index];
        }

        public static void SetEle<T>(T[] arr, int index, T value)
        {
            arr[index] = value;
        }

        [UnitTest]
        public void st_derived_to_base_array()
        {
            // Store derived-type objects into a base-type array.
            // Regression: is_assignable_from parameter order was reversed in StelemAnyRef,
            // causing this valid assignment to throw ArrayTypeMismatchException.
            var arr = new Animal[2];
            SetEle<Animal>(arr, 0, new Dog());
            SetEle<Animal>(arr, 1, new Cat());

            var x = GetEle<Animal>(arr, 0);
            Assert.IsTrue(x is Dog);
            var y = GetEle<Animal>(arr, 1);
            Assert.IsTrue(y is Cat);
        }

        [UnitTest]
        public void st_to_object_array()
        {
            // Store various derived types into object[].
            // With the bug, any non-object type stored via stelem.any would throw.
            var arr = new object[3];
            SetEle<object>(arr, 0, "hello");
            SetEle<object>(arr, 1, new Dog());
            SetEle<object>(arr, 2, 42);

            Assert.Equal("hello", (string)GetEle<object>(arr, 0));
            Assert.IsTrue(GetEle<object>(arr, 1) is Dog);
            Assert.Equal(42, (int)GetEle<object>(arr, 2));
        }

        [UnitTest]
        public void st_null_to_ref_array()
        {
            // Storing null should always succeed for reference type arrays.
            var arr = new Animal[1];
            SetEle<Animal>(arr, 0, null);
            Assert.Null(GetEle<Animal>(arr, 0));
        }

        [UnitTest]
        public void st_base_to_derived_array_throws()
        {
            // Array covariance: Dog[] can be assigned to Animal[].
            // But storing an Animal into the underlying Dog[] should throw ArrayTypeMismatchException.
            // Regression: with reversed parameter order, this would incorrectly succeed.
            Animal[] arr = new Dog[1];
            bool threw = false;
            try
            {
                SetEle<Animal>(arr, 0, new Animal());
            }
            catch (ArrayTypeMismatchException)
            {
                threw = true;
            }
            Assert.IsTrue(threw);
        }
    }
}
