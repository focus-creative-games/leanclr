//using Tests.Fixtures;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;

//namespace Tests.Bugs
//{
//    class B : TestAotRef
//    {

//    }

//    internal class Bug_2022_9_9 : TestCaseBase
//    {
//        [UnitTest]
//        public void method_match()
//        {
//            var arr = new List<B>() { new B() };
//            IEnumerable<TestAotRef> b = arr;
//            foreach(var o in b)
//            {
//                Assert.NotNull(o);
//            }
//        }

//    }
//}
