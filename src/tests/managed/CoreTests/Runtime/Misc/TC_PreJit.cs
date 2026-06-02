
//using HybridCLR;
//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;

//namespace Tests.Mics
//{
//    internal class TC_PreJit : GeneralTestCaseBase
//    {
//        interface IJitInterface
//        {
//            int Run();
//        }

//        class ProJitClass1 : IJitInterface
//        {
//            public int Run()
//            {
//                return 1;
//            }
//        }

//        class ProJitClass2
//        {
//            public int Run1()
//            {
//                return 1;
//            }


//            public int Run2()
//            {
//                return 2;
//            }

//            public static int Foo<T>()
//            {
//                return 1;
//            }
//        }

//        class ProJitClass3<T>
//        {
//            public int Run1()
//            {
//                return 1;
//            }
//        }


//        [UnitTest]
//        public void PreJitAOTMethod()
//        {
//            var method = typeof(Ldftn_A).GetMethod("Inc");
//            Assert.NotNull(method);

//            bool r = RuntimeApi.PreJitMethod(method);
//            Assert.False(r);
//        }

//        [UnitTest]
//        public void PreJitInterpreterMethod()
//        {
//            var method = typeof(ProJitClass1).GetMethod("Run");
//            bool r = RuntimeApi.PreJitMethod(method);
//#if UNITY_EDITOR
//            Assert.False(r);
//#else
//            Assert.True(r);
//#endif
//        }

//        [UnitTest]
//        public void PreJitInterpreterNotInstantiatedGenericMethod()
//        {
//            var method = typeof(ProJitClass2).GetMethod("Foo");
//            bool r = RuntimeApi.PreJitMethod(method);
//            Assert.False(r);
//        }

//        [UnitTest]
//        public void PreJitInterpreterInstantiatedGenericMethod()
//        {
//            var method = typeof(ProJitClass2).GetMethod("Foo");
//            var genericMethod = method.MakeGenericMethod(typeof(int));
//            bool r = RuntimeApi.PreJitMethod(genericMethod);
//#if UNITY_EDITOR
//            Assert.False(r);
//#else
//            Assert.True(r);
//#endif
//        }

//        [UnitTest]
//        public void PreJitAOTClass()
//        {
//            var type = typeof(Ldftn_A);
//            bool r = RuntimeApi.PreJitClass(type);
//            Assert.False(r);
//        }

//        [UnitTest]
//        public void PreJitInterpreterClass()
//        {
//            var type = typeof(ProJitClass2);
//            bool r = RuntimeApi.PreJitClass(type);
//#if UNITY_EDITOR
//            Assert.False(r);
//#else
//            Assert.True(r);
//#endif
//        }

//        [UnitTest]
//        public void PreJitGenericClassFail()
//        {
//            var t = typeof(ProJitClass3<>);
//            bool r = RuntimeApi.PreJitClass(t);
//            Assert.False(r);
//        }
//    }
//}
