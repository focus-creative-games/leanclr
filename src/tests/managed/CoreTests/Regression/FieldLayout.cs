
//using test;


//namespace Tests.Bugs
//{
//    public class Base<T1>
//    {
//        public T1 cc;
//    }

//    public class BaseCls<T1, T2> : Base<T1>
//    {
//        public T1 dd;
//    }

//    public class BaseCls2<T2> : BaseCls<Vector3, T2>
//    {
//        public int ee = 0;
//    }

//    public class SubCls : BaseCls2<Vector3>
//    {
//        public int ff = 1;
//    }

//    public class FieldLayout : GeneralTestCaseBase
//    {
//        [UnitTest]
//        public void Test1()
//        {
//            var c = new SubCls();
//            Assert.Equal(0, c.ee);
//        }
//    }
//}