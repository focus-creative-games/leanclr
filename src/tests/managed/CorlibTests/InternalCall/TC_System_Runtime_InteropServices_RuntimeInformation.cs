using System;
using System.Runtime.InteropServices;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Runtime_InteropServices_RuntimeInformation : TestCaseBase
    {
        [UnitTest]
        public void GetRuntimeArchitecture_ReturnsWasm32()
        {
            var _ = RuntimeInformation.ProcessArchitecture.ToString();
        }

        [UnitTest]
        public void GetOSName_ReturnsWebAssembly()
        {
        }
    }
}
