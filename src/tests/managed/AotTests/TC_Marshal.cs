// using System;
// using System.Runtime.InteropServices;

// /// <summary>
// /// 覆盖 <see cref="Marshal"/> 中与结构封送相关的 API（与 <c>src/runtime/vm/marshal.cpp</c> 对齐）。
// /// 用例均为无参 <see cref="UnitTestAttribute"/> 方法，由 <see cref="App.Main"/> 反射执行。
// /// </summary>
// public class TC_Marshal
// {
//     [StructLayout(LayoutKind.Sequential)]
//     private struct BlittablePair
//     {
//         public int A;
//         public int B;
//     }

//     [StructLayout(LayoutKind.Sequential)]
//     private struct BlittableMixed
//     {
//         public short Lo;
//         public short Hi;
//         public int I;
//         public long L;
//     }

//     [StructLayout(LayoutKind.Sequential)]
//     private struct BlittableNested
//     {
//         public BlittablePair P;
//         public int Tag;
//     }

//     [StructLayout(LayoutKind.Sequential)]
//     private struct WithIntPtrField
//     {
//         public IntPtr P;
//         public int X;
//     }

//     [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
//     private struct WithUnicodeString
//     {
//         public int Id;
//         public string Name;
//     }

//     [StructLayout(LayoutKind.Sequential)]
//     private struct WithAnsiString
//     {
//         [MarshalAs(UnmanagedType.LPStr)]
//         public string Text;
//         public int Code;
//     }

//     [StructLayout(LayoutKind.Sequential)]
//     private struct WithBstrField
//     {
//         [MarshalAs(UnmanagedType.BStr)]
//         public string Caption;
//         public int N;
//     }

//     [StructLayout(LayoutKind.Sequential)]
//     private struct WithNestedNonBlittable
//     {
//         public int Prefix;
//         public WithUnicodeString Inner;
//         public int Suffix;
//     }

//     [UnitTest]
//     public void UnitTest_SizeOf_BlittableStruct()
//     {
//         Assert.Equal(8, Marshal.SizeOf(typeof(BlittablePair)));
//         Assert.Equal(16, Marshal.SizeOf(typeof(BlittableMixed)));
//         Assert.Equal(12, Marshal.SizeOf(typeof(BlittableNested)));
//     }

//     [UnitTest]
//     public void UnitTest_OffsetOf_Fields()
//     {
//         Assert.Equal(0, Marshal.OffsetOf(typeof(BlittablePair), "A").ToInt32());
//         Assert.Equal(4, Marshal.OffsetOf(typeof(BlittablePair), "B").ToInt32());
//         Assert.Equal(0, Marshal.OffsetOf(typeof(BlittableNested), "P").ToInt32());
//         Assert.Equal(8, Marshal.OffsetOf(typeof(BlittableNested), "Tag").ToInt32());
//     }

//     [UnitTest]
//     public void UnitTest_StructureToPtr_PtrToStructure_Blittable_Roundtrip()
//     {
//         int n = Marshal.SizeOf(typeof(BlittablePair));
//         IntPtr p = Marshal.AllocHGlobal(n);
//         try
//         {
//             var src = new BlittablePair { A = 11, B = -3 };
//             Marshal.StructureToPtr(src, p, false);

//             object boxed = new BlittablePair();
//             Marshal.PtrToStructure(p, boxed);
//             var dst = (BlittablePair)boxed;
//             Assert.Equal(11, dst.A);
//             Assert.Equal(-3, dst.B);
//         }
//         finally
//         {
//             Marshal.FreeHGlobal(p);
//         }
//     }

//     [UnitTest]
//     public void UnitTest_BlittableNested_Roundtrip()
//     {
//         int n = Marshal.SizeOf(typeof(BlittableNested));
//         IntPtr p = Marshal.AllocHGlobal(n);
//         try
//         {
//             var src = new BlittableNested
//             {
//                 P = new BlittablePair { A = 2, B = 5 },
//                 Tag = 99,
//             };
//             Marshal.StructureToPtr(src, p, false);

//             object boxed = new BlittableNested();
//             Marshal.PtrToStructure(p, boxed);
//             var dst = (BlittableNested)boxed;
//             Assert.Equal(2, dst.P.A);
//             Assert.Equal(5, dst.P.B);
//             Assert.Equal(99, dst.Tag);
//         }
//         finally
//         {
//             Marshal.FreeHGlobal(p);
//         }
//     }

//     [UnitTest]
//     public void UnitTest_IntPtrField_Roundtrip()
//     {
//         int n = Marshal.SizeOf(typeof(WithIntPtrField));
//         IntPtr p = Marshal.AllocHGlobal(n);
//         try
//         {
//             var src = new WithIntPtrField { P = new IntPtr(0x1234), X = 7 };
//             Marshal.StructureToPtr(src, p, false);

