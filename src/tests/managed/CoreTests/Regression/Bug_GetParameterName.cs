using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{


    internal class Bug_GetParameterName : GeneralTestCaseBase
    {
        [UnitTest]
        public void param_name()
        {
            var type = typeof(AOTDefs.ValueTypeSize1[]);
            var method = type.GetMethod("GetValue", new Type[] {typeof(int)});
            var param = method.GetParameters()[0].Name;
            Assert.Equal("index", param);
        }

    }
}
