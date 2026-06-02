using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs.Bug2022_7_14
{
    public class A<T> : List<T>
    {
    }

    internal class Bug_2022_7_14 : GeneralTestCaseBase
    {
        [UnitTest]
        public void generic_type_name()
        {
            Assert.Equal("System.Collections.Generic.List`1[T]", typeof(A<>).BaseType.ToString());
        }
    }
}
