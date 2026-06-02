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
//    public class TC_MonoPInvokeWrapper : GeneralTestCaseBase
//    {

//        /*
//         * 
//         * Crash on mono
//        [UnitTest]
//        public void MarshalMulticast()
//        {
//            MyFunc func = MonoPInvokeWrapperPreserves.Sum;
//            func += MonoPInvokeWrapperPreserves.Sum2;
//            IntPtr funcPointer = Marshal.GetFunctionPointerForDelegate(func);
//            Assert.True(funcPointer == default);
//        }
//        */

//        [UnitTest]
//        public void GetAllReversePInvokeMethods()
//        {
//            foreach(var method in typeof(MonoPInvokeWrapperPreserves).GetMethods())
//            {
//                foreach(var attr in method.GetCustomAttributes(false))
//                {
//                    if (attr is MonoPInvokeCallbackAttribute)
//                    {
//                        if (!method.Name.Contains("Lua"))
//                        {
//                            continue;
//                        }
//                        Delegate del = Delegate.CreateDelegate(typeof(LuaFunction), method);
//                        IntPtr funcPointer = Marshal.GetFunctionPointerForDelegate(del);
//                        Assert.True(funcPointer != default);
//                    }
//                }
//            }
//        }

//        [UnitTest]
//        public void TestCallPrimitiveTypes()
//        {

//            var method = typeof(MonoPInvokeWrapperPreserves).GetMethod("Sum");
//            Delegate del = Delegate.CreateDelegate(typeof(MyFunc), method);
//            IntPtr funcPointer = Marshal.GetFunctionPointerForDelegate(del);
//            Assert.True(funcPointer != default);

//            //MyFunc func = Marshal.GetDelegateForFunctionPointer<MyFunc>(funcPointer);
//            //int c = func(1, 2);
//            //Assert.Equal(3, c);
////#if UNITY_2021_1_OR_NEWER
////            unsafe
////            {
////                delegate*<int, int, int> call = (delegate*<int, int, int>)funcPointer;
////                Assert.Equal(3, call(1, 2));
////            }
////#endif
//        }

//        [UnitTest]
//        public void TestCallStructTypes()
//        {

//            var method = typeof(MonoPInvokeWrapperPreserves).GetMethod("SumVec");
//            Delegate del = Delegate.CreateDelegate(typeof(MyVecFunc), method);
//            IntPtr funcPointer = Marshal.GetFunctionPointerForDelegate(del);
//            Assert.True(funcPointer != default);

////#if UNITY_2021_1_OR_NEWER
////            unsafe
////            {
////                delegate*<Vector3, Vector3, Vector3> call = (delegate*<Vector3, Vector3, Vector3>)funcPointer;
////                var c = call(Vector3.one, Vector3.one);
////                Assert.Equal(2.0f, c.x);
////                Assert.Equal(2.0f, c.y);
////                Assert.Equal(2.0f, c.z);
////            }
////#endif
//        }

//        [UnitTest]
//        public void TestCallConvension()
//        {
//            var attr = (UnmanagedFunctionPointerAttribute)typeof(LuaFunction).GetCustomAttribute(typeof(UnmanagedFunctionPointerAttribute));
//            Assert.NotNull(attr);
//            Assert.Equal(CallingConvention.Cdecl, attr.CallingConvention);
//        }

//        [UnitTest]
//        public void MarshalCcall()
//        {
//            var method = typeof(MonoPInvokeWrapperPreserves).GetMethod("LuaCallback");
//            Delegate del = Delegate.CreateDelegate(typeof(LuaFunction), method);
//            IntPtr funcPointer = Marshal.GetFunctionPointerForDelegate(del);
//            Assert.True(funcPointer != default);
//        }

//        [UnitTest]
//        public void MarshalDefaultCallFunc()
//        {
//            var method = typeof(MonoPInvokeWrapperPreserves).GetMethod("Sum");
//            Delegate del = Delegate.CreateDelegate(typeof(MyFunc), method);
//            IntPtr funcPointer = Marshal.GetFunctionPointerForDelegate(del);
//            Assert.True(funcPointer != default);
//        }

//        [UnitTest]
//        public void MarshalDefaultCallFunc2()
//        {
//            var method = typeof(MonoPInvokeWrapperPreserves).GetMethod("Sum2");
//            Delegate del = Delegate.CreateDelegate(typeof(MyFunc), method);
//            IntPtr funcPointer = Marshal.GetFunctionPointerForDelegate(del);
//            Assert.True(funcPointer != default);
//        }

//        [UnitTest]
//        public void MarshalDefaultCallAction()
//        {
//            var method = typeof(MonoPInvokeWrapperPreserves).GetMethod("Run");
//            Delegate del = Delegate.CreateDelegate(typeof(Action<int, int>), method);
//            IntPtr funcPointer = Marshal.GetFunctionPointerForDelegate(del);
//            Assert.True(funcPointer != default);
//        }

//        [UnitTest]
//        public void MarshalAotCcall()
//        {
//            var method = typeof(MonoPInvokeWrapperPreserves).GetMethod("Run2");
//            Delegate del = Delegate.CreateDelegate(typeof(CppBattleEngineReadFileEvent), method);
//            IntPtr funcPointer = Marshal.GetFunctionPointerForDelegate(del);
//            Assert.True(funcPointer != default);
//        }
//    }
//}
