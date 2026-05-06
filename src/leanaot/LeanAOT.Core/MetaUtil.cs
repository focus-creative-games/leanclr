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
using System.Diagnostics;
using System.Text;

namespace LeanAOT.Core
{

    public static class MetaUtil
    {
        public static string GetModuleNameWithoutExt(string moduleName)
        {
            return Path.GetFileNameWithoutExtension(moduleName);
        }

        public static string GetModuleNameWithoutExt(ModuleDef mod)
        {
            return mod.Assembly.Name;
        }

        public static (string, string) SplitNamespaceAndName(string fullName)
        {
            int index = fullName.LastIndexOf('/');
            if (index == -1)
            {
                int index2 = fullName.IndexOf('.');
                return index2 >= 0 ? (fullName.Substring(0, index2), fullName.Substring(index2 + 1)) : ("", fullName);
            }
            return ("", fullName.Substring(index + 1));
        }

        public static bool IsVoidType(TypeSig type)
        {
            return type.RemovePinnedAndModifiers().ElementType == ElementType.Void;
        }

        public static TypeDef GetBaseTypeDef(TypeDef type)
        {
            ITypeDefOrRef baseType = type.BaseType;
            if (baseType == null)
            {
                return null;
            }
            TypeDef baseTypeDef = baseType.ResolveTypeDef();
            if (baseTypeDef != null)
            {
                return baseTypeDef;
            }
            if (baseType is TypeSpec baseTypeSpec)
            {
                GenericInstSig genericIns = baseTypeSpec.TypeSig.ToGenericInstSig();
                return genericIns.GenericType.TypeDefOrRef.ResolveTypeDefThrow();
            }
            else
            {
                throw new Exception($"GetBaseTypeDef: {type} fail");
            }
        }

        public static TypeDef GetTypeDefOrGenericTypeBaseThrowException(ITypeDefOrRef type)
        {
            if (type.IsTypeDef)
            {
                return (TypeDef)type;
            }
            if (type.IsTypeRef)
            {
                return type.ResolveTypeDefThrow();
            }
            if (type.IsTypeSpec)
            {
                GenericInstSig gis = type.TryGetGenericInstSig();
                return gis.GenericType.ToTypeDefOrRef().ResolveTypeDefThrow();
            }
            throw new NotSupportedException($"{type}");
        }

        public static TypeDef GetTypeDefOrGenericTypeBaseOrNull(ITypeDefOrRef type)
        {
            if (type.IsTypeDef)
            {
                return (TypeDef)type;
            }
            if (type.IsTypeRef)
            {
                return type.ResolveTypeDefThrow();
            }
            if (type.IsTypeSpec)
            {
                GenericInstSig gis = type.TryGetGenericInstSig();
                if (gis == null)
                {
                    return null;
                }
                return gis.GenericType.ToTypeDefOrRef().ResolveTypeDefThrow();
            }
            return null;
        }

        public static TypeDef GetMemberRefTypeDefParentOrNull(IMemberRefParent parent)
        {
            if (parent is TypeDef typeDef)
            {
                return typeDef;
            }
            if (parent is TypeRef typeRef)
            {
                return typeRef.ResolveTypeDefThrow();
            }
            if (parent is TypeSpec typeSpec)
            {
                GenericInstSig gis = typeSpec.TryGetGenericInstSig();
                if (gis == null)
                {
                    return null;
                }
                return gis.GenericType.TypeDefOrRef.ResolveTypeDefThrow();
            }
            return null;
        }

        public static bool IsAssignableFrom(TypeDef fromType, TypeDef toType)
        {
            TypeDef cur = fromType;
            while (true)
            {
                if (cur == toType)
                {
                    return true;
                }
                if (toType.IsInterface)
                {
                    foreach (var interfaceType in cur.Interfaces)
                    {
                        TypeDef interfaceTypeDef = interfaceType.Interface.ResolveTypeDef();
                        if (interfaceTypeDef != null && interfaceTypeDef == toType)
                        {
                            return true;
                        }
                    }
                }
                cur = GetBaseTypeDef(cur);
                if (cur == null)
                {
                    return false;
                }
            }
        }

