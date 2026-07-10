using System;

public class TestClass
{
}

namespace CorlibTests.InternalCall
{
    internal class TC_RuntimeType_GlobalNamespace : TestCaseBase
    {
        [UnitTest]
        public void RuntimeType_GlobalNamespace_ReturnsNull()
        {
            Type type = typeof(TestClass);

            Assert.Null(type.Namespace);
            Assert.Equal("TestClass", type.FullName);
            Assert.Equal("TestClass", type.ToString());
        }
    }
}
