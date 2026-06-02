//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;

//namespace Main.AOTDefs
//{
//    public struct NativeArrayParamValueType
//    {
//        public int x;
//        public int y;
//        public long a;
//        public long b;

//        public int GetValue<T>(T a)
//        {
//            return GetValue0(default, default, default, default, default);
//        }

//        private int GetValue0(sbyte a1, double a2, Vector3 a3, float a4, Vector4 a5)
//        {
//            return x;
//        }

//        public int Run()
//        {
//            return GetValue(1);
//        }
//    }


//    public class NativeArrayParamClass
//    {
//        public int x;
//        public int y;
//        public long a;
//        public long b;

//        public int GetValue<T>(T a)
//        {
//            return GetValue0(default, default, default, default, default);
//        }

//        private int GetValue0(sbyte a1, double a2, Vector3 a3, float a4, Vector4 a5)
//        {
//            return x;
//        }

//        public int Run()
//        {
//            return GetValue(1);
//        }
//    }
//}
