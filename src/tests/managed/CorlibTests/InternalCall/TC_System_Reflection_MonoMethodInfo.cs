using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Reflection_MonoMethodInfo : TestCaseBase
    {
        [AttributeUsage(AttributeTargets.ReturnValue)]
        class MyParameterAttribute : Attribute
        {
        }
        class A
        {
            [return: MyParameter]
            public int Run()
            {
                return 0;
            }
        }

        [UnitTest]
        public void GetReturnParameter()
        {
            var methodInfo = typeof(A).GetMethod("Run");
            var returnParameter = methodInfo.ReturnParameter;
            Assert.Equal(typeof(int), returnParameter.ParameterType);
            var cas = returnParameter.CustomAttributes;
            Assert.NotNull(cas);
            Assert.Equal(1, cas.Count());
        }
    }
}
