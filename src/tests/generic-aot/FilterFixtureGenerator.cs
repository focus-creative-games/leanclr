using dnlib.DotNet;
using dnlib.DotNet.Emit;

internal static class FilterFixtureGenerator
{
    public static void Generate(string corelibPath, string output)
    {
        using var corelib = ModuleDefMD.Load(corelibPath);
        var reference = new AssemblyRefUser(corelib.Assembly.Name, corelib.Assembly.Version,
            corelib.Assembly.PublicKeyToken, corelib.Assembly.Culture);
        using var module = new ModuleDefUser("FilterFixture.dll", Guid.NewGuid(), reference) { Kind = ModuleKind.Dll };
        new AssemblyDefUser("FilterFixture", new Version(1, 0, 0, 0)).Modules.Add(module);
        var type = new TypeDefUser("", "FilterFixture", module.CorLibTypes.Object.TypeDefOrRef)
        {
            Attributes = TypeAttributes.Public | TypeAttributes.Abstract | TypeAttributes.Sealed,
        };
        module.Types.Add(type);
        var method = new MethodDefUser("Test", MethodSig.CreateStatic(module.CorLibTypes.Int32, module.CorLibTypes.Int32),
            MethodImplAttributes.IL | MethodImplAttributes.Managed, MethodAttributes.Public | MethodAttributes.Static);
        type.Methods.Add(method);
        var body = method.Body = new CilBody { InitLocals = true, MaxStack = 2 };
        body.Variables.Add(new Local(module.CorLibTypes.Int32));
        var exception = new TypeRefUser(module, "System", "InvalidOperationException", reference);
        var constructor = new MemberRefUser(module, ".ctor", MethodSig.CreateInstance(module.CorLibTypes.Void), exception);
        var start = Instruction.Create(OpCodes.Ldarg_0);
        var success = Instruction.Create(OpCodes.Ldc_I4_1);
        var filter = Instruction.Create(OpCodes.Pop);
        var handler = Instruction.Create(OpCodes.Pop);
        var done = Instruction.Create(OpCodes.Ldloc_0);
        foreach (var instruction in new[]
        {
            start, Instruction.Create(OpCodes.Brtrue_S, success), Instruction.Create(OpCodes.Newobj, constructor), Instruction.Create(OpCodes.Throw),
            success, Instruction.Create(OpCodes.Stloc_0), Instruction.Create(OpCodes.Leave_S, done),
            filter, Instruction.Create(OpCodes.Ldc_I4_1), Instruction.Create(OpCodes.Endfilter),
            handler, Instruction.Create(OpCodes.Ldc_I4_2), Instruction.Create(OpCodes.Stloc_0), Instruction.Create(OpCodes.Leave_S, done),
            done, Instruction.Create(OpCodes.Ret),
        }) body.Instructions.Add(instruction);
        body.ExceptionHandlers.Add(new ExceptionHandler(ExceptionHandlerType.Filter)
        {
            TryStart = start, TryEnd = filter, FilterStart = filter, HandlerStart = handler, HandlerEnd = done,
        });
        module.Write(output);
    }
}
