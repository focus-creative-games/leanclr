//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;


//namespace Tests.CSharp.Delegates
//{

//    delegate int InterpDele(int a, float b, string c, UnityEngine.Vector3 d);

//    class TC_Delegate_BeginEndInvoke : GeneralTestCaseBase
//    {
//        [UnitTest]
//        public void aot_begin_invoke()
//        {
//#if !UNITY_WEBGL
//            Func<int> a = () => 10;
//            var result = a.BeginInvoke((IAsyncResult ar) =>
//            {

//            }, null);
//            Assert.NotNull(result);
//            result.AsyncWaitHandle.WaitOne();
//            int b = a.EndInvoke(result);
//            Assert.Equal(10, b);
//#endif
//        }


//        [UnitTest]
//        public void interp_begin_invoke()
//        {
//#if !UNITY_WEBGL
//            InterpDele del = (int a, float b, string c, UnityEngine.Vector3 d) =>
//            {
//                return a + (int)b + c.Length + (int)d.x;
//            };
//            var result = del.BeginInvoke(1, 10f, "aaa", new UnityEngine.Vector3(100f,200f,300f), (IAsyncResult ar) =>
//            {

//            }, null);
//            result.AsyncWaitHandle.WaitOne();
//            int x = del.EndInvoke(result);
//            Assert.Equal(114, x);
//#endif
//        }

//        struct A
//        {
//            public int x;
//            public int y;
//        }
//        [UnitTest]
//        public void sort()
//        {
//            var arr = new List<A>();
//            arr.Add(new A { x = 1, y = 10 });
//            arr.Add(new A { x = 3, y = 30 });
//            arr.Add(new A { x = 2, y = 20 });

//            arr.Sort((a, b) => a.x.CompareTo(b.x));

//            var e1 = arr[0];
//            Assert.Equal(1, e1.x);
//            Assert.Equal(10, e1.y);
//            var e2 = arr[1];
//            Assert.Equal(2, e2.x);
//            Assert.Equal(20, e2.y);
//        }
//    }
//}
