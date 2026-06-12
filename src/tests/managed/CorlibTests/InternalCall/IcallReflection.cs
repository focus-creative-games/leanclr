using System;
using System.Reflection;

namespace CorlibTests.InternalCall
{
    /// <summary>
    /// Invoke internal-call methods on BCL types when no stable public API exists.
    /// </summary>
    internal static class IcallReflection
    {
        public static MethodInfo GetMethod(string typeFullName, string methodName, BindingFlags extraFlags, params Type[] parameterTypes)
        {
            Type type = ResolveType(typeFullName);
            var flags = BindingFlags.DeclaredOnly | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Static | BindingFlags.Instance | extraFlags;
            MethodInfo method = type.GetMethod(methodName, flags, null, parameterTypes, null);
            if (method != null)
                return method;
            string altName = methodName.EndsWith("()") ? methodName.Substring(0, methodName.Length - 2) : methodName + "()";
            return type.GetMethod(altName, flags, null, parameterTypes, null);
        }

        public static object Invoke(string typeFullName, string methodName, object instance, params object[] args)
        {
            Type type = ResolveType(typeFullName);
            var flags = BindingFlags.DeclaredOnly | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Static | BindingFlags.Instance;
            MethodInfo method = FindMethod(type, methodName, flags, args.Length);
            Assert.NotNull(method);
            return method.Invoke(method.IsStatic ? null : instance, args);
        }

        private static MethodInfo FindMethod(Type type, string methodName, BindingFlags flags, int argCount)
        {
            string bare = methodName.EndsWith("()") ? methodName.Substring(0, methodName.Length - 2) : methodName;
            foreach (var m in type.GetMethods(flags))
            {
                string name = m.Name;
                if (name != bare && name != methodName && name + "()" != methodName)
                    continue;
                if (m.GetParameters().Length != argCount)
                    continue;
                return m;
            }
            return null;
        }

        private static Type ResolveType(string typeFullName)
        {
            Type type = Type.GetType(typeFullName);
            if (type != null)
                return type;
            foreach (var asm in new[] { typeof(object).Assembly, typeof(Uri).Assembly })
            {
                type = asm.GetType(typeFullName, false, false);
                if (type != null)
                    return type;
            }
            throw new InvalidOperationException("Type not found: " + typeFullName);
        }
    }
}
