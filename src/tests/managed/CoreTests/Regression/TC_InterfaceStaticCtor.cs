using Tests.Fixtures;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    internal interface InterfaceStaticCtor
    {
        private static readonly object s_value = new object();

        static InterfaceStaticCtor()
        {
        }

        public static object StaticCtor()
        {
            return s_value;
        }
    }

    internal class TC_InterfaceStaticCtor : TestCaseBase
    {
        private void DeferCall3()
        {
            DeferCall2();
        }

        private void DeferCall2()
        {
            DeferCall1();
        }

        private void DeferCall1()
        {
            DeferCall();
        }

        private void DeferCall()
        {
            Assert.NotNull(InterfaceStaticCtor.StaticCtor());
        }

        private readonly List<Type> _types = new List<Type>();

        [UnitTest]
        public void NotNull()
        {
            Type type = typeof(InterfaceStaticCtor);
            _types.Add(type);
            DeferCall3();
        }
    }
}
