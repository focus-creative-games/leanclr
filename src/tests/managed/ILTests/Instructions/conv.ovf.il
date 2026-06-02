.assembly extern mscorlib { }
.assembly ILTests.Native { }

.class public auto ansi TestConvOvf
{
    .method public static int8 TestConvOvfI1(int32 a) cil managed
    {
        .maxstack 1
        ldarg.0
        conv.ovf.i1
        ret
    }

    .method public static uint8 TestConvOvfU1Un(int32 a) cil managed
    {
        .maxstack 1
        ldarg.0
        conv.ovf.u1.un
        ret
    }
}
