using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;


namespace AOTDefs
{
    public struct FT_AOT_ValueType
    {
        public int x;
        public float y;
        public string z;

        public void Run(int a)
        {
            x += a;
        }

        public int Show(int b)
        {
            return this.x + b;
        }

        public sbyte Foo(int b)
        {
            return (sbyte)(x + b);
        }
    }

    public delegate void DelAOTRun(ref FT_AOT_ValueType a, int b);
    public delegate int DelAOTShow(ref FT_AOT_ValueType a, int b);
    public delegate sbyte DelAOTFoo(ref FT_AOT_ValueType a, int b);



    public delegate void DelAOTRun2(int b);
    public delegate int DelAOTShow2(int b);
    public delegate sbyte DelAOTFoo2(int b);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate IntPtr CppBattleEngineReadFileEvent(IntPtr fileName);


    public class FT_AOT_Class
    {
        public int x;
        public float y;
        public string z;

        public void Run(int a)
        {
            x += a;
        }

        public int Show(int b)
        {
            return this.x + b;
        }

        public sbyte Foo(int b)
        {
            return (sbyte)(x + b);
        }

        public static void Run2(FT_AOT_Class s, int b)
        {
            s.x += b;
        }

        public static void Run3(FT_AOT_Class s, int b)
        {
            
        }

        public static int Show2(FT_AOT_Class s, int b)
        {
            return s.x + b;
        }

        public static sbyte Foo2(FT_AOT_Class s, int b)
        {
            return (sbyte)(s.x + b);
        }
    }




    public static class CallDelegateFuncs
    {
        public static void Invoke(Action d)
        {
            d();
        }

        public static void Invoke(Action<int> d, int b)
        {
            d(b);
        }

        public static int Invoke(Func<int, int> d, int b)
        {
            return d(b);
        }

        public static int Invoke(Func<int> d)
        {
            return d();
        }

        public static sbyte Invoke(Func<int, sbyte> d, int b)
        {
            return d(b);
        }

        public static void Invoke(DelAOTRun d, ref FT_AOT_ValueType a, int b)
        {
            d(ref a, b);
        }

        public static int Invoke(DelAOTShow d, ref FT_AOT_ValueType a, int b)
        {
            return d(ref a, b);
        }

        public static sbyte Invoke(DelAOTFoo d, ref FT_AOT_ValueType a, int b)
        {
            return d(ref a, b);
        }

        public static void Invoke(Action<FT_AOT_Class, int> d, FT_AOT_Class a, int b)
        {
            d(a, b);
        }

        public static int Invoke(Func<FT_AOT_Class, int, int> d, FT_AOT_Class a, int b)
        {
            return d(a, b);
        }

        public static sbyte Invoke(Func<FT_AOT_Class, int, sbyte> d, FT_AOT_Class a, int b)
        {
            return d(a, b);
        }

        public static int CallFuncN(Func<int> func, int n)
        {
            int sum = 0;
            for (int i = 0; i < n; i++)
            {
                sum += func();
            }
            return sum;
        }
    }
}