        public static bool IsInheritFrom(TypeDef typeDef, string baseTypeFullName)
        {
            TypeDef cur = typeDef;
            while (cur != null)
            {
                if (cur.FullName == baseTypeFullName)
                {
                    return true;
                }
                cur = GetBaseTypeDef(cur);
            }
            return false;
        }

        public static bool IsInheritFromDOTSTypes(TypeDef typeDef)
        {
            TypeDef cur = typeDef;
            while (true)
            {
                if (cur.Namespace.StartsWith("Unity.Entities") ||
                    //cur.Namespace.StartsWith("Unity.Jobs") ||
                    cur.Namespace.StartsWith("Unity.Burst"))
                {
                    return true;
                }
                foreach (var interfaceType in cur.Interfaces)
                {
                    TypeDef interfaceTypeDef = interfaceType.Interface.ResolveTypeDef();
                    if (interfaceTypeDef != null && (interfaceTypeDef.Namespace.StartsWith("Unity.Entities") ||
                        //interfaceTypeDef.Namespace.StartsWith("Unity.Jobs") ||
                        interfaceTypeDef.Namespace.StartsWith("Unity.Burst")))
                    {
                        return true;
                    }
                }

                cur = GetBaseTypeDef(cur);
                if (cur == null)
                {
                    return false;
                }
            }
        }

        public static bool IsInheritFromMonoBehaviour(TypeDef typeDef)
        {
            TypeDef cur = typeDef;
            while (true)
            {
                cur = GetBaseTypeDef(cur);
                if (cur == null)
                {
                    return false;
                }
                if (cur.Name == "MonoBehaviour" && cur.Namespace == "UnityEngine" && cur.Module.Name == "UnityEngine.CoreModule.dll")
                {
                    return true;
                }
            }
        }


        public static bool IsScriptType(TypeDef type)
        {
            for (TypeDef parentType = GetBaseTypeDef(type); parentType != null; parentType = GetBaseTypeDef(parentType))
            {
                if ((parentType.Name == "MonoBehaviour" || parentType.Name == "ScriptableObject")
                    && parentType.Namespace == "UnityEngine"
                    && parentType.Module.Assembly.Name == "UnityEngine.CoreModule")
                {
                    return true;
                }
            }

            return false;
        }

        public static bool IsSerializableType(TypeDef type)
        {
            return type.IsSerializable;
        }

        public static bool IsScriptOrSerializableType(TypeDef type)
        {
            return type.IsSerializable || IsScriptType(type);
        }

        public static bool IsSerializableTypeSig(TypeSig typeSig)
        {
            typeSig = typeSig.RemovePinnedAndModifiers();
            switch (typeSig.ElementType)
            {
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
                return true;
            case ElementType.Class:
                return IsScriptOrSerializableType(typeSig.ToTypeDefOrRef().ResolveTypeDefThrow());
            case ElementType.ValueType:
            {
                TypeDef typeDef = typeSig.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (typeDef.IsEnum)
                {
                    return true;
                }
                return typeDef.IsSerializable;
            }
            case ElementType.GenericInst:
            {
                GenericInstSig genericIns = typeSig.ToGenericInstSig();
                TypeDef typeDef = genericIns.GenericType.ToTypeDefOrRef().ResolveTypeDefThrow();
                return typeDef.FullName == "System.Collections.Generic.List`1" && IsSerializableTypeSig(genericIns.GenericArguments[0]);
            }
            case ElementType.SZArray:
            {
                return IsSerializableTypeSig(typeSig.RemovePinnedAndModifiers().Next);
            }
            default:
                return false;
            }
        }

