namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibArrayBufferIntrinsics()
        {
            RunCorlibArray();
            RunCorlibBuffer();
            RunCorlibIntrinsicObject();
            RunCorlibIntrinsicString();
            RunCorlibIntrinsicSpanPlaceholder();
        }

        public static void RunCorlibArray()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Array));
        }

        public static void RunCorlibArrayClearIntArrayAllElements()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Array), "Clear_IntArray_AllElements");
        }

        public static void RunCorlibArrayClearIntArrayPartialRange()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Array), "Clear_IntArray_PartialRange");
        }

        public static void RunCorlibArrayClearByteArrayPartialRange()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Array), "Clear_ByteArray_PartialRange");
        }

        public static void RunCorlibArrayClearBoolArrayAllElements()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Array), "Clear_BoolArray_AllElements");
        }

        public static void RunCorlibArrayClearIntArrayZeroLengthNoOp()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Array), "Clear_IntArray_ZeroLength_NoOp");
        }

        public static void RunCorlibArrayClearEmptyIntArray()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Array), "ClearEmptyIntArray");
        }

        public static void RunCorlibArrayClearIntArrayAtEndWithLengthZero()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Array), "ClearIntArrayAtEndWithLengthZero");
        }

        public static void RunCorlibBuffer()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_System_Buffer));
        }

        public static void RunCorlibBufferByteLengthByteArray()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Buffer), "ByteLength_ByteArray");
        }

        public static void RunCorlibBufferByteLengthIntArray()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Buffer), "ByteLength_IntArray");
        }

        public static void RunCorlibBufferMemoryCopyByteArray()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Buffer), "MemoryCopy_ByteArray");
        }

        public static void RunCorlibBufferBlockCopyByteArrayDifferentArrays()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Buffer), "BlockCopy_ByteArray_DifferentArrays");
        }

        public static void RunCorlibBufferBlockCopyByteArraySameArrayOverlap()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Buffer), "BlockCopy_ByteArray_SameArray_Overlap");
        }

        public static void RunCorlibBufferBlockCopyIntArrayDifferentArrays()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Buffer), "BlockCopy_IntArray_DifferentArrays");
        }

        public static void RunCorlibBufferBlockCopyIntArraySameArrayOverlap()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Buffer), "BlockCopy_IntArray_SameArray_Overlap");
        }

        public static void RunCorlibBufferBlockCopyIntArrayPartialElementBytes()
        {
            LegacyTestRunner.RunMethod(typeof(CorlibTests.InternalCall.TC_System_Buffer), "BlockCopy_IntArray_PartialElementBytes");
        }

        public static void RunCorlibIntrinsicObject()
        {
            LegacyTestRunner.RunType(typeof(Tests.Intrinsic.TC_System_Object));
        }

        public static void RunCorlibIntrinsicObjectDefaultCtorCreatesInstance()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.Intrinsic.TC_System_Object), "DefaultCtor_CreatesInstance");
        }

        public static void RunCorlibIntrinsicString()
        {
            LegacyTestRunner.RunType(typeof(Tests.Intrinsic.TC_String));
        }

        public static void RunCorlibIntrinsicStringGetChars1()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.Intrinsic.TC_String), "GetChars1");
        }

        public static void RunCorlibIntrinsicStringGetStringDataOffset()
        {
            LegacyTestRunner.RunMethod(typeof(Tests.Intrinsic.TC_String), "GetStringDataOffset");
        }

        public static void RunCorlibIntrinsicSpanPlaceholder()
        {
            LegacyTestRunner.RunType(typeof(Tests.Intrinsic.TC_Syste_Span));
        }
    }
}
