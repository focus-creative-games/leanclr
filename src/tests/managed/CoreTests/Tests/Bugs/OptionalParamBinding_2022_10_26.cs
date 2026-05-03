using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{

    internal class OptionalParamBinding_2022_10_26 : GeneralTestCaseBase
    {
        public static int Hello(int x)
        {
            return x;
        }

        public static int Foo(int x = 1)
        {
            return x;
        }

        public static string Foo2(string s = "abc")
        {
            return s;
        }

        public static Action Foo3(Action a = null)
        {
            return a;
        }

        enum Color
        {
            Red,
            Green,
            Blue,
        }

        static Color Foo4(Color c = Color.Red)
        {
            return c;
        }

        [UnitTest]
        public void ParamIsNotOptional()
        {
            var method = GetType().GetMethod("Hello", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.False(method.GetParameters()[0].IsOptional);
        }

        [UnitTest]
        public void ParamIsOptional()
        {
            var method = GetType().GetMethod("Foo", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.True(method.GetParameters()[0].IsOptional);
        }

        [UnitTest]
        public void GetParamDefaultValue()
        {
            var method = GetType().GetMethod("Foo", BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static);
            Assert.Equal(1, method.GetParameters()[0].DefaultValue);
        }

        [UnitTest]
        public void invoke()
        {
            int x = (int)GetType().InvokeMember("Foo", BindingFlags.Public | BindingFlags.NonPublic |
            BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Equal(1, x);
        }

        [UnitTest]
        public void invoke2()
        {
            string x = (string)GetType().InvokeMember("Foo2", BindingFlags.Public | BindingFlags.NonPublic |
            BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Equal("abc", x);
        }

        [UnitTest]
        public void invoke3()
        {
            Action x = (Action)GetType().InvokeMember("Foo3", BindingFlags.Public | BindingFlags.NonPublic |
            BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Null(x);
        }

        [UnitTest]
        public void invoke4()
        {
            Color x = (Color)GetType().InvokeMember("Foo4", BindingFlags.Public | BindingFlags.NonPublic |
            BindingFlags.Instance | BindingFlags.Static | BindingFlags.InvokeMethod | BindingFlags.OptionalParamBinding, null, null,
                null);
            Assert.Equal(Color.Red, x);
        }
    }
}
