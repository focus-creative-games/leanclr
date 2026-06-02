using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.Bugs
{
    internal class ValueTupleGeneric : GeneralTestCaseBase
    {
        class BaseRunner<T>
        {
        }
        class ValueTupleRunner<T1, T2> : BaseRunner<ValueTuple<T1, T2>>
        {
        }

        public static Type MakeGenericTypeWithName(Type baseGenericType, Type[] typeArguments)
        {
            if (!baseGenericType.IsGenericType)
                throw new Exception($"{baseGenericType.FullName} is not a generic type");
            var typeName = baseGenericType.FullName;
            while (!typeName.EndsWith("`"))
            {
                typeName = typeName.Remove(typeName.Length - 1, 1);
            }
            return baseGenericType.Assembly.GetType(typeName + typeArguments.Length).MakeGenericType(typeArguments);
        }


        [UnitTest]
        public void Bug()
        {
            var value = (33, "");
            var type = Activator.CreateInstance(MakeGenericTypeWithName(typeof(ValueTupleRunner<,>), value.GetType().GetGenericArguments()));
        }
    }
}
