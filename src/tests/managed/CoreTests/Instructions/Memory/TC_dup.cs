using System;
using test;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


/// <summary>
/// 对于local变量，如果变量未使用，在Release编译中该变量会被裁剪掉，本文件的测试只有再Debug编译下才有效
/// </summary>
namespace Tests.Instruments.Mems
{
    internal class TC_dup : GeneralTestCaseBase
    {
        // 不好构造dup的例子，暂时算了
    }
}
