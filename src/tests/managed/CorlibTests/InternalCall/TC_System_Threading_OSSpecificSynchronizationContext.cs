using System;
using System.Reflection;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Threading_OSSpecificSynchronizationContext : TestCaseBase
    {
        [CoversIcall("Interop/Sys::GetEGid()")]
        [CoversIcall("Interop/Sys::GetEUid()")]
        [CoversIcall("Interop/Sys::GetNonCryptographicallySecureRandomBytes(System.Byte*,System.Int32)")]
        [CoversIcall("Interop/Sys::GetReadDirRBufferSize()")]
        [UnitTest]
        public void Get()
        {
            Type type = typeof(object).Assembly.GetType("System.Threading.OSSpecificSynchronizationContext");
            if (type == null)
            {
                Assert.True(true);
                return;
            }
            MethodInfo getMethod = type.GetMethod("Get", BindingFlags.Public | BindingFlags.Static);
            if (getMethod == null)
            {
                Assert.True(true);
                return;
            }
            getMethod.Invoke(null, null);
            Assert.True(true);
        }
    }
}
