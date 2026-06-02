using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    internal class GenericVirtualMethodReflection : GeneralTestCaseBase
    {
        interface IGvmr
        {
            string Get<T>();
        }

        class GvmrImpl : IGvmr
        {
            public virtual string Get<T>()
            {
                return typeof(T).Name;
            }
        }

        abstract class GvmrBase
        {
            public abstract string Get<T>();
        }

        class Gvmr2 : GvmrBase
        {
            public override string Get<T>()
            {
                return typeof(T).Name;
            }
        }

        [UnitTest]
        public void CallInterface()
        {
            IGvmr g = new GvmrImpl();
            MethodInfo m = typeof(IGvmr).GetMethod("Get").MakeGenericMethod(typeof(string));
            Assert.Equal("String", m.Invoke(g, null));
        }

        [UnitTest]
        public void CallVirtual()
        {
            GvmrBase g = new Gvmr2();
            MethodInfo m = typeof(GvmrBase).GetMethod("Get").MakeGenericMethod(typeof(string));
            Assert.Equal("String", m.Invoke(g, null));
        }

        [UnitTest]
        public void CallGenericInterfaceVirtual()
        {
            string[] arr = new string[] { "1", "2" };
            MethodInfo m = typeof(IEnumerable<string>).GetMethod("GetEnumerator");
            IEnumerator<string> e2 = (IEnumerator<string>)m.Invoke(arr, null);
            e2.MoveNext();
            Assert.Equal("1", e2.Current);
        }
    }
}
