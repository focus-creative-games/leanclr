//using HybridCLR;
//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Runtime.InteropServices;
//using System.Text;
//using System.Threading.Tasks;

//namespace Tests.CSharp
//{
//    public static class NativeDll
//    {
//#if UNITY_STANDALONE_WIN
//        private const string dllName = "GameAssembly";
//#elif UNITY_ANDROID || UNITY_STANDALONE_LINUX
//    private const string dllName = "il2cpp";
//#else
//    private const string dllName = "__Internal";
//#endif

//#if UNITY_EDITOR
//        public static int GetIntArgument(int value)
//        {
//            return value;
//        }

//        //public static string GetStringArgument(string c)
//        //{
//        //    return c;
//        //}

//        public static int GetMultiArgument(int value, float c, double a, long e)
//        {
//            return value + (int)c + (int)a + (int)e;
//        }

//        public static int GetIntArgumentCdecl(int value)
//        {
//            return value;
//        }
//#else
//        [DllImport(dllName, EntryPoint = "RuntimeApi_GetIntArgument")]
//        public static extern int GetIntArgument(int value);
        
//        //[DllImport(dllName, EntryPoint = "RuntimeApi_GetStringArgument")]
//        //public static extern string GetStringArgument(string value);
        
//        [DllImport(dllName, EntryPoint = "RuntimeApi_GetMultiArgument")]
//        public static extern int GetMultiArgument(int value, float c, double a, long e);

//        [DllImport(dllName, EntryPoint = "RuntimeApi_GetIntArgumentCdecl", CallingConvention = CallingConvention.Cdecl)]
//        public static extern int GetIntArgumentCdecl(int value);
//#endif
//    }

//    internal class TC_ExternalCall : GeneralTestCaseBase
//    {
//        [UnitTest]
//        public void TestIntArgument()
//        {
//            Assert.Equal(5, NativeDll.GetIntArgument(5));
//        }

//        //[UnitTest]
//        //public void TestStringArgument()
//        //{
//        //    string str = "Hello";
//        //    Assert.Equal(str, NativeDll.GetStringArgument(str));
//        //}

//        [UnitTest]
//        public void TestMultiArgument()
//        {
//            Assert.Equal(11, NativeDll.GetMultiArgument(5, 1.0f, 2.0, 3));
//        }

//        [UnitTest]
//        public void TestIntArgumentCdecl()
//        {
//            Assert.Equal(5, NativeDll.GetIntArgumentCdecl(5));
//        }
//    }
//}
