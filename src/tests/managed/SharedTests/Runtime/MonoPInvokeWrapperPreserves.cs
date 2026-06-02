//using AOT;

//using HybridCLR;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Runtime.InteropServices;
//using System.Text;
//using System.Threading.Tasks;

//namespace Tests.CSharp
//{

//    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//    delegate int LuaFunction(IntPtr luaState);


//    delegate int MyFunc(int a, int b);

//    delegate UnityEngine.Vector3 MyVecFunc(UnityEngine.Vector3 a, UnityEngine.Vector3 b);

//    delegate float MySpecFunc(float a, sbyte b, ushort c, double d, ulong e);

//    public class MonoPInvokeWrapperPreserves
//    {
//        [ReversePInvokeWrapperGeneration(10)]
//        [MonoPInvokeCallback(typeof(LuaFunction))]
//        public static int LuaCallback(IntPtr luaState)
//        {
//            return 0;
//        }

//        [MonoPInvokeCallback(typeof(MyFunc))]
//        public static int Sum(int a, int b)
//        {
//            return a + b;
//        }

//        [MonoPInvokeCallback(typeof(MyFunc))]
//        public static int Sum2(int a, int b)
//        {
//            return a + b;
//        }

//        [MonoPInvokeCallback(typeof(Action<int, int>))]
//        public static void Run(int a, int b)
//        {

//        }

//        [MonoPInvokeCallback(typeof(MyVecFunc))]
//        public static UnityEngine.Vector3 SumVec(UnityEngine.Vector3 a, UnityEngine.Vector3 b)
//        {
//            return a + b;
//        }

//        [MonoPInvokeCallback(typeof(Func<object>))]
//        public static object GetObject()
//        {
//            return new object();
//        }

//        [MonoPInvokeCallback(typeof(Func<object, int>))]
//        public static int GetObject(object obj)
//        {
//            return 1;
//        }

//        [MonoPInvokeCallback(typeof(Func<int>))]
//        public static int GetInt()
//        {
//            return 1;
//        }

//        [MonoPInvokeCallback(typeof(CppBattleEngineReadFileEvent))]
//        public static IntPtr Run2(IntPtr b)
//        {
//            return default;
//        }

//        [MonoPInvokeCallback(typeof(MySpecFunc))]
//        public static float Run3(float a, sbyte b, ushort c, double d, ulong e)
//        {
//            return a + b + c + (float)d + (float)e;
//        }
//    }

//}
