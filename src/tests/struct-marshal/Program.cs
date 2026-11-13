using System;
using System.Runtime.InteropServices;
namespace StructMarshalTests
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Plain
    {
        public byte A;
        public int B;
        public double C;
        public IntPtr P;
    }
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct Packed
    {
        public byte A;
        public int B;
        public short C;
    }
    [StructLayout(LayoutKind.Explicit, Size = 16)]
    public struct Union
    {
        [FieldOffset(0)]
        public int I;
        [FieldOffset(0)]
        public float F;
        [FieldOffset(8)]
        public long L;
    }
    public enum Small : short
    {
        A = 123
    }
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode, Pack = 1)]
    public struct Converted
    {
        public byte Tag;
        public bool Flag;
        public char Ch;
        [MarshalAs(UnmanagedType.I1)]
        public bool Tiny;
        [MarshalAs(UnmanagedType.VariantBool)]
        public bool Variant;
        public Small Enum;
        public Packed Nested;
    }
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode, Pack = 1)]
    public struct Text
    {
        public byte Tag;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 6)]
        public string Inline;
        [MarshalAs(UnmanagedType.LPWStr)]
        public string Wide;
        [MarshalAs(UnmanagedType.LPStr)]
        public string Ansi;
        [MarshalAs(UnmanagedType.LPUTF8Str)]
        public string Utf8;
        [MarshalAs(UnmanagedType.BStr)]
        public string Bstr;
    }
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct Arrays
    {
        public byte Tag;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3, ArraySubType = UnmanagedType.I2)]
        public short[] Numbers;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2, ArraySubType = UnmanagedType.Bool)]
        public bool[] Flags;
    }
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct AnsiInline
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 4)]
        public string Value;
        public char Ch;
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct NestedText
    {
        public Text Value;
        public int Tail;
    }
    [StructLayout(LayoutKind.Sequential)]
    public class Formatted
    {
        public static int Calls;
        public int Number;
        [MarshalAs(UnmanagedType.LPWStr)]
        public string Value;
        public Formatted()
        {
            Calls++;
            Number = 99;
        }
    }
    [StructLayout(LayoutKind.Sequential)]
    public class FormattedChild : Formatted
    {
        public long Tail;
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct FailAfterString
    {
        [MarshalAs(UnmanagedType.LPWStr)]
        public string Value;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public int[] Values;
    }
    [StructLayout(LayoutKind.Auto)]
    public struct Invalid
    {
        public int Value;
    }
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct Fixed
    {
        public fixed long Data[4];
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct StringArray
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2, ArraySubType = UnmanagedType.LPWStr)]
        public string[] Values;
    }
    public static class Program
    {
        static void Check(bool value, string message)
        {
            if (!value)
                throw new Exception(message);
        }
        static void Eq(long a, long b, string message)
        {
            Check(a == b, message + ": " + a + " != " + b);
        }
        static IntPtr Alloc(int size)
        {
            IntPtr p = Marshal.AllocHGlobal(size + 16);
            for (int i = 0; i < size + 16; i++)
                Marshal.WriteByte(p, i, 0xCC);
            return p;
        }
        static void Guard(IntPtr p, int size)
        {
            for (int i = size; i < size + 16; i++)
                Eq(Marshal.ReadByte(p, i), 0xCC, "guard");
        }
        static int Size(Type t)
        {
            return Marshal.SizeOf(t);
        }
        public static void Primitive()
        {
            IntPtr p = Alloc(8);
            try
            {
                Marshal.StructureToPtr(12345, p, false);
                Eq((int)Marshal.PtrToStructure(p, typeof(int)), 12345, "int");
                Marshal.StructureToPtr(true, p, false);
                Eq(Marshal.ReadInt32(p), 1, "bool native");
                Check((bool)Marshal.PtrToStructure(p, typeof(bool)), "bool managed");
                Marshal.DestroyStructure(p, typeof(bool));
                Guard(p, 8);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void PlainLayout()
        {
            Eq(Size(typeof(Plain)), IntPtr.Size == 8 ? 24 : 24, "plain size");
            Eq((long)Marshal.OffsetOf(typeof(Plain), "B"), 4, "plain offset");
            int size = Size(typeof(Plain));
            IntPtr p = Alloc(size);
            try
            {
                var x = new Plain { A = 7, B = -123, C = 1.25, P = new IntPtr(1234) };
                Marshal.StructureToPtr(x, p, false);
                Eq(Marshal.ReadInt32(p, 4), -123, "native B");
                var y = (Plain)Marshal.PtrToStructure(p, typeof(Plain));
                Check(y.A == 7 && y.B == -123 && y.C == 1.25 && y.P == new IntPtr(1234), "plain roundtrip");
                Marshal.DestroyStructure(p, typeof(Plain));
                Guard(p, size);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void Packing()
        {
            Eq(Size(typeof(Packed)), 7, "packed size");
            Eq((long)Marshal.OffsetOf(typeof(Packed), "B"), 1, "packed offset");
            IntPtr p = Alloc(7);
            try
            {
                Marshal.StructureToPtr(new Packed { A = 1, B = 0x12345678, C = -9 }, p, false);
                Eq(Marshal.ReadInt32(p, 1), 0x12345678, "packed B");
                var y = (Packed)Marshal.PtrToStructure(p, typeof(Packed));
                Eq(y.C, -9, "packed C");
                Guard(p, 7);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void Explicit()
        {
            Eq(Size(typeof(Union)), 16, "union size");
            IntPtr p = Alloc(16);
            try
            {
                Marshal.StructureToPtr(new Union { I = 0x3F800000, L = 91 }, p, false);
                var y = (Union)Marshal.PtrToStructure(p, typeof(Union));
                Check(y.F == 1 && y.L == 91, "union");
                Guard(p, 16);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void ScalarConversions()
        {
            Eq(Size(typeof(Converted)), 19, "converted size");
            Eq((long)Marshal.OffsetOf(typeof(Converted), "Ch"), 5, "char offset");
            IntPtr p = Alloc(19);
            try
            {
                var x = new Converted
                {
                    Tag = 2,
                    Flag = true,
                    Ch = '\u4E2D',
                    Tiny = true,
                    Variant = true,
                    Enum = Small.A,
                    Nested = new Packed { A = 5, B = 19, C = 20 }
                };
                Marshal.StructureToPtr(x, p, false);
                Eq(Marshal.ReadInt32(p, 1), 1, "BOOL");
                Eq(Marshal.ReadInt16(p, 8), -1, "VARIANT_BOOL");
                var y = (Converted)Marshal.PtrToStructure(p, typeof(Converted));
                Check(y.Flag && y.Tiny && y.Variant && y.Ch == '\u4E2D' && y.Enum == Small.A && y.Nested.B == 19,
                      "converted");
                Guard(p, 19);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        static Text TextValue()
        {
            return new Text
            {
                Tag = 3,
                Inline = "abcdefghi",
                Wide = "wide\u4E2D",
                Ansi = "ascii",
                Utf8 = "utf8\u4E2D",
                Bstr = "a\0b"
            };
        }
        static void VerifyText(Text y)
        {
            Check(y.Tag == 3 && y.Inline == "abcde" && y.Wide == "wide\u4E2D" && y.Ansi == "ascii" &&
                      y.Utf8 == "utf8\u4E2D" && y.Bstr == "a\0b",
                  "text");
        }
        public static void Strings()
        {
            int size = 13 + 4 * IntPtr.Size;
            Eq(Size(typeof(Text)), size, "text size");
            IntPtr p = Alloc(size);
            try
            {
                Marshal.StructureToPtr(TextValue(), p, false);
                VerifyText((Text)Marshal.PtrToStructure(p, typeof(Text)));
                Guard(p, size);
                Marshal.DestroyStructure(p, typeof(Text));
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void InlineArrays()
        {
            Eq(Size(typeof(Arrays)), 15, "array size");
            IntPtr p = Alloc(15);
            try
            {
                Marshal.StructureToPtr(
                    new Arrays { Tag = 9, Numbers = new short[] { 1, 2, 3, 4 }, Flags = new bool[] { true, false } }, p,
                    false);
                Eq(Marshal.ReadInt16(p, 5), 3, "array native");
                Eq(Marshal.ReadInt32(p, 7), 1, "bool array native");
                var y = (Arrays)Marshal.PtrToStructure(p, typeof(Arrays));
                Check(y.Numbers.Length == 3 && y.Numbers[2] == 3 && y.Flags.Length == 2 && y.Flags[0] && !y.Flags[1],
                      "arrays");
                Marshal.DestroyStructure(p, typeof(Arrays));
                Guard(p, 15);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void Ansi()
        {
            Eq(Size(typeof(AnsiInline)), 5, "ANSI size");
            IntPtr p = Alloc(5);
            try
            {
                Marshal.StructureToPtr(new AnsiInline { Value = "abcdef", Ch = 'Z' }, p, false);
                var y = (AnsiInline)Marshal.PtrToStructure(p, typeof(AnsiInline));
                Check(y.Value == "abc" && y.Ch == 'Z', "ANSI");
                Guard(p, 5);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void DeleteOld()
        {
            int size = Size(typeof(Text));
            IntPtr p = Alloc(size);
            try
            {
                Marshal.StructureToPtr(TextValue(), p, false);
                for (int i = 0; i < 30; i++)
                    Marshal.StructureToPtr(TextValue(), p, true);
                VerifyText((Text)Marshal.PtrToStructure(p, typeof(Text)));
                Marshal.DestroyStructure(p, typeof(Text));
                Guard(p, size);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void NullStrings()
        {
            int size = Size(typeof(Text));
            IntPtr p = Alloc(size);
            try
            {
                Marshal.StructureToPtr(new Text(), p, false);
                var y = (Text)Marshal.PtrToStructure(p, typeof(Text));
                Check(y.Inline == "" && y.Wide == null && y.Ansi == null && y.Bstr == null, "null strings");
                Marshal.DestroyStructure(p, typeof(Text));
                Guard(p, size);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void NestedStrings()
        {
            int size = Size(typeof(NestedText));
            IntPtr p = Alloc(size);
            try
            {
                Marshal.StructureToPtr(new NestedText { Value = TextValue(), Tail = 77 }, p, false);
                var y = (NestedText)Marshal.PtrToStructure(p, typeof(NestedText));
                VerifyText(y.Value);
                Eq(y.Tail, 77, "nested tail");
                Marshal.DestroyStructure(p, typeof(NestedText));
                Guard(p, size);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void ExistingClass()
        {
            int size = Size(typeof(Formatted));
            IntPtr p = Alloc(size);
            try
            {
                Marshal.StructureToPtr(new Formatted { Number = 42, Value = "hello" }, p, false);
                var y = new Formatted();
                Marshal.PtrToStructure(p, y);
                Check(y.Number == 42 && y.Value == "hello", "existing class");
                var z = (Formatted)Marshal.PtrToStructure(p, typeof(Formatted));
                Check(z.Number == 42 && z.Value == "hello", "new class");
                Marshal.DestroyStructure(p, typeof(Formatted));
                Guard(p, size);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static unsafe void FixedBuffer()
        {
            Eq(Size(typeof(Fixed)), 32, "fixed size");
            Fixed x = new Fixed();
            x.Data[0] = 123;
            x.Data[3] = 456;
            IntPtr p = Alloc(32);
            try
            {
                Marshal.StructureToPtr(x, p, false);
                var y = (Fixed)Marshal.PtrToStructure(p, typeof(Fixed));
                Eq(y.Data[0], 123, "fixed first");
                Eq(y.Data[3], 456, "fixed last");
                Guard(p, 32);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void StringArrays()
        {
            int size = 2 * IntPtr.Size;
            Eq(Size(typeof(StringArray)), size, "string array size");
            IntPtr p = Alloc(size);
            try
            {
                Marshal.StructureToPtr(new StringArray { Values = new string[] { "one", "two" } }, p, false);
                GC.Collect();
                var y = (StringArray)Marshal.PtrToStructure(p, typeof(StringArray));
                Check(y.Values.Length == 2 && y.Values[0] == "one" && y.Values[1] == "two", "string array");
                Marshal.StructureToPtr(new StringArray { Values = new string[] { "three", null } }, p, true);
                y = (StringArray)Marshal.PtrToStructure(p, typeof(StringArray));
                Check(y.Values[0] == "three" && y.Values[1] == null, "string array replace");
                Marshal.DestroyStructure(p, typeof(StringArray));
                Guard(p, size);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void InvalidLayout()
        {
            bool caught = false;
            try
            {
                Size(typeof(Invalid));
            }
            catch (ArgumentException)
            {
                caught = true;
            }
            Check(caught, "auto layout rejected");
            caught = false;
            try { Size(typeof(Small)); } catch (ArgumentException) { caught = true; }
            Check(caught, "standalone enum rejected");
        }
        public static void GenericApis()
        {
            IntPtr p = Alloc(Size(typeof(Packed)));
            try
            {
                Marshal.StructureToPtr<Packed>(new Packed { A = 5, B = 7, C = 9 }, p, false);
                var y = Marshal.PtrToStructure<Packed>(p);
                Eq(y.B, 7, "generic");
                Marshal.WriteInt16(p, -11);
                Eq((short)Marshal.PtrToStructure<short>(p), -11, "generic short sign extension");
                Marshal.DestroyStructure<Packed>(p);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void ConstructorSemantics()
        {
            IntPtr p = Alloc(Size(typeof(Formatted)));
            try
            {
                Marshal.StructureToPtr(new Formatted { Number = 42, Value = "ctor" }, p, false);
                int before = Formatted.Calls;
                var x = (Formatted)Marshal.PtrToStructure(p, typeof(Formatted));
                Eq(Formatted.Calls - before, 1, "class constructor calls");
                Marshal.DestroyStructure(p, typeof(Formatted));
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void NullPointer()
        {
            Check(Marshal.PtrToStructure(IntPtr.Zero, typeof(Packed)) == null, "null pointer");
        }
        public static void FailureCleanup()
        {
            int size = Size(typeof(FailAfterString));
            IntPtr p = Alloc(size);
            try
            {
                Marshal.StructureToPtr(new FailAfterString { Value = "old", Values = new int[] { 1, 2 } }, p, false);
                for (int i = 0; i < 100; i++)
                {
                    bool caught = false;
                    try
                    {
                        Marshal.StructureToPtr(new FailAfterString { Value = "temporary", Values = new int[] { 3 } }, p,
                                               true);
                    }
                    catch (ArgumentException)
                    {
                        caught = true;
                    }
                    Check(caught, "bad array rejected");
                    var y = (FailAfterString)Marshal.PtrToStructure(p, typeof(FailAfterString));
                    Check(y.Value == "old" && y.Values[0] == 1 && y.Values[1] == 2, "failed write preserves resources");
                }
                Marshal.DestroyStructure(p, typeof(FailAfterString));
                Guard(p, size);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void Pressure()
        {
            for (int i = 0; i < 300; i++)
            {
                Strings();
                StringArrays();
                if (i % 10 == 0)
                    GC.Collect();
            }
        }
        public static void BooleanReads()
        {
            IntPtr p = Alloc(Size(typeof(Converted)));
            try
            {
                for (int i = 0; i < Size(typeof(Converted)); i++)
                    Marshal.WriteByte(p, i, 0);
                Marshal.WriteInt32(p, 1, 2);
                Marshal.WriteInt16(p, 8, 1);
                var y = (Converted)Marshal.PtrToStructure(p, typeof(Converted));
                Check(y.Flag && y.Variant, "noncanonical BOOL/VARIANT_BOOL");
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void InheritedClass()
        {
            int size = Size(typeof(FormattedChild));
            IntPtr p = Alloc(size);
            try
            {
                Marshal.StructureToPtr(new FormattedChild { Number = 51, Value = "base", Tail = 52 }, p, false);
                var y = (FormattedChild)Marshal.PtrToStructure(p, typeof(FormattedChild));
                Check(y.Number == 51 && y.Value == "base" && y.Tail == 52, "class inheritance");
                Marshal.DestroyStructure(p, typeof(FormattedChild));
                Guard(p, size);
            }
            finally
            {
                Marshal.FreeHGlobal(p);
            }
        }
        public static void Utf8Boundaries()
        {
            int size = Size(typeof(Text));
            IntPtr p = Alloc(size);
            IntPtr bytes = Marshal.AllocHGlobal(3);
            try
            {
                for (int i = 0; i < size; i++)
                    Marshal.WriteByte(p, i, 0);
                Marshal.WriteByte(bytes, 0, 0xE4);
                Marshal.WriteByte(bytes, 1, 0xB8);
                Marshal.WriteByte(bytes, 2, 0);
                Marshal.WriteIntPtr(p, 13 + 2 * IntPtr.Size, bytes);
                var y = (Text)Marshal.PtrToStructure(p, typeof(Text));
                Check(y.Utf8 == "\uFFFD", "truncated UTF-8 replacement");
                Marshal.WriteIntPtr(p, 13 + 2 * IntPtr.Size, IntPtr.Zero);
                Marshal.StructureToPtr(new Text { Utf8 = "a\uD800" }, p, false);
                y = (Text)Marshal.PtrToStructure(p, typeof(Text));
                Check(y.Utf8 == "a\uFFFD", "unpaired surrogate replacement");
                Marshal.DestroyStructure(p, typeof(Text));
                Guard(p, size);
            }
            finally
            {
                Marshal.FreeHGlobal(bytes);
                Marshal.FreeHGlobal(p);
            }
        }
        public static void All()
        {
            Primitive();
            PlainLayout();
            Packing();
            Explicit();
            ScalarConversions();
            Strings();
            InlineArrays();
            Ansi();
            DeleteOld();
            NullStrings();
            NestedStrings();
            ExistingClass();
            FixedBuffer();
            StringArrays();
            InvalidLayout();
            GenericApis();
            ConstructorSemantics();
            NullPointer();
            BooleanReads();
            InheritedClass();
            Utf8Boundaries();

        }
        public static void Main(string[] args)
        {
            if (args.Length == 0)
                All();
            else
                typeof(Program).GetMethod(args[0]).Invoke(null, null);
        }
    }
}
namespace StructMarshalTests
{
    // These fields test metadata decoding only; they do not enable these native conversions.
    public class MarshalMetadata
    {
        [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.I2)]
        public short[] Dynamic;
        [MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_I4)]
        public Array Safe;
        [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(CustomMarshaller), MarshalCookie = "marshal-cookie")]
        public object Custom;
    }
    public sealed class CustomMarshaller : ICustomMarshaler
    {
        public static ICustomMarshaler GetInstance(string cookie) { throw new NotSupportedException(); }
        public void CleanUpManagedData(object value) { throw new NotSupportedException(); }
        public void CleanUpNativeData(IntPtr value) { throw new NotSupportedException(); }
        public int GetNativeDataSize() { throw new NotSupportedException(); }
        public IntPtr MarshalManagedToNative(object value) { throw new NotSupportedException(); }
        public object MarshalNativeToManaged(IntPtr value) { throw new NotSupportedException(); }
    }
}
