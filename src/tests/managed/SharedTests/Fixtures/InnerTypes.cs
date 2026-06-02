using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AOTDefs
{
    public static class InnerTypes
    {
        public struct A
        {
            public int x;

            public override string ToString()
            {
                return $"A:{x}";
            }
        }

        public struct B
        {
            public int x;

            public override string ToString()
            {
                return $"B:{x}";
            }
        }

        public static int Sum(A a, B b)
        {
            return a.x + b.x;
        }

        public struct C<T, U>
        {
            public T x;
            public U y;

            public override string ToString()
            {
                return $"{x},{y}";
            }
        }

        public static string SumC<T, U>(C<T, U> c)
        {
            return c.ToString();
        }

        public static string SumC1(C<A, B> c)
        {
            return SumC(c);
        }
    }
}
