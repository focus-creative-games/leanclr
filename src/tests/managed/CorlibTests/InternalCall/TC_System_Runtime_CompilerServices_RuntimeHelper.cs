using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Runtime_CompilerServices_RuntimeHelper : TestCaseBase
    {
        [UnitTest]
        public void GetObjectValue_ok()
        {
            int a = 5;
            object boxedA = a;
            int result = (int)System.Runtime.CompilerServices.RuntimeHelpers.GetObjectValue(boxedA);
            Assert.Equal(a, result);
        }

        [UnitTest]
        public unsafe void GetOffSetToString()
        {
            var s = "abc";
            fixed (char* p = s)
            {
                Assert.Equal('a', p[0]);
            }
        }

        class A
        {
            public static int value = 1;
        }

        class B
        {
            static B()
            {
                A.value = 2;
            }
        }


        [UnitTest]
        public void RunClassConstructor_ok()
        {
            Assert.Equal(1, A.value);
            System.Runtime.CompilerServices.RuntimeHelpers.RunClassConstructor(typeof(B).TypeHandle);
            Assert.Equal(2, A.value);
        }

        [UnitTest]
        public void RunModuleConstructor_ok()
        {
            // There's no direct way to test this, but we can at least call it to ensure it doesn't throw.
            System.Runtime.CompilerServices.RuntimeHelpers.RunModuleConstructor(typeof(TC_System_Runtime_CompilerServices_RuntimeHelper).Module.ModuleHandle);
        }
    }
}
