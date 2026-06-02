
//using test;
//using System;
//using System.Collections;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;


//namespace Tests.CSharp
//{
//    class Fin
//    {
//        ~Fin()
//        {
//        }
//    }

//    class Fa0
//    {

//    }

//    class Fa1 : Fa0
//    {
//        ~Fa1()
//        {

//        }
//    }

//    class Fa2 : Ldftn_A
//    {
//        ~Fa2()
//        {

//        }
//    }

//    interface IExplicit
//    {
//        int Show();
//    }

//    class ExplicitMethod : IExplicit
//    {
//        private int depth = 0;

//        public virtual int Show()
//        {
//            return 1;
//        }

//        int IExplicit.Show()
//        {
//            if (depth == 0)
//            {
//                ++depth;
//                return Show();
//            }
//            else
//            {
//                return 2;
//            }
//        }
//    }

//    class ExplicitMethod2 : IExplicit
//    {
//        int IExplicit.Show()
//        {
//            return 2;
//        }
//    }

//    class ExplicteGenericMethod : IVG
//    {
//        T IVG.Bar<T>(T a)
//        {
//            return a;
//        }

//        string IVG.Foo<T>(T x)
//        {
//            return x.ToString();
//        }
//    }


//    internal class TC_explicit_implement : GeneralTestCaseBase
//    {
//        [UnitTest]
//        public void same_name()
//        {
//            IExplicit o = new ExplicitMethod();
//            Assert.Equal(1, o.Show());
//        }


//        [UnitTest]
//        public void test()
//        {
//            IExplicit o = new ExplicitMethod2();
//            Assert.Equal(2, o.Show());
//        }

//        [UnitTest]
//        public void explicit_generic_method()
//        {
//            IVG o = new ExplicteGenericMethod();
//            Assert.Equal(1, o.Bar<int>(1));
//        }
//    }
//}
