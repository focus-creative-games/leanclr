//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;
//

//namespace AOTDefs
//{

//    public struct IntVector3
//    {
//        public int a;
//        public int b;
//        public int c;
//    }

//    public class PreserveMethodBridges
//    {
//        public void F1(int a, IntVector3 b)
//        {

//        }

//        public unsafe int Run(bool x1, byte x2, sbyte x3, char x4, short x5, ushort x6,
//            int y1, uint y2, long y3, ulong y4, IntPtr y5, UIntPtr y6,
//            object a1, string a2,
//            AOT_Enum_byte b1, AOT_Enum_int b2, AOT_Enum_long b3,
//            UnityEngine.Vector2 c1, UnityEngine.Vector3 c2,
//            int* d1, TypedReference e1)
//        {
//            return y1;
//        }

//        public int Run2(int x, KeyValuePair<ValueTypeSize1, ValueTypeSize2> y)
//        {
//            return x;
//        }

//        public int Run3(Func<KeyValuePair<ValueTypeSize1, ValueTypeSize2>, int> func, KeyValuePair<ValueTypeSize1, ValueTypeSize2> x)
//        {
//            return func(x);
//        }

//        public static void Hello()
//        {

//            IEnumerable<(Vector3, sbyte, ushort)> arr = null;
//            var arr2 = arr.OrderBy(e => e.Item2);
//        }
//    }
//}
