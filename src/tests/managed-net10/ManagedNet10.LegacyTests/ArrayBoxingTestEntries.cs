namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunArrayInstructions()
        {
            RunArrayNewarrAndLength();
            RunArrayLoadElements();
            RunArrayLoadElementAddress();
            RunArrayStoreElements();
            RunArrayMultidimensional();
        }

        public static void RunArrayNewarrAndLength()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Arrays.TC_newarr),
                typeof(Tests.Instruments.Arrays.TC_ldlen));
        }

        public static void RunArrayLoadElements()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Arrays.TC_ldelem),
                typeof(Tests.Instruments.Arrays.TC_ldelem_i),
                typeof(Tests.Instruments.Arrays.TC_ldelem_i1),
                typeof(Tests.Instruments.Arrays.TC_ldelem_i2),
                typeof(Tests.Instruments.Arrays.TC_ldelem_i4),
                typeof(Tests.Instruments.Arrays.TC_ldelem_i8),
                typeof(Tests.Instruments.Arrays.TC_ldelem_r4),
                typeof(Tests.Instruments.Arrays.TC_ldelem_r8),
                typeof(Tests.Instruments.Arrays.TC_ldelem_ref),
                typeof(Tests.Instruments.Arrays.TC_ldelem_u1),
                typeof(Tests.Instruments.Arrays.TC_ldelem_u2),
                typeof(Tests.Instruments.Arrays.TC_ldelem_u4),
                typeof(Tests.Instruments.Arrays.TC_ldelem_u8),
                typeof(Tests.Instruments.Arrays.TC_ldelem_any_i1),
                typeof(Tests.Instruments.Arrays.TC_ldelem_any_i2),
                typeof(Tests.Instruments.Arrays.TC_ldelem_any_i4),
                typeof(Tests.Instruments.Arrays.TC_ldelem_any_i8),
                typeof(Tests.Instruments.Arrays.TC_ldelem_any_ref),
                typeof(Tests.Instruments.Arrays.TC_ldelem_any_u1),
                typeof(Tests.Instruments.Arrays.TC_ldelem_any_u2),
                typeof(Tests.Instruments.Arrays.TC_ldelem_any_u4),
                typeof(Tests.Instruments.Arrays.TC_ldelem_any_u8));
        }

        public static void RunArrayLoadElementAddress()
        {
            LegacyTestRunner.RunTypeWithNet10Replacements(typeof(Tests.Instruments.Arrays.TC_ldelema));
            LegacyTestRunner.RunType(typeof(ArrayAddressNet10Semantics));
        }

        public static void RunArrayStoreElements()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Arrays.TC_stelem_i),
                typeof(Tests.Instruments.Arrays.TC_stelem_i1),
                typeof(Tests.Instruments.Arrays.TC_stelem_i2),
                typeof(Tests.Instruments.Arrays.TC_stelem_i4),
                typeof(Tests.Instruments.Arrays.TC_stelem_i8),
                typeof(Tests.Instruments.Arrays.TC_stelem_r4),
                typeof(Tests.Instruments.Arrays.TC_stelem_r8),
                typeof(Tests.Instruments.Arrays.TC_stelem_ref),
                typeof(Tests.Instruments.Arrays.TC_stelem_struct),
                typeof(Tests.Instruments.Arrays.TC_stelem_any_i1),
                typeof(Tests.Instruments.Arrays.TC_stelem_any_i2),
                typeof(Tests.Instruments.Arrays.TC_stelem_any_i4),
                typeof(Tests.Instruments.Arrays.TC_stelem_any_i8),
                typeof(Tests.Instruments.Arrays.TC_stelem_any_ref));
        }

        public static void RunArrayMultidimensional()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Arrays.TC_MdArray_any),
                typeof(Tests.Instruments.Arrays.TC_MdArray_i1),
                typeof(Tests.Instruments.Arrays.TC_MdArray_i2),
                typeof(Tests.Instruments.Arrays.TC_MdArray_i4),
                typeof(Tests.Instruments.Arrays.TC_MdArray_i8),
                typeof(Tests.Instruments.Arrays.TC_MdArray_interp_class),
                typeof(Tests.Instruments.Arrays.TC_MdArray_object),
                typeof(Tests.Instruments.Arrays.TC_MdArray_u1),
                typeof(Tests.Instruments.Arrays.TC_MdArray_u2),
                typeof(Tests.Instruments.Arrays.TC_MdArray_u4),
                typeof(Tests.Instruments.Arrays.TC_MdArray_u8));
        }

        public static void RunArrayMdAny()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Arrays.TC_MdArray_any));
        }

        public static void RunArrayMdI1()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Arrays.TC_MdArray_i1));
        }

        public static void RunArrayMdI2()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Arrays.TC_MdArray_i2));
        }

        public static void RunArrayMdI4()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Arrays.TC_MdArray_i4));
        }

        public static void RunArrayMdI8()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Arrays.TC_MdArray_i8));
        }

        public static void RunArrayMdClass()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Arrays.TC_MdArray_interp_class));
        }

        public static void RunArrayMdObject()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Arrays.TC_MdArray_object));
        }

        public static void RunArrayMdU1()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Arrays.TC_MdArray_u1));
        }

        public static void RunArrayMdU2()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Arrays.TC_MdArray_u2));
        }

        public static void RunArrayMdU4()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Arrays.TC_MdArray_u4));
        }

        public static void RunArrayMdU8()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Arrays.TC_MdArray_u8));
        }

        public static void RunArrayMdManualSmoke()
        {
            int[,] values = new int[2, 3];
            values[1, 1] = 22;
            Assert.Equal(0, values[0, 0]);
            Assert.Equal(22, values[1, 1]);
        }

        public static void RunArrayMdManualDefaultOnly()
        {
            int[,] values = new int[2, 3];
            Assert.Equal(0, values[0, 0]);
        }

        public static void RunArrayMdManualSetOrigin()
        {
            int[,] values = new int[2, 3];
            values[0, 0] = 22;
            Assert.Equal(22, values[0, 0]);
        }

        public static void RunArrayMdManualSetOffset()
        {
            int[,] values = new int[2, 3];
            values[1, 1] = 22;
            Assert.Equal(22, values[1, 1]);
        }

        public static void RunBoxingInstructions()
        {
            LegacyTestRunner.RunTypes(
                typeof(Tests.Instruments.Boxs.TC_box),
                typeof(Tests.Instruments.Boxs.TC_unbox),
                typeof(Tests.Instruments.Boxs.TC_unbox_any));
        }

        public static void RunBox()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Boxs.TC_box));
        }

        public static void RunUnbox()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Boxs.TC_unbox));
        }

        public static void RunUnboxAny()
        {
            LegacyTestRunner.RunType(typeof(Tests.Instruments.Boxs.TC_unbox_any));
        }
    }

    internal sealed class ArrayAddressNet10Semantics
    {
        private struct StructA
        {
            public int Value;
        }

        private struct StructB
        {
            public int Value;
        }

        [UnitTest]
        public void LdelemaReinterpretedStructArrayThrowsArrayTypeMismatch()
        {
            StructA[] source = new StructA[] { new StructA { Value = 1 } };
            StructB[] reinterpreted = UnsafeUtility.As<StructA[], StructB[]>(ref source);
            Assert.ExpectException<System.ArrayTypeMismatchException>(() =>
            {
                ref StructB value = ref reinterpreted[0];
                value.Value = 2;
            });
        }
    }
}
