using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Reflection_EventInfo : TestCaseBase
    {
        class A
        {
            public event Action Run;

            public void OnRun()
            {
                Run?.Invoke();
            }
        }

        [UnitTest]
        public void EventInfo_MemberType_ok()
        {
            var eventInfo = typeof(A).GetEvent("Run");
            Assert.NotNull(eventInfo);
            Assert.Equal("Run", eventInfo.Name);
        }
    }
}
