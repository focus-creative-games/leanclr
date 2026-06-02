
//using test;
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Numerics;
//using System.Runtime.CompilerServices;
//using System.Runtime.InteropServices;
//using System.Text;
//using System.Threading.Tasks;
//using Tests.Bugs.Bug20220927;

//namespace Tests.Mics
//{
//    public class TC_MethodBridge : GeneralTestCaseBase
//    {
//        [UnitTest]
//        public unsafe void TestAll()
//        {
//            var c = new PreserveMethodBridges();
//            int v = c.Run(true, 1, -1, 'a', -1, 1, 100, 1, -1, 1, IntPtr.Zero, UIntPtr.Zero, null, null, AOT_Enum_byte.A, AOT_Enum_int.A, AOT_Enum_long.A, UnityEngine.Vector2.zero, UnityEngine.Vector3.zero, null, default(TypedReference));
//            Assert.Equal(100, v);
//        }

//        [UnitTest]
//        public void TestManaged2NativeValueTypeGenericAndGenericParam()
//        {
//            var c = new PreserveMethodBridges();
//            int v = c.Run2(1, default);
//            Assert.Equal(1, v);
//        }

//        [UnitTest]
//        public void TestNative2ManagedValueTypeGenericAndGenericParam()
//        {
//            var c = new PreserveMethodBridges();
//            int v = c.Run3(x => 1, default);
//            Assert.Equal(1, v);
//        }

//        [UnitTest]
//        public void Native2MangedFreeStack()
//        {
//            const int count = 64 * 1024;
//            int v = CallDelegateFuncs.CallFuncN(() => 1, count);
//            Assert.Equal(count, v);
//        }

//        public enum A:sbyte
//        {
//            T1,
//            T2
//        }

//        public sealed class AComparer : IEqualityComparer<A>
//        {
//            public readonly static AComparer Instance = new AComparer();

//            public bool Equals(A x, A y)
//            {
//                return x == y;
//            }

//            public int GetHashCode(A obj)
//            {
//                return (int)obj;
//            }
//        }

//        [UnitTest]
//        void NativeManagedMissing()
//        {
//            var dict = new Dictionary<A, int>(new AComparer());
//            dict.Add(A.T1, 1);
//        }

//        public enum EAny
//        {
//            A,
//            B,
//        }

//        [UnitTest]
//        void Native2ManagedMissing2()
//        {
//            var arr = new List<EAny> { EAny.A, EAny.B };
//            Assert.True(arr.Exists(x => x == EAny.A));
//        }

//        public class GenericParent<T>
//        {
//            public enum InnterEnum
//            {
//                A,
//                B,
//            }
//        }

//        [UnitTest]
//        void Native2ManagedMissing3()
//        {
//            var arr = new List<GenericParent<int>.InnterEnum> { GenericParent<int>.InnterEnum.A, GenericParent<int>.InnterEnum.B };
//            Assert.True(arr.Exists(x => x == GenericParent<int>.InnterEnum.A));
//        }

//        [UnitTest]
//        public void DelegateMissing()
//        {
//            var arr = new List<(Vector3, sbyte, ushort)>()
//            {
//                ( new Vector3(1,2,3), 1, 1),
//                ( new Vector3(1,2,3), 3, 1),
//                ( new Vector3(1,2,3), 4, 1),
//                ( new Vector3(1,2,3), 1, 1),
//            };
//            var arr2 = arr.OrderBy(e => e.Item2).ToList();
//            Assert.Equal(4, arr2[3].Item2);

//        }

//        [UnitTest]
//        public void DelegateMissing2()
//        {
//            var arr = new (Vector3, sbyte, ushort)[1];
//            //{
//            //    ( new Vector3(1,2,3), 1, 1),
//            //};
//            var arr2 = arr.OrderBy(e => e.Item2).ToArray();
//            Assert.Equal(0, arr2[0].Item2);

//        }

//        [UnitTest]
//        public void DelegateMissing3()
//        {
//            var d = new Dictionary<string, float>()
//            {
//                { "a", 1f },
//                { "b", 2f },
//                { "c", 3f},
//            };
//            var arr = d.OrderBy(s => s.Key).ToList();
//            Assert.Equal(1f, arr[0].Value);

//        }

//        [UnitTest]
//        public void GetFieldSeqP3()
//        {
//            var a = new MBStructSeqP3() { a = 1, b = "hello"};
//            Assert.Equal(0x5, a.Compute((x) => x.b.Length));
//        }

//        [UnitTest]
//        public void GetFieldExpP3()
//        {
//            var a = new MBStructExpP3() { a = 1, b = "hello" };
//            Assert.Equal(0x5, a.Compute((x) => x.b.Length));
//        }

//        [UnitTest]
//        public void GetFieldAutoP3()
//        {
//            var a = new MBStructP3() { a = 1, b = "hello" };
//            Assert.Equal(0x5, a.Compute((x) => x.b.Length));
//        }

//        struct MBStructP4
//        {
//            public int a;
//            public string b;
//            public byte c;
//            public float d;
//            public sbyte e;
//            public long f;
//            public double g;
//        }

//        [DllImport("abc")]
//        static extern MBStructP4 Sum(MBStructP4 a, MBStructP4 b);
//    }
//}
