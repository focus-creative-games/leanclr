using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AOTDefs
{
    public class AOTActivator
    {
        public static T Create<T>() where T : new()
        {
            return new T();
        }

        public class ClassA
        {
            public int x;
        }

        public struct StructA
        {
            public int x;
        }
    }
}
