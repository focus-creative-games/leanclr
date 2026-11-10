.assembly extern mscorlib {}
.assembly FilterFixture {}
.module FilterFixture.dll
.class public abstract sealed FilterFixture extends [mscorlib]System.Object
{
  .method public static int32 Test(int32 ok) cil managed
  {
    .maxstack 2
    .locals init (int32 result)
    .try
    {
      ldarg.0
      brtrue.s SUCCESS
      newobj instance void [mscorlib]System.InvalidOperationException::.ctor()
      throw
    SUCCESS:
      ldc.i4.1
      stloc.0
      leave.s DONE
    }
    filter
    {
      pop
      ldc.i4.1
      endfilter
    }
    {
      pop
      ldc.i4.2
      stloc.0
      leave.s DONE
    }
  DONE:
    ldloc.0
    ret
  }
}
