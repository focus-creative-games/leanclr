using dnlib.DotNet;
using LeanAOT.Core;

namespace LeanAOT.ToCpp
{

    public enum TypeNameRelaxLevel
    {
        ExactlyIncludeStringAndTypedByRef, // Generate the most specific type name possible, including string and typedbyref
        Exactly, // Generate the most specific type name possible
        EvalStackRelaxed, // Relax all reference types to leanclr::vm::RtObject*, , all pointer or ref types to void*, but keep primitive types unchanged
        AbiRelaxed, // all reference types to leanclr::vm::RtObject*, all pointer or ref types to void*
    }

    public class TypeNameService
    {
        private readonly MetadataService _metadataService;

        public TypeNameService(MetadataService metadataService)
        {
            _metadataService = metadataService;
        }


        private static readonly HashSet<string> _ptrLikeTypeNames = new HashSet<string>()
        {
            "System.IntPtr",
            "System.UIntPtr",
            "System.RuntimeTypeHandle",
            "System.RuntimeMethodHandle",
            "System.RuntimeFieldHandle",
        };

        public bool IsPtrLikeSystemValueType(TypeDef typeDef)
        {
            return MetaUtil.IsCorlibOrSystemOrSystemCore(typeDef.Module) && _ptrLikeTypeNames.Contains(typeDef.FullName);
        }