        public static bool IsSerializableField(FieldDef field)
        {
            if (field.IsStatic)
            {
                return false;
            }
            var fieldSig = field.FieldSig.Type;
            if (field.IsPublic)
            {
                return IsSerializableTypeSig(fieldSig);
            }
            if (field.CustomAttributes.Any(c => c.TypeFullName == "UnityEngine.SerializeField"))
            {
                //UnityEngine.Debug.Assert(IsSerializableTypeSig(fieldSig));
                return true;
            }
            return false;
        }
        public static bool IsValueType(TypeSig typeSig)
        {
            var a = typeSig.RemovePinnedAndModifiers();
            switch (a.ElementType)
            {
            case ElementType.Void: return false;
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
            case ElementType.I:
            case ElementType.U:
            case ElementType.Ptr:
            case ElementType.ByRef:
            case ElementType.ValueType:
            case ElementType.FnPtr:
                return true;
            case ElementType.Var:
            case ElementType.MVar:
                return true;
            case ElementType.String:
            case ElementType.TypedByRef:
            case ElementType.Object:
            case ElementType.Sentinel:
            case ElementType.SZArray:
            case ElementType.Array:
            case ElementType.Class:
                return false;
            case ElementType.GenericInst:
            {
                var gia = (GenericInstSig)a;
                TypeDef typeDef = gia.GenericType.ToTypeDefOrRef().ResolveTypeDef();
                if (typeDef == null)
                {
                    throw new Exception($"type:{a} definition could not be found");
                }
                if (typeDef.IsEnum)
                {
                    return true;
                }
                return typeDef.IsValueType;
            }
            case ElementType.ValueArray: return true;
            case ElementType.Module: return false;
            default:
                throw new NotSupportedException(typeSig.ToString());
            }
        }

        public static bool IsEnumType(TypeSig typeSig)
        {
            return IsEnumType(typeSig.ToTypeDefOrRef());
        }

        public static bool IsEnumType(ITypeDefOrRef type)
        {
            TypeDef typeDef = type.ResolveTypeDef();
            return typeDef != null && typeDef.IsEnum;
        }

        public static bool IsDerivedFromMulticastDelegate(ITypeDefOrRef type)
        {
            TypeDef typeDef = type.ResolveTypeDef();
            if (typeDef == null)
            {
                return false;
            }
            TypeDef baseTypeDef = GetBaseTypeDef(typeDef);
            return baseTypeDef != null && baseTypeDef.FullName == "System.MulticastDelegate";
        }

        //public static bool ContainsContainsGenericParameter1(MethodDef method)
        //{
        //    Assert.IsTrue(!(method.DeclaringType.ContainsGenericParameter || method.MethodSig.ContainsGenericParameter));
        //    return false;
        //}

        public static bool ContainsContainsGenericParameter1(MethodSpec methodSpec)
        {
            if (methodSpec.GenericInstMethodSig.ContainsGenericParameter)
            {
                return true;
            }
            IMethodDefOrRef method = methodSpec.Method;
            if (method.IsMethodDef)
            {
                return false;// ContainsContainsGenericParameter1((MethodDef)method);
            }
            if (method.IsMemberRef)
            {
                return ContainsContainsGenericParameter1((MemberRef)method);
            }
            throw new Exception($"unknown method: {method}");
        }

        public static bool ContainsContainsGenericParameter1(MemberRef memberRef)
        {
            IMemberRefParent parent = memberRef.Class;
            if (parent is TypeSpec typeSpec)
            {
                return typeSpec.ContainsGenericParameter;
            }
            return false;
        }

        public static bool ContainsContainsGenericParameter(IMethod method)
        {
            Debug.Assert(method.IsMethod);
            if (method is MethodDef methodDef)
            {
                return false;
            }

            if (method is MethodSpec methodSpec)
            {
                return ContainsContainsGenericParameter1(methodSpec);
            }
            if (method is MemberRef memberRef)
            {
                return ContainsContainsGenericParameter1(memberRef);
            }
            throw new Exception($"unknown method: {method}");
        }



        public static TypeSig Inflate(TypeSig sig, GenericArgumentContext ctx)
        {
            if (ctx == null || !sig.ContainsGenericParameter)
            {
                return sig;
            }
            return ctx.Resolve(sig);
        }

        public static IList<TypeSig> TryInflate(IList<TypeSig> sig, GenericArgumentContext ctx)
        {
            if (sig == null || ctx == null)
            {
                return sig;
            }
            return sig.Select(s => Inflate(s, ctx)).ToList() ?? null;
        }

        public static GenericInstMethodSig InflateGenericInstMethodSig(GenericInstMethodSig gims, GenericArgumentContext gac)
        {
            var newGenericArguments = TryInflate(gims.GenericArguments, gac);
            if (newGenericArguments == gims.GenericArguments)
            {
                return gims;
            }
            return new GenericInstMethodSig(newGenericArguments);
        }

