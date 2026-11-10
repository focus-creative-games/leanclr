.assembly extern mscorlib {}
.assembly PortableMemory {}
.module PortableMemory.dll
.class public auto ansi PortableILFixture extends [mscorlib]System.Object
{
    .method public static int32 Run() cil managed
    {
        .maxstack 1
        br.s dispatch
    case0:
        ldc.i4.s 42
        ret
    case1:
        ldc.i4.s 43
        ret
    dispatch:
        ldc.i4.1
        switch (case0, case1)
        ldc.i4.m1
        ret
    }
    .method public static int64 Constant() cil managed
    {
        .maxstack 1
        ldc.i8 0x0123456789ABCDEF
        ret
    }
    .method public static float64 DoubleConstant() cil managed
    {
        .maxstack 1
        ldc.r8 12345.678
        ret
    }
}
