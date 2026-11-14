using System;
using System.Collections.Generic;
using System.Reflection;
using System.Reflection.Emit;

// Decode the measured assembly itself; do not infer IL families from C# syntax.
public static class ILInventory
{
    public static void Main(string[] args)
    {
        Dictionary<ushort, OpCode> codes = new Dictionary<ushort, OpCode>();
        foreach (FieldInfo field in typeof(OpCodes).GetFields(BindingFlags.Public | BindingFlags.Static))
        {
            OpCode op = (OpCode)field.GetValue(null);
            codes[(ushort)op.Value] = op;
        }
        Assembly assembly = Assembly.LoadFrom(System.IO.Path.GetFullPath(args[0]));
        foreach (Type type in assembly.GetTypes())
        foreach (MethodInfo method in type.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.Instance | BindingFlags.DeclaredOnly))
        {
            MethodBody body = method.GetMethodBody();
            if (body == null) continue;
            byte[] il = body.GetILAsByteArray();
            for (int i = 0; i < il.Length; )
            {
                int offset = i;
                ushort value = il[i++];
                if (value == 0xfe) value = (ushort)(0xfe00 | il[i++]);
                OpCode op = codes[value];
                int size;
                switch (op.OperandType)
                {
                    case OperandType.InlineNone: size = 0; break;
                    case OperandType.ShortInlineI: case OperandType.ShortInlineVar: case OperandType.ShortInlineBrTarget: size = 1; break;
                    case OperandType.InlineVar: size = 2; break;
                    case OperandType.InlineI8: case OperandType.InlineR: size = 8; break;
                    case OperandType.InlineSwitch: size = 4 + 4 * BitConverter.ToInt32(il, i); break;
                    default: size = 4; break;
                }
                string target = "";
                if (op.OperandType == OperandType.InlineMethod)
                {
                    MethodBase called = method.Module.ResolveMethod(BitConverter.ToInt32(il, i), type.GetGenericArguments(), method.GetGenericArguments());
                    target = called.DeclaringType + "::" + called.Name;
                }
                Console.WriteLine(type.FullName + "::" + method.Name + "\t" + offset.ToString("X4") + "\t" + op.Name + "\t" + target);
                i += size;
            }
        }
    }
}
