using System;
using test;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Instruments.Mems
{
    /// <summary>
    /// 与AOT之前的返回值在 Boostrap里测过了，因此这儿只测interpreter 的 ret。
    /// 但interpreter的ret，实际上在 ldarg指令里隐含测过了，因此这儿不再测试
    /// </summary>
    internal class TC_ret : GeneralTestCaseBase
    {

    }
}
