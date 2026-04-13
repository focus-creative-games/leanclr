// Copyright 2025 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

using dnlib.DotNet;
using System.Text;

namespace LeanAOT.Core
{
    public static class NameUtil
    {
        public static string StandardizeName(string name)
        {
            var sb = new StringBuilder();
            foreach (var ch in name)
            {
                if (char.IsLetterOrDigit(ch) || ch == '_')
                {
                    sb.Append(ch);
                }
                else
                {
                    sb.Append('_');
                }
            }
            return sb.ToString();
        }

        public static string CreateFullQualifiedTypeName(TypeSig type)
        {
            var sb = new StringBuilder();
            AppendFullQualifiedTypeName(sb, type);
            return sb.ToString();
        }

        public static void AppendFullQualifiedTypeName(StringBuilder sb, TypeSig type)
        {
            type = type.RemovePinnedAndModifiers();
            switch (type.ElementType)
            {
            case ElementType.Void:
                sb.Append("void");
                break;
            case ElementType.Boolean:
                sb.Append("bool");
                break;
            case ElementType.Char:
                sb.Append("char");
                break;
            case ElementType.I1:
                sb.Append("sbyte");
                break;
            case ElementType.U1:
                sb.Append("byte");
                break;
            case ElementType.I2:
                sb.Append("short");
                break;
            case ElementType.U2:
                sb.Append("ushort");
                break;
            case ElementType.I4:
                sb.Append("int");
                break;
            case ElementType.U4:
                sb.Append("uint");
                break;
            case ElementType.I8:
                sb.Append("long");
                break;
            case ElementType.U8:
                sb.Append("ulong");
                break;
            case ElementType.R4:
                sb.Append("float");
                break;
            case ElementType.R8:
                sb.Append("double");
                break;
            case ElementType.String:
                sb.Append("string");
                break;
            case ElementType.Ptr:
            {
                AppendFullQualifiedTypeName(sb, type.Next);
                sb.Append('*');
                break;
            }
            case ElementType.ByRef:
            {
                AppendFullQualifiedTypeName(sb, type.Next);
                sb.Append('&');
                break;
            }
            case ElementType.ValueType:
            case ElementType.Class:
            {
                var classOrValueTypeSig = (ClassOrValueTypeSig)type;
                TypeDef typeDef = classOrValueTypeSig.TypeDefOrRef.ResolveTypeDef();
                sb.Append(MetaUtil.GetModuleNameWithoutExt(typeDef.Module));
                sb.Append('.');
                sb.Append(typeDef.FullName);
                break;
            }
            case ElementType.Array:
            {
                var arraySig = (ArraySig)type;
                AppendFullQualifiedTypeName(sb, arraySig.Next);
                sb.Append('[');
                for (int i = 0; i < arraySig.Rank - 1; i++)
                {
                    sb.Append(',');
                }
                sb.Append(']');
                break;
            }
            case ElementType.SZArray:
            {
                var szArraySig = (SZArraySig)type;
                AppendFullQualifiedTypeName(sb, szArraySig.Next);
                sb.Append("[]");
                break;
            }
            case ElementType.GenericInst:
            {
                var genericInstSig = (GenericInstSig)type;
                AppendFullQualifiedTypeName(sb, genericInstSig.GenericType);
                sb.Append('<');
                foreach (var arg in genericInstSig.GenericArguments)
                {
                    AppendFullQualifiedTypeName(sb, arg);
                    sb.Append(',');
                }
                sb.Append('>');
                break;
            }
            case ElementType.Var:
            {
                var genericVarSig = (GenericSig)type;
                sb.Append('!');
                sb.Append(genericVarSig.Number);
                break;
            }
            case ElementType.MVar:
            {
                var genericVarSig = (GenericSig)type;
                sb.Append("!!");
                sb.Append(genericVarSig.Number);
                break;
            }
            case ElementType.Object:
                sb.Append("object");
                break;
            case ElementType.TypedByRef:
                sb.Append("typedref");
                break;
            case ElementType.I:
                sb.Append("intptr");
                break;
            case ElementType.U:
                sb.Append("uintptr");
                break;
            case ElementType.R:
                sb.Append("real");
                break;
            case ElementType.FnPtr:
                sb.Append("fnptr");
                // throw new NotSupportedException("FnPtr is not supported");
                break;
            default:
                throw new NotSupportedException(type.ToString());
            }
        }

        public static void AppendCorlibTypeName(StringBuilder sb, TypeSig type)
        {
            type = type.RemovePinnedAndModifiers();
            switch (type.ElementType)
            {
            case ElementType.Void:
            case ElementType.Boolean:
            case ElementType.Char:
            case ElementType.I1:
            case ElementType.U1:
            case ElementType.I2:
            case ElementType.U2:
            case ElementType.I4:
            case ElementType.U4:
            case ElementType.I8:
            case ElementType.U8:
            case ElementType.R4:
            case ElementType.R8:
            case ElementType.String:
            case ElementType.Object:
            case ElementType.TypedByRef:
            case ElementType.I:
            case ElementType.U:
            case ElementType.ValueType:
            case ElementType.Class:
            {
                sb.Append(type.FullName);
                break;
            }
            case ElementType.Ptr:
            {
                AppendCorlibTypeName(sb, type.Next);
                sb.Append('*');
                break;
            }
            case ElementType.ByRef:
            {
                AppendCorlibTypeName(sb, type.Next);
                sb.Append('&');
                break;
            }
            case ElementType.Array:
            {
                var arraySig = (ArraySig)type;
                AppendCorlibTypeName(sb, arraySig.Next);
                sb.Append('[');
                for (int i = 0; i < arraySig.Rank - 1; i++)
                {
                    sb.Append(',');
                }
                sb.Append(']');
                break;
            }
            case ElementType.SZArray:
            {
                var szArraySig = (SZArraySig)type;
                AppendCorlibTypeName(sb, szArraySig.Next);
                sb.Append("[]");
                break;
            }
            case ElementType.GenericInst:
            {
                var genericInstSig = (GenericInstSig)type;
                AppendCorlibTypeName(sb, genericInstSig.GenericType);
                sb.Append('<');
                foreach (var arg in genericInstSig.GenericArguments)
                {
                    AppendCorlibTypeName(sb, arg);
                    sb.Append(',');
                }
                sb.Append('>');
                break;
            }
            // case ElementType.Var:
            // {
            //     var genericVarSig = (GenericSig)type;
            //     sb.Append('!');
            //     sb.Append(genericVarSig.Number);
            //     break;
            // }
            // case ElementType.MVar:
            // {
            //     var genericVarSig = (GenericSig)type;
            //     sb.Append("!!");
            //     sb.Append(genericVarSig.Number);
            //     break;
            // }
            // case ElementType.R:
            case ElementType.FnPtr:
                sb.Append("fnptr");
                // throw new NotSupportedException("FnPtr is not supported");
                break;
            default:
                throw new NotSupportedException(type.ToString());
            }
        }

        public static string GetICallFullMethodName(MethodDef methodDef)
        {
            var result = new StringBuilder();
            result.Append(methodDef.DeclaringType.FullName);
            result.Append("::");
            result.Append(methodDef.Name);
            result.Append('(');
            bool first = true;
            foreach (var param in methodDef.GetParams())
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    result.Append(',');
                }
                AppendCorlibTypeName(result, param);
            }
            result.Append(')');
            return result.ToString();
        }
    }
}
