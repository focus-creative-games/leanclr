using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace Tests.Bugs.Bug20220927
{
    interface IX
    {
        string ToString();
    }

    struct NewToString
    {
        public new string ToString()
        {
            return "A.ToString";
        }
    }

    struct NewToString2 : IX
    {
        public new string ToString()
        {
            return "B.ToString";
        }
    }

    interface IY
    {
        string Foo();
    }

    struct C : IY
    {

        string IY.Foo()
        {
            return "IY.Foo";
        }

        public string Foo()
        {
            return "C.Foo";
        }
    }


    public interface IZ
    {
        string Bar();
    }

    struct D : IZ
    {

        string IZ.Bar()
        {
            return "IZ.Bar";
        }

        public string Bar()
        {
            return "D.Bar";
        }
    }

    struct E : IComparable<E>
    {
        int IComparable<E>.CompareTo(E other)
        {
            return 10;
        }

        public int CompareTo(E other)
        {
            return 20;
        }
    }



    internal class Bug_2022_9_27 : GeneralTestCaseBase
    {
        string Test1<T>(T obj)
        {
            return obj.ToString();
        }

        string Test2<T>(T obj) where T : IY
        {
            return obj.Foo();
        }

        string Test3<T>(T obj) where T : IZ
        {
            return obj.Bar();
        }

        int Test4<T>(T obj) where T : IComparable<T>
        {
            return obj.CompareTo(obj);
        }

        [UnitTest]
        public void test1()
        {
            string s = Test1(new NewToString());
            Assert.Equal("Tests.Bugs.Bug20220927.NewToString", s);
        }

        [UnitTest]
        public void test2()
        {
            string s = Test1(new NewToString2());
            Assert.Equal("Tests.Bugs.Bug20220927.NewToString2", s);
        }

        [UnitTest]
        public void test3()
        {
            string s = Test2(new C());
            Assert.Equal("IY.Foo", s);
        }

        [UnitTest]
        public void test4()
        {
            string s = Test3(new D());
            Assert.Equal("IZ.Bar", s);
        }

        [UnitTest]
        public void test5()
        {
            int s = Test4(new E());
            Assert.Equal(10, s);
        }
    }
}
