.assembly extern mscorlib {}
.assembly InvalidLayouts {}
.module InvalidLayouts.dll
.class public explicit sealed BadAlignedRef extends [mscorlib]System.ValueType
{
    .size 16
    .field [1] public object Ref
}
.class public explicit sealed BadRefOverlap extends [mscorlib]System.ValueType
{
    .size 16
    .field [0] public object Ref
    .field [0] public int32 Scalar
}
.class public sequential sealed NestedRef extends [mscorlib]System.ValueType
{
    .field public object Ref
    .field public int32 Number
}
.class public explicit sealed BadNestedOverlap extends [mscorlib]System.ValueType
{
    .size 32
    .field [0] public valuetype NestedRef Nested
    .field [0] public int32 Scalar
}
.class public explicit sealed BadOffsetOverflow extends [mscorlib]System.ValueType
{
    .field [4294967294] public int64 Number
}
