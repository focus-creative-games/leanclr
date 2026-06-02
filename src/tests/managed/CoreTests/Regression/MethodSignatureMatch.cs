
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using AOTDefs;

namespace Tests.Bugs
{
    internal class MethodSignatureMatch : GeneralTestCaseBase
    {
        public class OVMParent
        {
            public virtual int Func1()
            {
                return 1;
            }

            public virtual int Func1<T>()
            {
                return 2;
            }

            public virtual int Func1<T1, T2>()
            {
                return 3;
            }

            //public virtual int Init()
            //{
            //    return Func1();
            //}

            //public virtual int Init()
            //{
            //    return Func1();
            //}
        }

        public class OVMChild : OVMParent
        {
            public override int Func1()
            {
                return 10;
            }
            public override int Func1<T>()
            {
                return 20;
            }
            public override int Func1<T1, T2>()
            {
                return 30;
            }

            public int Init1()
            {
                return Func1();
            }

            public int Init2()
            {
                return Func1<int>();
            }

            public int Init3()
            {
                return Func1<int, long>();
            }
        }

        [UnitTest]
        public void Test1()
        {
            var test = new OVMChild();
            Assert.Equal(10, test.Init1());
        }

        [UnitTest]
        public void Test2()
        {
            var test = new OVMChild();
            Assert.Equal(20, test.Init2());
        }

        [UnitTest]
        public void Test3()
        {
            var test = new OVMChild();
            Assert.Equal(30, test.Init3());
        }

        [UnitTest]
        public void Test4()
        {
            var test = new InstanceMethodRefs();
            Assert.Equal(2, test.Foo1());
        }

        [UnitTest]
        public void Test5()
        {
            Assert.Equal(2, StaticMethodRefs.Foo1());
        }
    }
}