//             object boxed = new WithIntPtrField();
//             Marshal.PtrToStructure(p, boxed);
//             var dst = (WithIntPtrField)boxed;
//             Assert.Equal(new IntPtr(0x1234), dst.P);
//             Assert.Equal(7, dst.X);
//         }
//         finally
//         {
//             Marshal.FreeHGlobal(p);
//         }
//     }

//     [UnitTest]
//     public void UnitTest_String_Unicode_Default_Roundtrip()
//     {
//         int n = Marshal.SizeOf(typeof(WithUnicodeString));
//         IntPtr p = Marshal.AllocHGlobal(n);
//         try
//         {
//             var src = new WithUnicodeString { Id = 42, Name = "LeanCLR" };
//             Marshal.StructureToPtr(src, p, false);

//             object boxed = new WithUnicodeString();
//             Marshal.PtrToStructure(p, boxed);
//             var dst = (WithUnicodeString)boxed;
//             Assert.Equal(42, dst.Id);
//             Assert.Equal("LeanCLR", dst.Name);
//             Marshal.DestroyStructure(p, typeof(WithUnicodeString));
//         }
//         finally
//         {
//             Marshal.FreeHGlobal(p);
//         }
//     }

//     [UnitTest]
//     public void UnitTest_String_Ansi_MarshalAs_Roundtrip()
//     {
//         int n = Marshal.SizeOf(typeof(WithAnsiString));
//         IntPtr p = Marshal.AllocHGlobal(n);
//         try
//         {
//             var src = new WithAnsiString { Text = "ascii", Code = 1 };
//             Marshal.StructureToPtr(src, p, false);

//             object boxed = new WithAnsiString();
//             Marshal.PtrToStructure(p, boxed);
//             var dst = (WithAnsiString)boxed;
//             Assert.Equal("ascii", dst.Text);
//             Assert.Equal(1, dst.Code);
//             Marshal.DestroyStructure(p, typeof(WithAnsiString));
//         }
//         finally
//         {
//             Marshal.FreeHGlobal(p);
//         }
//     }

//     [UnitTest]
//     public void UnitTest_String_Bstr_MarshalAs_Roundtrip()
//     {
//         int n = Marshal.SizeOf(typeof(WithBstrField));
//         IntPtr p = Marshal.AllocHGlobal(n);
//         try
//         {
//             var src = new WithBstrField { Caption = "BSTR\u00A9", N = 3 };
//             Marshal.StructureToPtr(src, p, false);

//             object boxed = new WithBstrField();
//             Marshal.PtrToStructure(p, boxed);
//             var dst = (WithBstrField)boxed;
//             Assert.Equal("BSTR\u00A9", dst.Caption);
//             Assert.Equal(3, dst.N);
//             Marshal.DestroyStructure(p, typeof(WithBstrField));
//         }
//         finally
//         {
//             Marshal.FreeHGlobal(p);
//         }
//     }

//     [UnitTest]
//     public void UnitTest_NullString_Field_Roundtrip()
//     {
//         int n = Marshal.SizeOf(typeof(WithUnicodeString));
//         IntPtr p = Marshal.AllocHGlobal(n);
//         try
//         {
//             var src = new WithUnicodeString { Id = 0, Name = null };
//             Marshal.StructureToPtr(src, p, false);

//             object boxed = new WithUnicodeString();
//             Marshal.PtrToStructure(p, boxed);
//             var dst = (WithUnicodeString)boxed;
//             Assert.Equal(0, dst.Id);
//             Assert.Null(dst.Name);
//             Marshal.DestroyStructure(p, typeof(WithUnicodeString));
//         }
//         finally
//         {
//             Marshal.FreeHGlobal(p);
//         }
//     }

//     [UnitTest]
//     public void UnitTest_StructureToPtr_DeleteOld_Replaces_String()
//     {
//         int n = Marshal.SizeOf(typeof(WithUnicodeString));
//         IntPtr p = Marshal.AllocHGlobal(n);
//         try
//         {
//             var first = new WithUnicodeString { Id = 1, Name = "First" };
//             Marshal.StructureToPtr(first, p, false);

//             var second = new WithUnicodeString { Id = 2, Name = "Second" };
//             Marshal.StructureToPtr(second, p, true);

//             object boxed = new WithUnicodeString();
//             Marshal.PtrToStructure(p, boxed);
//             var dst = (WithUnicodeString)boxed;
//             Assert.Equal(2, dst.Id);
//             Assert.Equal("Second", dst.Name);
//             Marshal.DestroyStructure(p, typeof(WithUnicodeString));
//         }
//         finally
//         {
//             Marshal.FreeHGlobal(p);
//         }
//     }

