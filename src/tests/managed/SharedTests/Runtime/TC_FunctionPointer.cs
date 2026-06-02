//using AOT;
//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Reflection;
//using System.Runtime.InteropServices;
//using System.Text;
//using System.Threading.Tasks;


//namespace Tests.CSharp
//{
//    internal class TC_FunctionPointer : GeneralTestCaseBase
//    {
//#if UNITY_2021_1_OR_NEWER
//        public unsafe static int UnsafeFunc(int* x)
//        {
//            return *x + 1;
//        }

//        public unsafe static int UnsafeFuncPointer(delegate*<int*, int> func)
//        {
//            var i = 1;

//            return func(&i);
//        }
//#endif


//        [UnitTest]
//        public unsafe void CallDef()
//        {
//#if UNITY_2021_1_OR_NEWER
//            Assert.Equal(2, UnsafeFuncPointer(&UnsafeFunc));
//#endif
//        }

//        [UnitTest]
//        public unsafe void CallRef()
//        {
//#if UNITY_2021_1_OR_NEWER
//            Assert.Equal(2, AOTDefs.FunctionPointer.UnsafeFuncPointer(&UnsafeFunc));
//#endif
//        }

//        [UnmanagedFunctionPointer(CallingConvention.Winapi)]
//        public delegate int MyAddFunc(int x);

//        [MonoPInvokeCallback(typeof(MyAddFunc))]
//        public static int MyAdd(int x)
//        {
//            return x + 1;
//        }

//        [UnitTest]
//        public unsafe void CallNative()
//        {
//#if UNITY_2021_1_OR_NEWER

//            MethodInfo method = GetType().GetMethod("MyAdd");
//            Delegate del = Delegate.CreateDelegate(typeof(MyAddFunc), method);
//            IntPtr funcPointer = Marshal.GetFunctionPointerForDelegate(del);

//            Assert.Equal(2, ((delegate*<int, int>)funcPointer)(1));
//#endif
//        }
//    }
//}
