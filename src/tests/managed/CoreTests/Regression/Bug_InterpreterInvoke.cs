//using Main.AOTDefs;
//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;


//namespace Tests.Bugs
//{
//    public class Bug_InterpreterInvoke : GeneralTestCaseBase
//    {
//        [UnitTest]
//        public void TestValueType()
//        {
//            var v = new NativeArrayParamValueType();
//            v.x = 1;
//            v.y = 2;
//            v.a = 3;
//            v.b = 4;

//            int x = v.GetValue(1f);
//            Assert.Equal(1, x);
//        }


//        [UnitTest]
//        public void TestClass()
//        {
//            var v = new NativeArrayParamClass();
//            v.x = 1;
//            v.y = 2;
//            v.a = 3;
//            v.b = 4;

//            int x = v.GetValue(1f);
//            Assert.Equal(1, x);
//        }
//    }
//}
