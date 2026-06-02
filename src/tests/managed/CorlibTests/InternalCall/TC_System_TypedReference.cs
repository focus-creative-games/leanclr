using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_TypedReference : TestCaseBase
    {
        [UnitTest]
        public void MakeTypedReference_Test()
        {
            int intValue = 42;
            TypedReference tr = __makeref(intValue);
            int extractedValue = __refvalue(tr, int);
            Assert.Equal(42, extractedValue);
        }

        [UnitTest]
        public void InternalToObject_Test()
        {
            int intValue = 100;
            TypedReference tr = __makeref(intValue);
            object obj = TypedReference.ToObject(tr);
            Assert.IsTrue(obj is int);
            Assert.Equal(100, (int)obj);
        }
    }
}
