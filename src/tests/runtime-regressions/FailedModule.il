.assembly extern mscorlib {}
.assembly FailedModule {}
.module FailedModule.dll
.method private static specialname rtspecialname void .cctor() cil managed
{
    .maxstack 1
    ldstr "module initializer failed"
    newobj instance void [mscorlib]System.Exception::.ctor(string)
    throw
}
