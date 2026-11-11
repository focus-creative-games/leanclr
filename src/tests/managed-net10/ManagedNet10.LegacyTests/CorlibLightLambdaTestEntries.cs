namespace ManagedNet10.LegacyTests
{
    internal static partial class Program
    {
        public static void RunCorlibLightLambda()
        {
            LegacyTestRunner.RunType(typeof(CorlibTests.InternalCall.TC_LightLambda));
            LegacyTestRunner.RunType(typeof(CorlibLightLambdaNet10Semantics));
        }
    }

    internal sealed class CorlibLightLambdaNet10Semantics
    {
        private delegate int InIntBinary(in int a, in int b);

        [UnitTest]
        public void CompileStaticCallLambda()
        {
            System.Linq.Expressions.ParameterExpression left =
                System.Linq.Expressions.Expression.Parameter(typeof(int), "left");
            System.Linq.Expressions.ParameterExpression right =
                System.Linq.Expressions.Expression.Parameter(typeof(int), "right");
            System.Reflection.MethodInfo sum = typeof(CorlibTests.InternalCall.TC_LightLambda).GetMethod(
                "Sum",
                System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Static);

            Assert.NotNull(sum);

            System.Linq.Expressions.Expression<System.Func<int, int, int>> lambda =
                System.Linq.Expressions.Expression.Lambda<System.Func<int, int, int>>(
                    System.Linq.Expressions.Expression.Call(sum, left, right),
                    left,
                    right);
            System.Func<int, int, int> compiled = lambda.Compile();

            Assert.Equal(7, compiled(3, 4));
        }

        [UnitTest]
        public void CompileStaticCallLambdaWithInArguments()
        {
            System.Linq.Expressions.ParameterExpression left =
                System.Linq.Expressions.Expression.Parameter(typeof(int).MakeByRefType(), "left");
            System.Linq.Expressions.ParameterExpression right =
                System.Linq.Expressions.Expression.Parameter(typeof(int).MakeByRefType(), "right");
            System.Reflection.MethodInfo sum = typeof(CorlibTests.InternalCall.TC_LightLambda).GetMethod(
                "Sum2",
                System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Static);

            Assert.NotNull(sum);

            System.Linq.Expressions.Expression<InIntBinary> lambda =
                System.Linq.Expressions.Expression.Lambda<InIntBinary>(
                    System.Linq.Expressions.Expression.Call(sum, left, right),
                    left,
                    right);
            InIntBinary compiled = lambda.Compile();
            int a = 5;
            int b = 6;

            Assert.Equal(11, compiled(in a, in b));
        }
    }
}
