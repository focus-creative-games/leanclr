using System;

namespace GcTests.Fixtures
{
    internal abstract class GcTestCaseBase : TestCaseBase
    {
        protected static void FullCollect()
        {
            GC.Collect();
            GC.WaitForPendingFinalizers();
            GC.Collect();
        }
    }
}
