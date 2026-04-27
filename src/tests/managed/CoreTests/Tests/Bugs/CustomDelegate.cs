using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace CoreTests.Tests.Bugs
{
    public class TestAttribute : Attribute
    {
        public string Name;
        public string Value;

        public TestAttribute(string name, string value)
        {
            Name = name;
            Value = value;
        }
    }

    [TestAttribute("CustomDelegate", "Test")]
    internal class CustomDelegate
    {
        private static void OnA(CustomDelegate arg)
        {
            var attr = arg.GetType().GetCustomAttribute<TestAttribute>();
        }

        public delegate void D(CustomDelegate arg);

        [UnitTest]
        public void Run()
        {
            var d = new D(OnA);
            var instance = new CustomDelegate();
            d(instance);
        }
    }
}
