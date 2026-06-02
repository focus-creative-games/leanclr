//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;


//namespace Tests.CSharp
//{
//    internal class TC_async : GeneralTestCaseBase
//    {

//        private async Task<string> Sum0()
//        {
//            await Task.Delay(10);
//            return "xxx";
//        }

//        [UnitTest]
//        public void TestString()
//        {
//            if (Application.platform == RuntimePlatform.WebGLPlayer)
//            {
//                return;
//            }
//            string x = Task.Run(Sum0).Result;
//            Assert.Equal("xxx", x);
//        }

//        private async Task<int> Sum1()
//        {
//            await Task.Delay(10);
//            return 5;
//        }

//        [UnitTest]
//        public void TestInt()
//        {
//            if (Application.platform == RuntimePlatform.WebGLPlayer)
//            {
//                return;
//            }
//            int x = Task.Run(Sum1).Result;
//            Assert.Equal(5, x);
//        }

//        enum EColor
//        {
//            A = 1,
//            B = 2,
//        }

//        private async Task<EColor> Sum2()
//        {
//            await Task.Delay(10);
//            return EColor.A;
//        }

//        [UnitTest]
//        public void TestEnum()
//        {
//            if (Application.platform == RuntimePlatform.WebGLPlayer)
//            {
//                return;
//            }
//            EColor x = Task.Run(Sum2).Result;
//            Assert.Equal(EColor.A, x);
//        }

//        struct Foo
//        {
//            public int x;
//            public int y;
//        }

//        private async Task<Foo> Sum3()
//        {
//            await Task.Delay(10);
//            return new Foo { x = 1, y = 2 };
//        }

//        [UnitTest]
//        public void TestStruct()
//        {
//            if (Application.platform == RuntimePlatform.WebGLPlayer)
//            {
//                return;
//            }
//            Foo x = Task.Run(Sum3).Result;
//            Assert.Equal(1, x.x);
//            Assert.Equal(2, x.y);
//        }
//    }
//}