        public static IMethod InflateMethod(IMethod method, GenericArgumentContext gac)
        {
            if (method is MethodDef md)
            {
                return method;
            }
            if (method is MemberRef mr)
            {
                return new MemberRefUser(mr.Module, mr.Name, InflateMethodSig(mr.MethodSig, gac), Inflate(((ITypeDefOrRef)mr.Class).ToTypeSig(), gac).ToTypeDefOrRef());
            }
            if (method is MethodSpec ms)
            {
                var genericInstMethodSig = InflateGenericInstMethodSig(ms.GenericInstMethodSig, gac);
                if (ms.Method is MethodDef methodDef)
                {
                    return new MethodSpecUser(methodDef, genericInstMethodSig);
                }
                if (ms.Method is MemberRef memerRef)
                {
                    var inflatedMemberRef = new MemberRefUser(memerRef.Module, memerRef.Name, InflateMethodSig(memerRef.MethodSig, gac), Inflate(((ITypeDefOrRef)memerRef.Class).ToTypeSig(), gac).ToTypeDefOrRef());
                    return new MethodSpecUser(inflatedMemberRef, genericInstMethodSig);
                }
            }
            throw new Exception($"Unsupported IMethod type for inflation: {method.FullName}");
        }

        public static MethodSig InflateMethodSig(MethodSig methodSig, GenericArgumentContext genericArgumentContext)
        {
            var newReturnType = Inflate(methodSig.RetType, genericArgumentContext);
            var newParams = new List<TypeSig>();
            foreach (var param in methodSig.Params)
            {
                newParams.Add(Inflate(param, genericArgumentContext));
            }
            var newParamsAfterSentinel = new List<TypeSig>();
            if (methodSig.ParamsAfterSentinel != null)
            {
                throw new NotSupportedException($"methodSig.ParamsAfterSentinel is not supported: {methodSig}");
                //foreach (var param in methodSig.ParamsAfterSentinel)
                //{
                //    newParamsAfterSentinel.Add(Inflate(param, genericArgumentContext));
                //}
            }
            return new MethodSig(methodSig.CallingConvention, methodSig.GenParamCount, newReturnType, newParams, null);
        }

        public static IList<TypeSig> GetGenericArguments(IMemberRefParent type)
        {
            if (type is TypeDef typeDef)
            {
                return null;
            }
            if (type is TypeRef typeRef)
            {
                return null;
            }
            if (type is TypeSpec typeSpec)
            {
                GenericInstSig genericInstSig = typeSpec.TypeSig.ToGenericInstSig();
                return genericInstSig?.GenericArguments;
            }
            throw new NotSupportedException($"type:{type}");
        }

        public static IList<TypeSig> GetTypeGenericArguments(IMemberRefParent type)
        {
            if (type is TypeDef typeDef)
            {
                return null;
            }
            if (type is TypeRef typeRef)
            {
                return null;
            }
            if (type is TypeSpec typeSpec)
            {
                GenericInstSig genericInstSig = typeSpec.TypeSig.ToGenericInstSig();
                return genericInstSig?.GenericArguments;
            }
            throw new NotSupportedException($"type:{type}");
        }

        public static GenericArgumentContext GetTypeGenericArgumentContext(ITypeDefOrRef type)
        {
            return new GenericArgumentContext(GetTypeGenericArguments(type), null);
        }

        public static GenericArgumentContext GetMethodGenericArgumentContext(IMethod method)
        {
            if (method is MethodDef methodDef)
            {
                return new GenericArgumentContext(null, null);
            }
            if (method is MethodSpec methodSpec)
            {
                var methodGenericArguments = methodSpec.GenericInstMethodSig.GenericArguments;
                var methodBase = methodSpec.Method;
                if (methodBase is MethodDef methodDef2)
                {
                    return new GenericArgumentContext(null, methodGenericArguments);
                }
                if (methodBase is MemberRef memberRef2)
                {
                    return new GenericArgumentContext(GetTypeGenericArguments(memberRef2.Class), methodGenericArguments);
                }
                throw new NotSupportedException($"GetMethodGenericArgumentContext:{method}");
            }
            if (method is MemberRef memberRef)
            {
                return new GenericArgumentContext(GetTypeGenericArguments(memberRef.Class), null);
            }
            throw new NotSupportedException($"method:{method}");
        }

