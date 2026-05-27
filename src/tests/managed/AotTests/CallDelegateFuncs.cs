using System;


namespace Tests.CSharp.Delegates
{
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
