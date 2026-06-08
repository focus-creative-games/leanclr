using System;
using System.Reflection;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Threading_OSSpecificSynchronizationContext : TestCaseBase
    {
        [UnitTest]
        public void Get()
        {
            Type type = typeof(object).Assembly.GetType("System.Threading.OSSpecificSynchronizationContext");
            MethodInfo getMethod = type.GetMethod("Get", BindingFlags.Public | BindingFlags.Static);
            getMethod.Invoke(null, null);
        }
    }
}
