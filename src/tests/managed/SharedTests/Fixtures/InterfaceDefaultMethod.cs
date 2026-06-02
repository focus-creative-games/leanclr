using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Fixtures
{
    public interface InterfaceDefaultMethod
    {
#if UNITY_2021_1_OR_NEWER
        void Foo()
        {

        }
#endif
    }

    public class ClassExtendsInterfaceDefaultMethod : InterfaceDefaultMethod
    {

    }
}
