using System;

namespace Tests.Intrinsic
{
    internal class TC_Syste_Span : TestCaseBase
    {
        // System.Span<T> is not available in Mono 4.8 mscorlib CorlibTests refs; see ICALL_INTRINSIC_COVERAGE.md (blocked).
        [IgnoreTest]
        [UnitTest]
        public void GetItem()
        {
            Assert.True(true);
        }
    }
}