//     [UnitTest]
//     public void UnitTest_NestedNonBlittable_Roundtrip_And_Destroy()
//     {
//         int n = Marshal.SizeOf(typeof(WithNestedNonBlittable));
//         IntPtr p = Marshal.AllocHGlobal(n);
//         try
//         {
//             var src = new WithNestedNonBlittable
//             {
//                 Prefix = 10,
//                 Inner = new WithUnicodeString { Id = 20, Name = "Inner" },
//                 Suffix = 30,
//             };
//             Marshal.StructureToPtr(src, p, false);

//             object boxed = new WithNestedNonBlittable();
//             Marshal.PtrToStructure(p, boxed);
//             var dst = (WithNestedNonBlittable)boxed;
//             Assert.Equal(10, dst.Prefix);
//             Assert.Equal(20, dst.Inner.Id);
//             Assert.Equal("Inner", dst.Inner.Name);
//             Assert.Equal(30, dst.Suffix);

//             Marshal.DestroyStructure(p, typeof(WithNestedNonBlittable));
//         }
//         finally
//         {
//             Marshal.FreeHGlobal(p);
//         }
//     }

//     [UnitTest]
//     public void UnitTest_PtrToStructure_TypeOverload()
//     {
//         int n = Marshal.SizeOf(typeof(BlittablePair));
//         IntPtr p = Marshal.AllocHGlobal(n);
//         try
//         {
//             var src = new BlittablePair { A = 100, B = 200 };
//             Marshal.StructureToPtr(src, p, false);

//             object o = Marshal.PtrToStructure(p, typeof(BlittablePair));
//             Assert.NotNull(o);
//             var dst = (BlittablePair)o;
//             Assert.Equal(100, dst.A);
//             Assert.Equal(200, dst.B);
//         }
//         finally
//         {
//             Marshal.FreeHGlobal(p);
//         }
//     }

//     [UnitTest]
//     public void UnitTest_DestroyStructure_Blittable_NoThrow()
//     {
//         int n = Marshal.SizeOf(typeof(BlittablePair));
//         IntPtr p = Marshal.AllocHGlobal(n);
//         try
//         {
//             var src = new BlittablePair { A = 1, B = 2 };
//             Marshal.StructureToPtr(src, p, false);
//             Marshal.DestroyStructure(p, typeof(BlittablePair));
//         }
//         finally
//         {
//             Marshal.FreeHGlobal(p);
//         }
//     }

//     [UnitTest]
//     public void UnitTest_SizeOf_NonBlittable_WithString()
//     {
//         int n = Marshal.SizeOf(typeof(WithUnicodeString));
//         Assert.IsTrue(n >= 4 + IntPtr.Size);
//     }

//     [UnitTest]
//     public void UnitTest_BlittableMixed_Roundtrip()
//     {
//         int n = Marshal.SizeOf(typeof(BlittableMixed));
//         IntPtr p = Marshal.AllocHGlobal(n);
//         try
//         {
//             var src = new BlittableMixed { Lo = 1, Hi = 2, I = 0x10203040, L = unchecked((long)0x8899AABB_CCDDEEFFUL) };
//             Marshal.StructureToPtr(src, p, false);
//             object boxed = new BlittableMixed();
//             Marshal.PtrToStructure(p, boxed);
//             var dst = (BlittableMixed)boxed;
//             Assert.Equal((short)1, dst.Lo);
//             Assert.Equal((short)2, dst.Hi);
//             Assert.Equal(0x10203040, dst.I);
//             Assert.Equal(unchecked((long)0x8899AABB_CCDDEEFFUL), dst.L);
//         }
//         finally
//         {
//             Marshal.FreeHGlobal(p);
//         }
//     }

//     [UnitTest]
//     public void UnitTest_AllocFree_CoTaskMem()
//     {
//         IntPtr p = Marshal.AllocCoTaskMem(64);
//         Assert.IsTrue(p != IntPtr.Zero);
//         Marshal.WriteInt32(p, 0, 0x11223344);
//         Assert.Equal(0x11223344, Marshal.ReadInt32(p, 0));
//         Marshal.FreeCoTaskMem(p);
//     }

//     [UnitTest]
//     public void UnitTest_StructureToPtr_Reject_ReferenceTypeObject()
//     {
//         Assert.ExpectException<Exception>(() =>
//         {
//             IntPtr p = Marshal.AllocHGlobal(16);
//             try
//             {
//                 Marshal.StructureToPtr("not-a-struct", p, false);
//             }
//             finally
//             {
//                 Marshal.FreeHGlobal(p);
//             }
//         });
//     }
// }