        public static GenericArgumentContext GetInflatedMemberRefGenericArgument(IMemberRefParent type, GenericArgumentContext ctx)
        {
            if (type is TypeDef typeDef)
            {
                return null;
            }
            if (type is TypeRef typeRef)
            {
                return null;
            }
            if (type is TypeSpec typeSpec)
            {
                GenericInstSig genericInstSig = typeSpec.TypeSig.ToGenericInstSig();
                if (genericInstSig == null)
                {
                    return ctx;
                }
                return new GenericArgumentContext(TryInflate(genericInstSig.GenericArguments, ctx), null);
            }
            throw new NotSupportedException($"type:{type}");
        }

        public static MethodSig GetInflatedMethodSig(IMethod method, GenericArgumentContext ctx)
        {
            if (method is MethodDef methodDef)
            {
                return methodDef.MethodSig;
            }
            if (method is MemberRef memberRef)
            {
                return InflateMethodSig(memberRef.MethodSig, GetInflatedMemberRefGenericArgument(memberRef.Class, ctx));
            }
            if (method is MethodSpec methodSpec)
            {
                var genericInstMethodSig = methodSpec.GenericInstMethodSig;
                if (methodSpec.Method is MethodDef methodDef2)
                {
                    return InflateMethodSig(methodDef2.MethodSig, new GenericArgumentContext(null, TryInflate(genericInstMethodSig.GenericArguments, ctx)));
                }
                if (methodSpec.Method is MemberRef memberRef2)
                {
                    return InflateMethodSig(memberRef2.MethodSig, new GenericArgumentContext(
                        GetInflatedMemberRefGenericArgument(memberRef2.Class, ctx)?.typeArgsStack,
                        TryInflate(genericInstMethodSig.GenericArguments, ctx)));
                }

            }
            throw new NotSupportedException($" method: {method}");
        }

        public static TypeSig InflateFieldSig(IField field, GenericArgumentContext ctx)
        {
            if (field is FieldDef fieldDef)
            {
                return fieldDef.FieldType;
            }
            if (field is MemberRef memberRef)
            {
                return Inflate(memberRef.FieldSig.Type, new GenericArgumentContext(TryInflate(GetGenericArguments(memberRef.Class), ctx), null));
            }

            throw new Exception($"unknown field:{field}");
        }

        public static IField InflateField(IField field, GenericArgumentContext ctx)
        {
            if (field is FieldDef fieldDef)
            {
                return field;
            }
            if (field is MemberRef memberRef)
            {
                var newFieldSigType = Inflate(memberRef.FieldSig.Type, new GenericArgumentContext(TryInflate(GetGenericArguments(memberRef.Class), ctx), null));
                return new MemberRefUser(memberRef.Module, memberRef.Name, new FieldSig(newFieldSigType), Inflate(((ITypeDefOrRef)memberRef.Class).ToTypeSig(), ctx).ToTypeDefOrRef());
            }
            throw new Exception($"unknown field:{field}");
        }

        public static TypeSig GetThisType(IMethod method)
        {
            TypeSig declaringType = method.DeclaringType.ToTypeSig();
            switch (declaringType.ElementType)
            {
            case ElementType.Object:
            case ElementType.String:
            case ElementType.Class:
            case ElementType.Array:
            case ElementType.SZArray:
                return declaringType;
            default:
            {
                TypeDef typeDef = declaringType.ToTypeDefOrRef().ResolveTypeDef();
                if (typeDef != null && typeDef.IsValueType)
                {
                    return new ByRefSig(declaringType);
                }
                else
                {
                    return declaringType;
                }
            }
            }
        }

        //public static ThisArgType GetThisArgType(IMethod method)
        //{
        //    if (!method.MethodSig.HasThis)
        //    {
        //        return ThisArgType.None;
        //    }
        //    if (method is MethodDef methodDef)
        //    {
        //        return methodDef.DeclaringType.IsValueType ? ThisArgType.ValueType : ThisArgType.Class;
        //    }
        //    if (method is MemberRef memberRef)
        //    {
        //        TypeDef typeDef = MetaUtil.GetMemberRefTypeDefParentOrNull(memberRef.Class);
        //        if (typeDef == null)
        //        {
        //            return ThisArgType.Class;
        //        }
        //        return typeDef.IsValueType ? ThisArgType.ValueType : ThisArgType.Class;
        //    }
        //    if (method is MethodSpec methodSpec)
        //    {
        //        return GetThisArgType(methodSpec.Method);
        //    }
        //    throw new NotSupportedException($" method: {method}");
        //}

