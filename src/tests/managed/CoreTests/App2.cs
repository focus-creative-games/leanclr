using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace test
{
    public partial class App
    {
        [DllImport("nonexistent.dll")]
        public static extern void TestPinvoke();

        public static int Run0(object o)
        {
            return o.GetHashCode();
        }

        public static Type Run1()
        {
            var o = new object();
            return o.GetType();
        }

        class C1: ICloneable
        {
            public int a;
            public int b;

            public object Clone()
            {
                return MemberwiseClone();
            }
        }

        public static int Run2()
        {
            var o = new C1 { a = 10, b = 20 };
            var o2 = (C1)o.Clone();
            return o2.b;
        }

        public static string Run3()
        {
            char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e' };
            return new string(arr);
        }

        public static string Run4()
        {
            char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e' };
            return new string(arr, 1, 2);
        }

        public unsafe static string Run5()
        {
            char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e', '\0' };
            fixed (char* p = &arr[0])
            {
                return new string(p);
            }
        }

        public unsafe static string Run6()
        {
            char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e', '\0' };
            fixed (char* p = &arr[0])
            {
                return new string(p, 1, 2);
            }
        }

        public unsafe static string Run7()
        {
            var arr = new sbyte[] { (sbyte)'a', (sbyte)'b', (sbyte)'c', (sbyte)'d', (sbyte)'e', 0 };
            fixed (sbyte* p = &arr[0])
            {
                return new string(p);
            }
        }

        public unsafe static string Run8()
        {
            var arr = new sbyte[] { (sbyte)'a', (sbyte)'b', (sbyte)'c', (sbyte)'d', (sbyte)'e', 0 };
            fixed (sbyte* p = &arr[0])
            {
                return new string(p, 1, 2);
            }
        }

        public unsafe static string Run9()
        {
            var arr = new sbyte[] { (sbyte)'a', (sbyte)'b', (sbyte)'c', (sbyte)'d', (sbyte)'e', 0 };
            fixed (sbyte* p = &arr[0])
            {
                // this function will be redirected to string.Ctor(sbyte*, int, int, Encoding)
                return new string(p, 1, 2, Encoding.UTF8);
            }
        }

        //public unsafe static string Run10()
        //{
        //    char[] arr = new char[] { 'a', 'b', 'c', 'd', 'e' };
        //    return new string(new ReadOnlySpan<char>(arr));
        //}

        public static string Run11()
        {
            // for test string.FastAllocateString because string.Concat will call it internally
            string a = "abc";
            string b = "def";
            return string.Concat(a, b);
        }

        public static string Run12()
        {
            string a = "abc";
            // will call string.InternalIntern internally
            string b = string.Intern(a);
            return b;
        }

        public static string Run13()
        {
            string a = "abc";
            // will call string.InternalIsInterned internally
            string b = string.IsInterned(a);
            return b;
        }

        public static int Run14()
        {
            string s = "hello";
            return s.Length;
        }
    }
}
