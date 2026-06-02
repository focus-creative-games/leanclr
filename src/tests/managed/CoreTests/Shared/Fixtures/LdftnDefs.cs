using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AOTDefs
{
    public class Ldftn_A
    {
        public int x = 1;

        public void Inc()
        {
            ++x;
        }

        public int Foo()
        {
            return x;
        }

        public static int Sqr(int a)
        {
            return a * a;
        }
    }

    public struct Ldftn_A2
    {
        public int x;

        public void Inc()
        {
            ++x;
        }

        public int Foo()
        {
            return x;
        }

        public static int Sqr(int a)
        {
            return a * a;
        }
    }
}
