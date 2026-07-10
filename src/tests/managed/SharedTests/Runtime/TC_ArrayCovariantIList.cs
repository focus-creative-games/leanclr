using System.Collections.Generic;

namespace Tests.CSharp
{
    internal class TC_ArrayCovariantIList : TestCaseBase
    {
        class TestBase
        {
            public int Value;
        }

        class TestDerived : TestBase
        {
        }

        [UnitTest]
        public void Array_CovariantGenericIList_Read()
        {
            TestDerived[] derivedArray = { new TestDerived { Value = 13 } };

            IList<TestBase> list = derivedArray;

            Assert.Equal(1, list.Count);
            Assert.Equal(13, list[0].Value);
        }
    }
}
