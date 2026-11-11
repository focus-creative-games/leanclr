extern alias GenericArguments;
using System;
using GenericAotLibrary;
using ForeignReference = GenericArguments::GenericAotArguments.ReferenceValue;
using ForeignValue = GenericArguments::GenericCollision.Payload;

namespace AotTests
{
    public class TC_GenericEh
    {
        private static void Counts(int effects, int catches, int finals)
        {
            Assert.True(Eh.Effects == effects && Eh.Catches == catches && Eh.Finals == finals);
        }
        [UnitTest] public void Scalars()
        {
            Eh.Reset();
            Assert.True(Eh.Echo<int>(int.MinValue, 0) == int.MinValue);
            Assert.True(Eh.Echo<uint>(uint.MaxValue, 0) == uint.MaxValue);
            Assert.True(Eh.Echo<long>(long.MinValue, 0) == long.MinValue);
            Assert.True(Eh.Echo<ulong>(ulong.MaxValue, 0) == ulong.MaxValue);
            Assert.True(Eh.Echo<float>(-0.25f, 0) == -0.25f && Eh.Echo<double>(-0.125, 0) == -0.125);
            Assert.True(Eh.Echo<sbyte>(-128, 0) == -128 && Eh.Echo<ushort>(65535, 0) == 65535);
            Assert.True(Eh.Echo<bool>(true, 0) && Eh.Echo<char>('\u4E2D', 0) == '\u4E2D');
            Assert.True(float.IsNaN(Eh.Echo<float>(float.NaN, 0)));
            Assert.True(Eh.Echo<double>(double.NegativeInfinity, 0) == double.NegativeInfinity);
            Counts(12, 12, 12);
        }
        [UnitTest] public void References()
        {
            Eh.Reset();
            var value = new ForeignReference { Marker = 19 };
            Assert.True(Eh.Echo<ForeignReference>(value, 3) == value);
            Assert.True(Eh.Echo<ForeignReference>(value, 0) == value);
            Assert.True(Eh.Echo<string>(null, 0) == null && Eh.Echo<string>("hello", 0) == "hello");
            Counts(4, 3, 4);
        }
        [UnitTest] public void ConstructedReferences()
        {
            Eh.Reset();
            var value = new EhRef<ForeignValue> { Marker = 23 };
            Assert.True(Eh.Echo<EhRef<ForeignValue>>(value, 0) == value && value.Marker == 23);
            Counts(1, 1, 1);
        }
        [UnitTest] public void MutatedArguments()
        {
            Eh.Reset();
            var first = new ForeignReference { Marker = 1 };
            var replacement = new ForeignReference { Marker = 2 };
            Assert.True(Eh.Mutate<ForeignReference>(first, replacement, 0) == replacement);
            Assert.True(Eh.Mutate<int>(7, int.MinValue, 0) == int.MinValue);
            Counts(2, 2, 2);
        }
        [UnitTest] public void TypedCatches()
        {
            Eh.Reset();
            Assert.True(Eh.TypedCatch<int>(false) == 7 && Eh.TypedCatch<int>(true) == 11);
            Assert.True(Eh.TypedCatch<string>(true) == 11 && Eh.TypedCatch<ForeignValue>(true) == 11);
            Counts(4, 3, 4);
        }
        [UnitTest] public void NestedFinally()
        {
            Eh.Reset();
            Assert.True(Eh.Nested<ForeignReference>(0) == -1);
            Assert.True(Eh.Nested<ForeignReference>(3) == 4);
            Counts(2, 1, 4);
        }
        [UnitTest] public void FinallyReplacesReturn()
        {
            Eh.Reset();
            bool caught = false;
            try { Eh.FinallyThrows<int>(int.MinValue); }
            catch (InvalidOperationException) { caught = true; }
            Assert.True(caught);
            Counts(1, 0, 1);
        }
        [UnitTest] public void Rethrow()
        {
            Eh.Reset();
            bool caught = false;
            try { Eh.Rethrow<ForeignReference>(); }
            catch (EhError<ForeignReference>) { caught = true; }
            Assert.True(caught);
            Counts(1, 1, 1);
        }
        [UnitTest] public void ClassAndMethodContexts()
        {
            Eh.Reset();
            var first = new ForeignReference { Marker = 29 };
            var second = new ForeignReference { Marker = 31 };
            var box = new EhHolder<ForeignReference>(first, 3);
            Assert.True(box.Combine<uint>(second, uint.MaxValue, 0) == uint.MaxValue && box.Value == first);
            Assert.True(box.Combine<double>(second, -0.25, 3) == -0.25 && box.Value == second);
            Assert.True(EhHolder<ForeignReference>.Initializations == 1);
            Counts(3, 1, 3);
        }
        [UnitTest] public void ConstructorHandoff()
        {
            Eh.Reset();
            var box = new EhHolder<int>(int.MinValue, 0);
            Assert.True(box.Value == int.MinValue && box.Marker == -1 && EhHolder<int>.Initializations == 1);
            Counts(1, 1, 1);
        }
        [UnitTest] public void VoidAndArrays()
        {
            Eh.Reset();
            Eh.Void<int>(0);
            ForeignValue[] row = new ForeignValue[1];
            Assert.True(Eh.Array<ForeignValue>(new ForeignValue[][] { row }, 2) == row);
            Counts(2, 2, 2);
        }
        [UnitTest] public void PostCatchContinuation()
        {
            Eh.Reset();
            Assert.True(Eh.Continuation<int>(0) == 101 && Eh.Continuation<string>(0) == 102);
            Assert.True(Eh.Tails == 2);
            Counts(2, 2, 0);
        }
        [UnitTest] public void ConservativeFallbacks()
        {
            ForeignValue value = new ForeignValue { Marker = 37 };
            Assert.True(Eh.Echo<ForeignValue>(value, 0).Marker == 37);
            int number = 41;
            Assert.True(Eh.ByRef<int>(ref number) == 41 && Eh.Address<int>() == 8);
            var receiver = new EhValue<int>();
            Assert.True(receiver.Read(0) == -1);
            Assert.True(Eh.Filter<int>(43) == 43 && Eh.ComplexFinally<int>(47) == 47);
        }
    }
}
