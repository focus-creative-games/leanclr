using System;

namespace Tests.Intrinsic
{
    internal class TC_System_Object : TestCaseBase
    {
        [CoversIcall("System.Object::.ctor()")]
        [UnitTest]
        public void DefaultCtor_CreatesInstance()
        {
            object obj = new object();
            Assert.NotNull(obj);
        }
    }
}
