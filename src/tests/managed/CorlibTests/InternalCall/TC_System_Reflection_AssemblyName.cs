using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace CorlibTests.InternalCall
{
    internal class TC_System_Reflection_AssemblyName : TestCaseBase
    {
        [UnitTest]
        public void ParseAssemblyName()
        {
            var an = new AssemblyName("mscorlib");
            Assert.Equal("mscorlib", an.Name);
            Assert.Null(an.CultureName);
            Assert.Null(an.Version);
        }

        [UnitTest]
        public void GetNativeName()
        {
            var an = GetType().Assembly.GetName();
            Assert.Equal("CorlibTests", an.Name);
        }


        [AttributeUsage(AttributeTargets.Class)]
        class MyCustomAttribute : Attribute
        {
            private int _count;

            private string _name;

            public int Value { get; set; }

            public MyCustomAttribute(string name, int count)
            {
                _name = name;
                _count = count;
            }
        }

        [MyCustomAttribute("walon", 42, Value = 100)]
        class A
        {
        }

        [UnitTest]
        public void ResolveArgumentsInternal()
        {
            var cads = typeof(A).GetCustomAttributesData();
            Assert.Equal(1, cads.Count);

            var cad = cads[0];

            var ctorArgs = cad.ConstructorArguments;
            Assert.Equal(2, ctorArgs.Count);

            var name = ctorArgs[0].Value;
            Assert.Equal("walon", name);
            var count = ctorArgs[1].Value;
            Assert.Equal(42, count);

            var namedArgs = cad.NamedArguments;
            Assert.Equal(1, namedArgs.Count);
            var value = namedArgs[0];
            Assert.Equal("Value", value.MemberName);
            Assert.Equal(100, value.TypedValue.Value);
        }
    }
}
