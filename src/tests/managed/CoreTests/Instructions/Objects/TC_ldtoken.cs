
using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Test;
using AOTDefs;

namespace Tests.Instruments.Objs
{
    class OutG<T>
    {
        public class InnerG<V>
        {

            public static void Show(int a)
            {

            }

            public static void Run<S>(S a)
            {

            }

            public class InterG2<K>
            {

                public static void Show(int a)
                {

                }

                public static void Run<S>(S a)
                {

                }

            }

            public delegate void Foo(int a);
        }

        public enum InterE
        {
            A,
        }

        public static void Show(int a)
        {

        }

        public static void Run<S>(S a)
        {

        }
    }

    internal class TC_ldtoken : GeneralTestCaseBase
    {

        [UnitTest]
        public void type_1()
        {
            Type t = typeof(string);
            Assert.Equal("String", t.Name);
        }


        [UnitTest]
        public void type_2()
        {
            Type t = typeof(IEnumerable<>);
            Assert.Equal("IEnumerable`1", t.Name);
        }


        [UnitTest]
        public void type_3()
        {
            Type t = typeof(IEnumerable<int>);
            Assert.Equal("IEnumerable`1", t.Name);
        }

        [UnitTest]
        public void type_4()
        {
            Type t = typeof(OutG<>.InnerG<>);
            Assert.Equal("InnerG`1", t.Name);
        }

        [UnitTest]
        public void type_5()
        {
            Type t = typeof(OutG<int>.InnerG<int>);
            Assert.Equal("InnerG`1", t.Name);
        }

        [UnitTest]
        public void type_6()
        {
            Type t = typeof(OutG<int>.InterE);
            Assert.Equal("InterE", t.Name);
        }

        [UnitTest]
        public void type_7()
        {
            Type t = typeof(OutG<int>.InnerG<int>.InterG2<int>);
            Assert.Equal("InterG2`1", t.Name);
        }

        [UnitTest]
        public void type_8()
        {
            Type t = typeof(OutG<int>.InnerG<int>.Foo);
            Assert.Equal("Foo", t.Name);
        }

        [UnitTest]
        public void method_1()
        {
            Action<int> a = OutG<int>.Run<int>;
            Assert.Equal("Run", a.Method.Name);
        }

        [UnitTest]
        public void method_2()
        {
            Action<int> a = OutG<int>.InnerG<object>.Run<int>;
            Assert.Equal("Run", a.Method.Name);
        }

        [UnitTest]
        public void method_3()
        {
            Action<int> a = OutG<int>.InnerG<object>.InterG2<int>.Run<int>;
            Assert.Equal("Run", a.Method.Name);
        }

        [UnitTest]
        public void method_4()
        {
            Action<int> a = OutG<int>.Show;
            Assert.Equal("Show", a.Method.Name);
        }

        [UnitTest]
        public void method_5()
        {
            Action<int> a = OutG<int>.InnerG<object>.Show;
            Assert.Equal("Show", a.Method.Name);
        }

        [UnitTest]
        public void method_6()
        {
            Action<int> a = OutG<int>.InnerG<object>.InterG2<int>.Show;
            Assert.Equal("Show", a.Method.Name);
        }

        [UnitTest]
        public void methodref_1()
        {
            Func<int, int> a = Ldftn_A.Sqr;
            Assert.Equal("Sqr", a.Method.Name);
        }
    }
}
