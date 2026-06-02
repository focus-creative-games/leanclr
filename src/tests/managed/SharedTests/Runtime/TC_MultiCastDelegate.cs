
//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;

//namespace Tests.CSharp.Delegates
//{
//    public class FT_MultiCast
//    {
//        public int total = 1;


//        public int Func1(sbyte a, float b, double c, Vector3d d, uint e, ushort f)
//        {
//            return total += a + (int)b + (int)c + (int)d.x + (int)e + f;
//        }

//        public static int Func2(sbyte a, float b, double c, Vector3d d, uint e, ushort f)
//        {
//            return a + (int)b + (int)c + (int)d.x + (int)e + f;
//        }


//        public void Action1(sbyte a, float b, double c, Vector3d d, uint e, ushort f)
//        {
//            total += a + (int)b + (int)c + (int)d.x + (int)e + f;
//        }
//    }

//    public class TC_MultiCastDelegate : GeneralTestCaseBase
//    {

//        [UnitTest]
//        public void call1()
//        {
//            var c = new FT_MultiCast();
//            Func<sbyte, float, double, Vector3d, uint, ushort, int> f = c.Func1;
//            f += FT_MultiCast.Func2;
//            int r = f(1, 10f, 100, new Vector3d { x = 200 }, 1000, 10000);
//            Assert.Equal(11311, r);
//            Assert.Equal(11312, c.total);
//        }

//        [UnitTest]
//        public void call2()
//        {
//            var c = new FT_MultiCast();
//            Action<sbyte, float, double, Vector3d, uint, ushort> f = c.Action1;
//            f += c.Action1;
//            f(1, 10f, 100, new Vector3d { x = 200 }, 1000, 10000);
//        }

//        [UnitTest]
//        public void call3()
//        {
//            Action<bool, int, int> f = TestHandler;
//            f += TestHandler;
//            f(true, 1, 2);
//        }

//        private void TestHandler(bool arg1, int arg2, int arg3)
//        {
            
//        }
//    }
//}
