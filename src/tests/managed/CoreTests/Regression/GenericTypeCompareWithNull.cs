using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    internal class GenericTypeCompareWithNull : GeneralTestCaseBase
    {
        private static void CompareValueTypeWithNull<T>()
        {
            var value = default(T) == null;
            Assert.False(value);
            if (default(T) == null)
            {
                Assert.Fail();
            }
            else
            {
                Assert.True(true);
            }
        }

        private static void CompareValueTypeWithNull2<T>()
        {
            var value = default(T) != null;
            Assert.True(value);
            if (default(T) != null)
            {
                Assert.True(true);
            }
            else
            {
                Assert.Fail();
            }
        }
        private static void CompareValueTypeWithNull3<T>(T x, bool v)
        {
            var value = x == null;
            Assert.Equal(v, value);
            if (x == null)
            {
                Assert.True(v);
            }
            else
            {
                Assert.False(v);
            }
        }


        private static void CompareClassOrNullableWithNull<T>()
        {
            var value = default(T) == null;
            Assert.True(value);
            if (default(T) == null)
            {
                Assert.True(true);
            }
            else
            {
                Assert.Fail();
            }
        }

        private static void CompareClassOrNullableWithNull2<T>()
        {
            var value = default(T) != null;
            Assert.False(value);
            if (default(T) != null)
            {
                Assert.Fail();
            }
            else
            {
                Assert.True(true);
            }
        }

        private static void CompareClassOrNullableWithNull3<T>(T x, bool v)
        {
            var value = x == null;
            Assert.Equal(value, v);
            if (x == null)
            {
                Assert.True(v);
            }
            else
            {
                Assert.False(v);
            }
        }

        [UnitTest]
        public void TestInt()
        {
            CompareValueTypeWithNull<int>();
        }

        [UnitTest]
        public void TestInt2()
        {
            CompareValueTypeWithNull2<int>();
        }

        [UnitTest]
        public void TestInt3()
        {
            CompareValueTypeWithNull3<int>(0, false);
        }

        [UnitTest]
        public void TestInt3_2()
        {
            CompareValueTypeWithNull3<int>(1, false);
        }

        [UnitTest]
        public void TestGenericPair()
        {
            CompareValueTypeWithNull<KeyValuePair<int, int>>();
        }

        [UnitTest]
        public void TestGenericPair2()
        {
            CompareValueTypeWithNull2<KeyValuePair<int, int>>();
        }

        [UnitTest]
        public void TestGenericPair3()
        {
            CompareValueTypeWithNull3<KeyValuePair<int, int>>(default, false);
            CompareValueTypeWithNull3<KeyValuePair<int, int>>(new KeyValuePair<int, int>(1, 2), false);
        }

        [UnitTest]
        public void TestObject()
        {
            CompareClassOrNullableWithNull<object>();
        }

        [UnitTest]
        public void TestObject2()
        {
            CompareClassOrNullableWithNull<object>();
        }

        [UnitTest]
        public void TestObject3()
        {
            CompareClassOrNullableWithNull3<object>(null, true);
        }

        [UnitTest]
        public void TestObject3_2()
        {
            CompareClassOrNullableWithNull3<object>(this, false);
        }

        [UnitTest]
        public void TestNullable()
        {
            CompareClassOrNullableWithNull<int?>();
        }

        [UnitTest]
        public void TestNullable2()
        {
            CompareClassOrNullableWithNull2<int?>();
        }

        [UnitTest]
        public void TestNullable3()
        {
            CompareClassOrNullableWithNull3<int?>(null, true);
        }

        [UnitTest]
        public void TestNullable3_2()
        {
            CompareClassOrNullableWithNull3<int?>(1, false);
        }
    }
}
