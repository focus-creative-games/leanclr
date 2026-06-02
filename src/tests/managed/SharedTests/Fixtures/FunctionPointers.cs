using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AOTDefs
{
#if UNITY_2021_1_OR_NEWER
    public class FunctionPointer 
    {
        public unsafe static int UnsafeFunc(int* x)
        {
            return *x + 1;
        }

        public unsafe static int UnsafeFuncPointer(delegate*<int*, int> func)
        {
            int i = 1;

            return func(&i).GetHashCode();
        }

        public unsafe void Call()
        {
            UnsafeFuncPointer(&UnsafeFunc);
        }
    }

    public class FunctionPointer2<T> where T : unmanaged
    {
        public unsafe static int UnsafeFunc(T* x)
        {
            return (int)x;
        }

        public unsafe static int UnsafeFuncPointer(delegate*<T*, int> func)
        {
            T i = default;

            return func(&i).GetHashCode();
        }

        public unsafe void Call()
        {
            UnsafeFuncPointer(&UnsafeFunc);
        }
    }

    public class RefFunctionPointer
    {
        public static void Ref1()
        {
            var tc = new FunctionPointer2<int>();
            tc.Call();
        }
    }
#endif
}
