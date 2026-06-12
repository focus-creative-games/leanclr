using System;

namespace CorlibTests.InternalCall
{
    internal class TC_System_DateTime : TestCaseBase
    {
        [CoversIcall("System.DateTime::GetSystemTimeAsFileTime()")]
        [UnitTest]
        public void UtcNow_UsesSystemTime()
        {
            DateTime before = DateTime.UtcNow;
            DateTime after = DateTime.UtcNow;
            Assert.IsTrue(after >= before);
            Assert.IsTrue(before.Ticks > 0);
        }
    }
}
