namespace AotTests
{
    public class TC_PgoCaches
    {
        public interface IApply { int Step(int value); }
        public class First : IApply { public int Step(int value) { return value + 1; } }
        public class Second : IApply { public int Step(int value) { return value + 3; } }
        public class Third : IApply { public int Step(int value) { return value + 7; } }
        public class Fourth : IApply { public int Step(int value) { return value + 11; } }
        public class Fifth : IApply { public int Step(int value) { return value + 13; } }
        public static int Mono(IApply value, int count)
        {
            int sum = 0;
            for (int i = 0; i < count; ++i) sum = value.Step(sum);
            return sum;
        }
        public static int Bi(IApply first, IApply second, int count)
        {
            int sum = 0;
            for (int i = 0; i < count; ++i) sum = ((i & 1) == 0 ? first : second).Step(sum);
            return sum;
        }
        public static int Mega(IApply[] values, int count)
        {
            int sum = 0;
            for (int i = 0; i < count; ++i) sum = values[i % values.Length].Step(sum);
            return sum;
        }
        public static int Mixed(IApply first, IApply second, int count)
        {
            int sum = 0;
            for (int i = 0; i < count; ++i)
            {
                sum = first.Step(sum);
                sum = ((i & 1) == 0 ? first : second).Step(sum);
                sum = ((i & 1) == 0 ? first : second).Step(sum);
                sum = first.Step(sum);
            }
            return sum;
        }
        public static int HotLater(IApply first, IApply second)
        {
            int sum = 0;
            for (int i = 0; i < 200; ++i) sum = first.Step(sum);
            for (int i = 0; i < 200; ++i) sum = ((i & 1) == 0 ? first : second).Step(sum);
            for (int i = 0; i < 1000; ++i) sum = ((i & 1) == 0 ? first : second).Step(sum);
            return sum;
        }
        public static int Cold(IApply value, int count)
        {
            int sum = 0;
            for (int i = 0; i < count; ++i) sum = value.Step(sum);
            return sum;
        }
        public static int Phase(IApply first, IApply second, IApply third, IApply fourth)
        {
            int sum = 0;
            for (int i = 0; i < 200; ++i)
                sum = (i < 100 ? ((i & 1) == 0 ? first : second) : ((i & 1) == 0 ? third : fourth)).Step(sum);
            return sum;
        }
        public static void Run()
        {
            var first = new First(); var second = new Second();
            Assert.True(Mono(first, 200) == 200 && Bi(first, second, 200) == 400);
            Assert.True(Mega(new IApply[] { first, second, new Third(), new Fourth(), new Fifth() }, 200) == 1400);
            Assert.True(Mixed(first, second, 200) == 1200 && HotLater(first, second) == 2600);
            for (int i = 0; i < 100; ++i) Assert.True(Cold(first, 1) == 1);
            Assert.True(Phase(first, second, new Third(), new Fourth()) == 1100);
        }
    }
}
