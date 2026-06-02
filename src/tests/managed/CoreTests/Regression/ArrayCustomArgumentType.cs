using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    internal class ArrayCustomArgumentType : GeneralTestCaseBase
    {
        class TestAttr : Attribute
        {
            public readonly Type type;


            public TestAttr(Type type)
            {
                this.type = type;
            }
        }

        [TestAttr(typeof(int[]))]
        class Test
        {

        }

        [TestAttr(typeof(Test[]))]
        class Test2
        {

        }

        [UnitTest]
        public void GetArrayType1()
        {
            var type = typeof(Test);
            var attr = type.GetCustomAttributes(typeof(TestAttr), false);
            Assert.NotNull(attr);
            Assert.Equal(1, attr.Length);
            Assert.Equal(typeof(int[]), ((TestAttr)attr[0]).type);
        }

        [UnitTest]
        public void GetArrayType2()
        {
            var type = typeof(Test2);
            var attr = type.GetCustomAttributes(typeof(TestAttr), false);
            Assert.NotNull(attr);
            Assert.Equal(1, attr.Length);
            Assert.Equal(typeof(Test[]), ((TestAttr)attr[0]).type);
        }
    }
}