        public static MethodSig ToSharedMethodSig(ICorLibTypes corTypes, MethodSig methodSig)
        {
            var newReturnType = methodSig.RetType;
            var newParams = new List<TypeSig>();
            foreach (var param in methodSig.Params)
            {
                newParams.Add(ToShareTypeSig(corTypes, param));
            }
            if (methodSig.ParamsAfterSentinel != null)
            {
                //foreach (var param in methodSig.ParamsAfterSentinel)
                //{
                //    newParamsAfterSentinel.Add(ToShareTypeSig(corTypes, param));
                //}
                throw new NotSupportedException($"methodSig.ParamsAfterSentinel is not supported: {methodSig}");
            }
            return new MethodSig(methodSig.CallingConvention, methodSig.GenParamCount, newReturnType, newParams, null);
        }

        public static TypeSig ToShareTypeSig(ICorLibTypes corTypes, TypeSig typeSig)
        {
            var a = typeSig.RemovePinnedAndModifiers();
            switch (a.ElementType)
            {
            case ElementType.Void: return corTypes.Void;
            case ElementType.Boolean: return corTypes.Byte;
            case ElementType.Char: return corTypes.UInt16;
            case ElementType.I1: return corTypes.SByte;
            case ElementType.U1: return corTypes.Byte;
            case ElementType.I2: return corTypes.Int16;
            case ElementType.U2: return corTypes.UInt16;
            case ElementType.I4: return corTypes.Int32;
            case ElementType.U4: return corTypes.UInt32;
            case ElementType.I8: return corTypes.Int64;
            case ElementType.U8: return corTypes.UInt64;
            case ElementType.R4: return corTypes.Single;
            case ElementType.R8: return corTypes.Double;
            case ElementType.String: return corTypes.Object;
            case ElementType.TypedByRef: return corTypes.TypedReference;
            case ElementType.I: return corTypes.IntPtr;
            case ElementType.U: return corTypes.UIntPtr;
            case ElementType.Object: return corTypes.Object;
            case ElementType.Sentinel: return typeSig;
            case ElementType.Ptr: return corTypes.UIntPtr;
            case ElementType.ByRef: return corTypes.UIntPtr;
            case ElementType.SZArray: return typeSig;
            case ElementType.Array: return typeSig;
            case ElementType.ValueType:
            {
                TypeDef typeDef = a.ToTypeDefOrRef().ResolveTypeDef();
                if (typeDef == null)
                {
                    throw new Exception($"type:{a} definition could not be found");
                }
                if (typeDef.IsEnum)
                {
                    return ToShareTypeSig(corTypes, typeDef.GetEnumUnderlyingType());
                }
                return typeSig;
            }
            case ElementType.Var:
            case ElementType.MVar:
            case ElementType.Class: return corTypes.Object;
            case ElementType.GenericInst:
            {
                var gia = (GenericInstSig)a;
                TypeDef typeDef = gia.GenericType.ToTypeDefOrRef().ResolveTypeDef();
                if (typeDef == null)
                {
                    throw new Exception($"type:{a} definition could not be found");
                }
                if (typeDef.IsEnum)
                {
                    return ToShareTypeSig(corTypes, typeDef.GetEnumUnderlyingType());
                }
                if (!typeDef.IsValueType)
                {
                    return corTypes.Object;
                }
                // il2cpp will raise error when try to share generic value type
                return typeSig;
                //return new GenericInstSig(gia.GenericType, gia.GenericArguments.Select(ga => ToShareTypeSig(corTypes, ga)).ToList());
            }
            case ElementType.FnPtr: return corTypes.UIntPtr;
            case ElementType.ValueArray: return typeSig;
            case ElementType.Module: return typeSig;
            default:
                throw new NotSupportedException(typeSig.ToString());
            }
        }


