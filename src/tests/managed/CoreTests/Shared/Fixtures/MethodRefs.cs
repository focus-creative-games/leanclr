using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AOTDefs
{
    public class InstanceMethodRefs
    {
        public static int Foo1<T1, T2>()
        {
            return 1;
        }

        public int Foo1()
        {
            return 2;
        }

        public static int Foo1<T>()
        {
            return 3;
        }

        public static int Foo1<T1, T2, T3>()
        {
            return 4;
        }
    }

    public class StaticMethodRefs
    {
        public int Foo1<T1, T2>()
        {
            return 1;
        }

        public static int Foo1()
        {
            return 2;
        }

        public int Foo1<T>()
        {
            return 3;
        }

        public int Foo1<T1, T2, T3>()
        {
            return 4;
        }
    }
}
