using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;


namespace Tests.Bugs
{
    public class LoopCallInvoke : GeneralTestCaseBase
    {
        private static int Foo()
        {
            return 0;
        }


        [UnitTest]

        public static void Crash()
        {  
            /// <summary>
            /// 超大调用数量会崩溃 最新版本6.6会出现  之前的老版本 5.x是正常的
            /// </summary>
            var count = 50000;
            Func<int> f = Foo;

            //这里大概到17000就崩溃了
            for (int i = 0; i < count; i++)
            {
                f();
            }
        }
    }
}