        public static void AppendIl2CppStackTraceNameOfTypeSig(StringBuilder sb, TypeSig typeSig)
        {
            typeSig = typeSig.RemovePinnedAndModifiers();

            switch (typeSig.ElementType)
            {
            case ElementType.Void: sb.Append("Void"); break;
            case ElementType.Boolean: sb.Append("Boolean"); break;
            case ElementType.Char: sb.Append("Char"); break;
            case ElementType.I1: sb.Append("SByte"); break;
            case ElementType.U1: sb.Append("Byte"); break;
            case ElementType.I2: sb.Append("Int16"); break;
            case ElementType.U2: sb.Append("UInt16"); break;
            case ElementType.I4: sb.Append("Int32"); break;
            case ElementType.U4: sb.Append("UInt32"); break;
            case ElementType.I8: sb.Append("Int64"); break;
            case ElementType.U8: sb.Append("UInt64"); break;
            case ElementType.R4: sb.Append("Single"); break;
            case ElementType.R8: sb.Append("Double"); break;
            case ElementType.String: sb.Append("String"); break;
            case ElementType.Ptr: AppendIl2CppStackTraceNameOfTypeSig(sb, typeSig.Next); sb.Append('*'); break;
            case ElementType.ByRef: AppendIl2CppStackTraceNameOfTypeSig(sb, typeSig.Next); sb.Append('&'); break;
            case ElementType.ValueType:
            case ElementType.Class:
            {
                var classOrValueTypeSig = (ClassOrValueTypeSig)typeSig;
                TypeDef typeDef = classOrValueTypeSig.TypeDefOrRef.ResolveTypeDef();
                if (typeDef == null)
                {
                    throw new Exception($"type:{classOrValueTypeSig} definition could not be found");
                }
                sb.Append(typeDef.Name);
                break;
            }
            case ElementType.GenericInst:
            {
                var genericInstSig = (GenericInstSig)typeSig;
                AppendIl2CppStackTraceNameOfTypeSig(sb, genericInstSig.GenericType);
                break;
            }
            case ElementType.Var:
            case ElementType.MVar:
            {
                var varSig = (GenericSig)typeSig;
                sb.Append(varSig.GenericParam.Name);
                break;
            }
            case ElementType.I: sb.Append("IntPtr"); break;
            case ElementType.U: sb.Append("UIntPtr"); break;
            case ElementType.FnPtr: sb.Append("IntPtr"); break;
            case ElementType.Object: sb.Append("Object"); break;
            case ElementType.SZArray:
            {
                var szArraySig = (SZArraySig)typeSig;
                AppendIl2CppStackTraceNameOfTypeSig(sb, szArraySig.Next);
                sb.Append("[]");
                break;
            }
            case ElementType.Array:
            {
                var arraySig = (ArraySig)typeSig;
                AppendIl2CppStackTraceNameOfTypeSig(sb, arraySig.Next);
                sb.Append('[');
                for (int i = 0; i < arraySig.Rank - 1; i++)
                {
                    sb.Append(',');
                }
                sb.Append(']');
                break;
            }
            case ElementType.TypedByRef: sb.Append("TypedReference"); break;
            default:
                throw new NotSupportedException(typeSig.ToString());
            }
        }

        public static TypeDef GetRootDeclaringType(TypeDef type)
        {
            TypeDef cur = type;
            while (true)
            {
                TypeDef declaringType = cur.DeclaringType;
                if (declaringType == null)
                {
                    return cur;
                }
                cur = declaringType;
            }
        }

        public static string CreateMethodDefIl2CppStackTraceSignature(MethodDef method)
        {
            var result = new StringBuilder();
            TypeDef declaringType = method.DeclaringType;

            string namespaze = GetRootDeclaringType(declaringType).Namespace;
            if (!string.IsNullOrEmpty(namespaze))
            {
                result.Append(namespaze);
                result.Append('.');
            }
            result.Append(declaringType.Name);
            result.Append(':');
            result.Append(method.Name);
            result.Append('(');

            int index = 0;
            foreach (TypeSig p in method.GetParams())
            {
                if (index > 0)
                {
                    result.Append(", ");
                }
                AppendIl2CppStackTraceNameOfTypeSig(result, p);
                ++index;
            }
            result.Append(')');
            return result.ToString();
        }

        public static bool IsCorlibOrSystemOrSystemCore(ModuleDef module)
        {
            UTF8String moduleName = module.Assembly.Name;
            return module.IsCoreLibraryModule == true || moduleName == "mscorlib" || moduleName == "System" || moduleName == "System.Core";
        }
    }
}
