using System;
using System.Linq.Expressions;
using System.Reflection;

namespace CorlibTests.InternalCall
{
    class TC_LightLambda
    {
        public static int Sum(int a, int b)
        {
            return a + b;
        }


        public static int Sum2(in int a, in int b)
        {
            return a + b;
        }

        // TODO, icall `System.Reflection.Emit.AssemblyBuilder::basic_init` is not implemented
        //[UnitTest]
        public void CreateLambda()
        {
            var eventType = typeof(Func<int, int, int>);
            var p1 = Expression.Parameter(typeof(int), "arg1");
            var p2 = Expression.Parameter(typeof(int), "arg2");
            var method = typeof(TC_LightLambda).GetMethod("Sum", BindingFlags.Static | BindingFlags.Public);
            var methodCallExpression = Expression.Call(method, p1, p2);
            var lambda = Expression.Lambda(eventType, methodCallExpression, p1, p2).Compile();
        }

        delegate int FunIn(in int a, in int b);

        // TODO, icall `System.Reflection.Emit.AssemblyBuilder::basic_init` is not implemented
        //[UnitTest]
        public void CreateLambdaInArgument()
        {
            var eventType = typeof(FunIn);
            var p1 = Expression.Parameter(typeof(int).MakeByRefType(), "arg1");
            var p2 = Expression.Parameter(typeof(int).MakeByRefType(), "arg2");
            var method = typeof(TC_LightLambda).GetMethod("Sum2", BindingFlags.Static | BindingFlags.Public);
            var methodCallExpression = Expression.Call(method, p1, p2);
            var lambda = Expression.Lambda(eventType, methodCallExpression, p1, p2).Compile();
        }
    }
}
