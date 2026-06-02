using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{


    internal class TC_CallUnmanagedDelegate : GeneralTestCaseBase
    {
        class TestClass
        {

        }

        static TestClass Creator()
        {
            return new TestClass();
        }

        //static TestClass Creator(int a)
        //{
        //    return new TestClass();
        //}


        private unsafe void Test2(int a, int b)
        {
#if UNITY_2021_3_OR_NEWER
            ((delegate*<TestClass>)(&Creator))();
            Assert.Equal(1, a);
            Assert.Equal(2, b);
#endif
        }

        [UnitTest]
        public void Test()
        {
            Test2(1, 2);
        }
    }
}