        public string GetCppTypeNameAsFieldOrArgOrLoc(TypeSig typeSig, TypeNameRelaxLevel relaxLevel)
        {
            typeSig = typeSig.RemovePinnedAndModifiers();
            switch (typeSig.ElementType)
            {
            case ElementType.Void:
                return "void";
            case ElementType.Boolean:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                case TypeNameRelaxLevel.AbiRelaxed:
                    return "bool";
                case TypeNameRelaxLevel.EvalStackRelaxed:
                    return "int32_t";
                }
                break;
            }
            case ElementType.Char:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                case TypeNameRelaxLevel.AbiRelaxed:
                    return "leanclr::Utf16Char";
                case TypeNameRelaxLevel.EvalStackRelaxed:
                    return "int32_t";
                }
                break;
            }
            case ElementType.I1:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                case TypeNameRelaxLevel.AbiRelaxed:
                    return "int8_t";
                case TypeNameRelaxLevel.EvalStackRelaxed:
                    return "int32_t";
                }
                break;
            }
            case ElementType.U1:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                case TypeNameRelaxLevel.AbiRelaxed:
                    return "uint8_t";
                case TypeNameRelaxLevel.EvalStackRelaxed:
                    return "int32_t";
                }
                break;
            }
            case ElementType.I2:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                case TypeNameRelaxLevel.AbiRelaxed:
                    return "int16_t";
                case TypeNameRelaxLevel.EvalStackRelaxed:
                    return "int32_t";
                }
                break;
            }
            case ElementType.U2:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                case TypeNameRelaxLevel.AbiRelaxed:
                    return "uint16_t";
                case TypeNameRelaxLevel.EvalStackRelaxed:
                    return "int32_t";
                }
                break;
            }
            case ElementType.I4:
                return "int32_t";
            case ElementType.U4:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                case TypeNameRelaxLevel.AbiRelaxed:
                    return "uint32_t";
                case TypeNameRelaxLevel.EvalStackRelaxed:
                    return "int32_t";
                }
                break;
            }
            case ElementType.I8:
                return "int64_t";
            case ElementType.U8:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                case TypeNameRelaxLevel.AbiRelaxed:
                    return "uint64_t";
                case TypeNameRelaxLevel.EvalStackRelaxed:
                    return "int64_t";
                }
                break;
            }
            case ElementType.R4:
                return ConstStrings.Float32TypeName;
            case ElementType.R8:
                return ConstStrings.Float64TypeName;
            case ElementType.String:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                    return $"{_metadataService.GetTypeDetail(typeSig.ToTypeDefOrRef()).InstanceTypeName}*";
                case TypeNameRelaxLevel.Exactly:
                    return ConstStrings.StringPtrTypeName;
                case TypeNameRelaxLevel.EvalStackRelaxed:
                case TypeNameRelaxLevel.AbiRelaxed:
                    return ConstStrings.ObjectPtrTypeName;
                }
                break;
            }
            case ElementType.Ptr:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                {
                    var ptrTypeSig = (typeSig as PtrSig).Next;
                    return GetCppTypeNameAsFieldOrArgOrLoc(ptrTypeSig, relaxLevel) + "*";
                }
                case TypeNameRelaxLevel.EvalStackRelaxed:
                case TypeNameRelaxLevel.AbiRelaxed:
                    return "void*";
                }
                break;
            }
            case ElementType.ByRef:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                {
                    var byRefTypeSig = (typeSig as ByRefSig).Next;
                    return GetCppTypeNameAsFieldOrArgOrLoc(byRefTypeSig, TypeNameRelaxLevel.Exactly) + "*";
                }
                case TypeNameRelaxLevel.EvalStackRelaxed:
                case TypeNameRelaxLevel.AbiRelaxed:
                    return "void*";
                }
                break;
            }
            case ElementType.ValueType:
            {
                ITypeDefOrRef type = typeSig.ToTypeDefOrRef();
                TypeDef typeDef = type.ResolveTypeDefThrow();
                if (typeDef.IsEnum)
                {
                    return GetCppTypeNameAsFieldOrArgOrLoc(typeDef.GetEnumUnderlyingType(), relaxLevel);
                }
                // if (relaxLevel == TypeNameRelaxLevel.AbiRelaxed && IsPtrLikeSystemValueType(typeDef))
                // {
                //     return "void*";
                // }
                return _metadataService.GetTypeDetail(type).InstanceTypeName;
            }
            case ElementType.Class:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                {
                    return $"{_metadataService.GetTypeDetail(typeSig.ToTypeDefOrRef()).InstanceTypeName}*";
                }
                case TypeNameRelaxLevel.EvalStackRelaxed:
                case TypeNameRelaxLevel.AbiRelaxed:
                    return ConstStrings.ObjectPtrTypeName;
                }
                break;
            }
            case ElementType.Var:
            case ElementType.MVar: throw new NotSupportedException("Generic type parameters are not supported in method parameters.");
            case ElementType.Array:
            case ElementType.SZArray:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                    return ConstStrings.ArrayPtrTypeName;
                case TypeNameRelaxLevel.EvalStackRelaxed:
                case TypeNameRelaxLevel.AbiRelaxed:
                    return ConstStrings.ObjectPtrTypeName;
                }
                break;
            }
            case ElementType.GenericInst:
            {
                var genericInstSig = (GenericInstSig)typeSig;
                TypeDef genericTypeDef = genericInstSig.GenericType.ToTypeDefOrRef().ResolveTypeDefThrow();
                if (genericTypeDef.IsEnum)
                {
                    return GetCppTypeNameAsFieldOrArgOrLoc(genericTypeDef.GetEnumUnderlyingType(), relaxLevel);
                }
                if (genericTypeDef.IsValueType)
                {
                    return $"{_metadataService.GetTypeDetail(typeSig.ToTypeDefOrRef()).InstanceTypeName}";
                }
                else
                {
                    switch (relaxLevel)
                    {
                    case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                    case TypeNameRelaxLevel.Exactly:
                        return $"{_metadataService.GetTypeDetail(typeSig.ToTypeDefOrRef()).InstanceTypeName}*";
                    case TypeNameRelaxLevel.EvalStackRelaxed:
                    case TypeNameRelaxLevel.AbiRelaxed:
                        return ConstStrings.ObjectPtrTypeName;
                    }
                    break;
                }
            }
            case ElementType.TypedByRef:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                    return _metadataService.GetTypeDetail(typeSig.ToTypeDefOrRef()).InstanceTypeName;
                default: return ConstStrings.TypedByRefTypeName;
                }
            }
            case ElementType.ValueArray:
                throw new NotSupportedException("ValueArray is not supported in method parameters.");
            case ElementType.I:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                case TypeNameRelaxLevel.EvalStackRelaxed:
                    return "intptr_t";
                case TypeNameRelaxLevel.AbiRelaxed:
                    return "void*";
                }
                break;
            }
            case ElementType.U:
            {
                switch (relaxLevel)
                {
                case TypeNameRelaxLevel.ExactlyIncludeStringAndTypedByRef:
                case TypeNameRelaxLevel.Exactly:
                case TypeNameRelaxLevel.EvalStackRelaxed:
                    return "uintptr_t";
                case TypeNameRelaxLevel.AbiRelaxed:
                    return "void*";
                }
                break;
            }
            case ElementType.R:
                return "double"; // Assuming R maps to double
            case ElementType.FnPtr:
                return "void*"; // Function pointers can be represented as void*
            case ElementType.Object:
                return ConstStrings.ObjectPtrTypeName;
            // Implement logic to generate a C++ type name from the .NET type signature
            default:
                throw new NotSupportedException($"ElementType {typeSig.ElementType} is not supported in method parameters.");
            }
            throw new NotSupportedException($"Unsupported TypeNameRelaxLevel: {relaxLevel}");
        }

        public string GetDirectlyCppTypeName(TypeSig typeSig)
        {
            switch (typeSig.ElementType)
            {
            case ElementType.Void:
                return "void";
            case ElementType.Boolean:
                return "bool";
            case ElementType.Char:
                return "leanclr::Utf16Char";
            case ElementType.I1:
                return "int8_t";
            case ElementType.U1:
                return "uint8_t";
            case ElementType.I2:
                return "int16_t";
            case ElementType.U2:
                return "uint16_t";
            case ElementType.I4:
                return "int32_t";
            case ElementType.U4:
                return "uint32_t";
            case ElementType.I8:
                return "int64_t";
            case ElementType.U8:
                return "uint64_t";
            case ElementType.R4:
                return ConstStrings.Float32TypeName;
            case ElementType.R8:
                return ConstStrings.Float64TypeName;
            case ElementType.String:
                return ConstStrings.StringTypeName;
            case ElementType.Ptr:
            {
                var ptrTypeSig = (typeSig as PtrSig).Next;
                return GetDirectlyCppTypeName(ptrTypeSig) + "*";
            }
            case ElementType.ByRef:
            {
                var byRefTypeSig = (typeSig as ByRefSig).Next;
                return GetDirectlyCppTypeName(byRefTypeSig);
            }
            case ElementType.ValueType:
            case ElementType.Class:
            case ElementType.GenericInst:
            {
                ITypeDefOrRef type = typeSig.ToTypeDefOrRef();
                TypeDef typeDef = type.ResolveTypeDefThrow();
                if (typeDef.IsEnum)
                {
                    return GetDirectlyCppTypeName(typeDef.GetEnumUnderlyingType());
                }
                return _metadataService.GetTypeDetail(type).InstanceTypeName;
            }
            case ElementType.Var:
            case ElementType.MVar: throw new NotSupportedException("Generic type parameters are not supported in method parameters.");
            case ElementType.Array:
            case ElementType.SZArray:
                return ConstStrings.ArrayTypeName;
            case ElementType.TypedByRef:
                return ConstStrings.TypedByRefTypeName;
            case ElementType.ValueArray:
                throw new NotSupportedException("ValueArray is not supported in method parameters.");
            case ElementType.I:
                return "intptr_t";
            case ElementType.U:
                return "uintptr_t";
            case ElementType.R:
                return "double"; // Assuming R maps to double
            case ElementType.FnPtr:
                return "void*"; // Function pointers can be represented as void*
            case ElementType.Object:
                return ConstStrings.ObjectTypeName;
            // Implement logic to generate a C++ type name from the .NET type signature
            default:
                throw new NotSupportedException($"ElementType {typeSig.ElementType} is not supported in method parameters.");
            }
        }
    }
}
