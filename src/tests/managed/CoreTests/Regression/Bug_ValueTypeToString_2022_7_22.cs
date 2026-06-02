using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{

    /// <summary>
    /// Unity 2021 调用值类型成员函数，并且发生补充元数据机制时出错。 
    /// </summary>
    internal class Bug_ValueTypeToString_2022_7_22 : GeneralTestCaseBase
    {
        [UnitTest]
        public void tostring()
        {
            var p = new KeyValuePair<ulong, int>(1, 2);
            var s = p.ToString();
            Assert.Equal("[1, 2]", s);
        }
    }
}
