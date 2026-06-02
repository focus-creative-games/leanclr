using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BootstrapTests
{
    internal class TestReturnValue
    {
        public static int ReturnInt()
        {
            return 42;
        }

        public static string ReturnString()
        {
            return "abc";
        }

        public struct Vec3
        {
            public int x;
            public int y;
            public int z;
        }

        public static Vec3 ReturnStruct()
        {
            Vec3 v;
            v.x = 1;
            v.y = 2;
            v.z = 3;
            return v;
        }
    }
}
