using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    public interface InterfaceWithStaticMethodInMiddle
    {
        int Foo1();

#if UNITY_2021_1_OR_NEWER
        static int Helper()
        {
            return 42;
        }
#endif

        int Foo2();
    }

    public class InterfaceWithStaticMethodInMiddleImpl : InterfaceWithStaticMethodInMiddle
    {
        public int Foo1()
        {
#if UNITY_2021_1_OR_NEWER
            return InterfaceWithStaticMethodInMiddle.Helper();
#endif
            return 42;
        }

        public int Foo2()
        {
            return 0;
        }
    }
}
