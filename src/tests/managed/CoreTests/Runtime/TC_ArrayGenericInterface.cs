using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.CSharp
{

    internal class TC_ArrayGenericInterface : GeneralTestCaseBase
    {
        class TestClass
        {
            public int x;
        }

        struct TestStruct
        {
            public int x;
        }

        [UnitTest]
        public void class_get_1()
        {
            var a = new TestClass[1] { new TestClass { x = 10 } };
            IList<TestClass> b = a;
            var s = b[0];
            Assert.Equal(10, s.x);
        }

        [UnitTest]
        public void class_set_1()
        {
            var a = new TestClass[1];
            IList<TestClass> b = a;
            b[0] = new TestClass { x = 11 };
            var k = b[0];
            Assert.Equal(11, k.x);
            Assert.Equal(11, a[0].x);
        }

        [UnitTest]
        public void struct_get_1()
        {
            var a = new TestStruct[1] { new TestStruct { x = 10 } };
            IList<TestStruct> b = a;
            var s = b[0];
            Assert.Equal(10, s.x);
        }

        [UnitTest]
        public void struct_set_1()
        {
            var a = new TestStruct[1];
            IList<TestStruct> b = a;
            b[0] = new TestStruct { x = 11 };
            var k = b[0];
            Assert.Equal(11, k.x);
            Assert.Equal(11, a[0].x);
        }
    }
}
