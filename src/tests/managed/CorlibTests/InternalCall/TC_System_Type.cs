using System;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Type : TestCaseBase
    {
        [CoversIcall("System.Type::internal_from_handle")]
        [UnitTest]
        public void GetTypeFromHandle_RoundTripsRuntimeType()
        {
            RuntimeTypeHandle handle = typeof(string).TypeHandle;
            Type resolved = Type.GetTypeFromHandle(handle);
            Assert.Equal(typeof(string), resolved);
        }
    }
}
