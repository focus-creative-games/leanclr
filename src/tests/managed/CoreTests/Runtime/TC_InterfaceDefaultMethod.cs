using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.CSharp
{
    public interface IShow
    {
#if UNITY_2021_1_OR_NEWER
        int Show(int a)
        {
            return a + 1;
        }
#else
        int Show(int a);
#endif
    }

    public class MyShow : IShow
    {
#if !UNITY_2021_1_OR_NEWER
        public int Show(int a)
        {
            return a + 1;
        }
#endif
    }

    public interface ITest
    {
        int Say();
    }
    public interface ITest2 : ITest
    {
#if UNITY_2021_1_OR_NEWER
        int ITest.Say()
        {
            return 1;
        }
#endif
    }
    public class Test : ITest2
    {
#if !UNITY_2021_1_OR_NEWER
        public int Say()
        {
            return 1;
        }
#endif
    }
    
    internal class TC_InterfaceDefaultMethod : GeneralTestCaseBase
    {
        [UnitTest]
        public void test()
        {
            IShow a = new MyShow();
            Assert.Equal(11, a.Show(10));
        }
        
        
        [UnitTest]
        public void test2()
        {
            ITest a = new Test();
            Assert.Equal(1, a.Say());
        }
    }
    }

