.assembly extern System.Runtime
{
    .publickeytoken = (B0 3F 5F 7F 11 D5 0A 3A)
    .ver 10:0:0:0
}
.assembly ILTests.Native { }

.class public auto ansi TestInstructionOpcodes extends [System.Runtime]System.Object
{
    .method public static float64 CkfiniteDouble(float64) cil managed
    {
        .maxstack 1
        ldarg.0
        ckfinite
        ret
    }

    .method public static float32 CkfiniteFloat(float32) cil managed
    {
        .maxstack 1
        ldarg.0
        ckfinite
        conv.r4
        ret
    }

    .method private static int32 CalliAddOneTarget(int32) cil managed
    {
        .maxstack 2
        ldarg.0
        ldc.i4.1
        add
        ret
    }

    .method public static int32 CalliAddOne(int32) cil managed
    {
        .maxstack 2
        ldarg.0
        ldftn int32 TestInstructionOpcodes::CalliAddOneTarget(int32)
        calli int32(int32)
        ret
    }

    .method public static int32 BreakPassThrough(int32) cil managed
    {
        .maxstack 2
        break
        ldarg.0
        ldc.i4.1
        add
        ret
    }

    .method private static int32 JmpAddOneTarget(int32) cil managed
    {
        .maxstack 2
        ldarg.0
        ldc.i4.1
        add
        ret
    }

    .method public static int32 JmpAddOne(int32) cil managed
    {
        .maxstack 1
        ldarg.0
        jmp int32 TestInstructionOpcodes::JmpAddOneTarget(int32)
    }

    .method public static vararg int32 CountArglist(...) cil managed
    {
        .maxstack 1
        .locals init (
            valuetype [System.Runtime]System.ArgIterator args
        )

        ldloca.s args
        arglist
        call instance void [System.Runtime]System.ArgIterator::.ctor(valuetype [System.Runtime]System.RuntimeArgumentHandle)
        ldloca.s args
        call instance int32 [System.Runtime]System.ArgIterator::GetRemainingCount()
        ret
    }
}
