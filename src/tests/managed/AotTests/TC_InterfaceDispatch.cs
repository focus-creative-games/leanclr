using System;
using System.Runtime.CompilerServices;

namespace AotTests
{
    public class TC_InterfaceDispatch
    {
        public interface IValue { int Read(int divisor); void Add(int value); object Reference(); }
        public sealed class First : IValue
        {
            public int Value;
            public int Calls;
            public int Read(int divisor) { ++Calls; return Value / divisor; }
            public void Add(int value) { Value += value; }
            public object Reference() { return this; }
        }
        public sealed class Second : IValue
        {
            public int Read(int divisor) { return 20 / divisor; }
            public void Add(int value) { }
            public object Reference() { return null; }
        }
        public sealed class Interpreted : IValue
        {
            [AotMethod(false)] public int Read(int divisor) { return 30 / divisor; }
            [AotMethod(false)] public void Add(int value) { }
            [AotMethod(false)] public object Reference() { return this; }
        }
        public interface IGeneric { T Echo<T>(T value); }
        public sealed class Generic : IGeneric { public T Echo<T>(T value) { return value; } }
        public interface IOut<out T> { T Get(); }
        public sealed class Output : IOut<string> { public string Get() { return "ok"; } }
        public struct Boxed : IValue
        {
            public int Value;
            public int Read(int divisor) { return Value / divisor; }
            public void Add(int value) { Value += value; }
            public object Reference() { return null; }
        }
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static int Sum(IValue[] values, int count)
        {
            int result = 0;
            for (int i = 0; i < count; ++i) result += values[i % values.Length].Read(1);
            return result;
        }
        [UnitTest] public void Monomorphic()
        {
            First value = new First { Value = 7 };
            Assert.True(Sum(new IValue[] { value }, 20) == 140 && value.Calls == 20);
        }
        [UnitTest] public void Polymorphic()
        {
            First a = new First { Value = 7 }, b = new First { Value = 11 };
            Assert.True(Sum(new IValue[] { a, b, new Second() }, 30) == 380);
            Assert.True(a.Calls == 10 && b.Calls == 10);
        }
        [UnitTest] public void InterpreterTargets()
        {
            IValue value = new Interpreted();
            Assert.True(Sum(new IValue[] { value, new First { Value = 7 } }, 20) == 370);
            for (int i = 0; i < 10; ++i) { value.Add(i); Assert.True(value.Reference() == value); }
        }
        [UnitTest] public void VoidAndReferences()
        {
            First first = new First();
            IValue[] values = { first, new Second() };
            for (int i = 0; i < 20; ++i)
            {
                IValue value = values[i & 1];
                value.Add(1);
                Assert.True(value.Reference() == ((i & 1) == 0 ? first : null));
            }
            Assert.True(first.Value == 10);
        }
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static T GenericLoop<T>(IGeneric value, T input)
        {
            for (int i = 0; i < 10; ++i) input = value.Echo<T>(input);
            return input;
        }
        [UnitTest] public void GenericContexts()
        {
            Generic value = new Generic();
            // Static anchors register both implementations. The interface call
            // still has to preserve its actual target's generic context/ABI.
            Assert.True(value.Echo<int>(17) == 17 && value.Echo<string>("x") == "x");
            Assert.True(GenericLoop<int>(value, 19) == 19 && GenericLoop<string>(value, null) == null);
        }
        [UnitTest] public void Variance()
        {
            IOut<object> value = new Output();
            object result = null;
            for (int i = 0; i < 10; ++i) result = value.Get();
            Assert.True((string)result == "ok");
        }
        [UnitTest] public void BoxedReceiver()
        {
            IValue value = new Boxed { Value = 9 };
            for (int i = 0; i < 10; ++i) { value.Add(1); Assert.True(value.Read(1) == 10 + i); }
        }
        [UnitTest] public void WarmTargetThrows()
        {
            First first = new First { Value = 12 };
            IValue value = first;
            int result = 0;
            try { for (int i = 0; i < 4; ++i) result += value.Read(2 - i); }
            catch (DivideByZeroException) { result += 100; }
            Assert.True(result == 118 && first.Calls == 3);
        }
        [UnitTest] public void WarmTargetThenNull()
        {
            First first = new First { Value = 7 };
            IValue[] values = { first, first, null };
            int result = 0;
            try { for (int i = 0; i < 3; ++i) result += values[i].Read(1); }
            catch (NullReferenceException) { result += 100; }
            Assert.True(result == 114 && first.Calls == 2);
        }
        [MethodImpl(MethodImplOptions.NoInlining)]
        public static int Reenter(IValue value, int depth)
        {
            int result = 0;
            for (int i = 0; i < 3; ++i)
            {
                result += value.Read(1);
                if (depth > 0) result += Reenter(new Second(), depth - 1);
            }
            return result;
        }
        [UnitTest] public void RecursiveFrames()
        {
            First value = new First { Value = 7 };
            Assert.True(Reenter(value, 1) == 201 && value.Calls == 3);
        }
        public static int StraightLine(IValue value) { return value.Read(1); }
        public static int Budget(IValue value)
        {
            int result = 0;
            for (int i = 0; i < 10; ++i)
                result += value.Read(1) + value.Read(1) + value.Read(1) + value.Read(1) + value.Read(1);
            return result;
        }
        [UnitTest] public void CacheBudget()
        {
            First value = new First { Value = 7 };
            Assert.True(StraightLine(value) == 7 && Budget(value) == 350 && value.Calls == 51);
        }
    }
}
