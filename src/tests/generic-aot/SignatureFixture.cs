using dnlib.DotNet;

// C# cannot spell return custom modifiers. Add ABI-neutral modopt wrappers to
// two real interface implementations; their declarations remain plain I4/U4.
string output = args[0] + ".modified";
using (var module = ModuleDefMD.Load(args[0]))
{
    if (args.Length >= 2 && args[1] == "--rewrite-only")
    {
        var corelib = module.GetAssemblyRefs().Single(reference => reference.Name == "System.Private.CoreLib");
        // Framework ilasm adds mscorlib for primitive signatures even when all
        // explicit references target CoreLib. Retarget that implicit reference.
        var implicitCorelib = module.CorLibTypes.AssemblyRef;
        implicitCorelib.Name = corelib.Name;
        implicitCorelib.Version = corelib.Version;
        implicitCorelib.PublicKeyOrToken = corelib.PublicKeyOrToken;
    }
    if (args.Length < 2 || args[1] != "--rewrite-only")
    {
    var fixture = module.Types.Single(type => type.Name == "GenericFixtures");
    var modifier = new TypeRefUser(module, "System.Runtime.CompilerServices", "IsConst", module.CorLibTypes.AssemblyRef);
    foreach (string name in new[] { "IntegerReader", "UnsignedReader" })
    {
        var method = fixture.NestedTypes.Single(type => type.Name == name).Methods.Single(method => method.Name == "Read");
        method.MethodSig.RetType = new CModOptSig(modifier, method.MethodSig.RetType);
    }
    }
    module.Write(output);
}
File.Move(output, args[0], true);
