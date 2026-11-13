using dnlib.DotNet;
using dnlib.DotNet.Emit;
var source = ModuleDefMD.Load(args[0]);
var module = new ModuleDefUser("CalliILTests", Guid.NewGuid(), source.CorLibTypes.AssemblyRef);
new AssemblyDefUser("CalliILTests", new Version(1, 0, 0, 0)).Modules.Add(module);
var importer = new Importer(module);
var type = new TypeDefUser("CalliILTests", "Program", module.CorLibTypes.Object.TypeDefOrRef) { Attributes = TypeAttributes.Public | TypeAttributes.Abstract | TypeAttributes.Sealed };
module.Types.Add(type);
TypeDef Source(string name) => source.Types.First(t => t.Name == name);
IMethod Method(string type, string name, int count) => importer.Import(Source(type).Methods.First(m => m.Name == name && m.MethodSig.Params.Count == count));
var receiver = importer.Import(Source("Receiver"));
var cell = importer.Import(Source("Cell"));
MethodDef Create(string name) { var m = new MethodDefUser(name, MethodSig.CreateStatic(module.CorLibTypes.Int32), MethodImplAttributes.IL | MethodImplAttributes.Managed, MethodAttributes.Public | MethodAttributes.Static) { Body = new CilBody { InitLocals = true, MaxStack = 8 } }; type.Methods.Add(m); return m; }
foreach (var name in new[] { "Instance", "ExplicitThis", "Virtual", "ValueThis", "BoxedVirtual", "NullThis" })
{
    var method = Create(name); var body = method.Body; var il = body.Instructions;
    var sig = MethodSig.CreateInstance(module.CorLibTypes.Int32, module.CorLibTypes.Int32);
    if (name == "NullThis")
    {
        il.Add(Instruction.Create(OpCodes.Ldnull)); il.Add(Instruction.Create(OpCodes.Ldftn, Method("Receiver", "Constant", 0))); sig = MethodSig.CreateInstance(module.CorLibTypes.Int32);
    }
    else if (name == "BoxedVirtual")
    {
        var local = new Local(new ValueTypeSig(cell)); body.Variables.Add(local);
        il.Add(Instruction.Create(OpCodes.Ldloca, local)); il.Add(Instruction.Create(OpCodes.Initobj, cell));
        il.Add(Instruction.Create(OpCodes.Ldloca, local)); il.Add(Instruction.CreateLdcI4(42)); il.Add(Instruction.Create(OpCodes.Stfld, importer.Import(Source("Cell").Fields.First(f => f.Name == "Value"))));
        il.Add(Instruction.Create(OpCodes.Ldloc, local)); il.Add(Instruction.Create(OpCodes.Box, cell)); il.Add(Instruction.Create(OpCodes.Dup));
        il.Add(Instruction.Create(OpCodes.Ldvirtftn, importer.Import(new MemberRefUser(module, "GetHashCode", MethodSig.CreateInstance(module.CorLibTypes.Int32), module.CorLibTypes.Object.TypeDefOrRef))));
        sig = MethodSig.CreateInstance(module.CorLibTypes.Int32);
    }
    else if (name == "ValueThis")
    {
        var local = new Local(new ValueTypeSig(cell)); body.Variables.Add(local);
        il.Add(Instruction.Create(OpCodes.Ldloca, local)); il.Add(Instruction.Create(OpCodes.Initobj, cell));
        il.Add(Instruction.Create(OpCodes.Ldloca, local)); il.Add(Instruction.CreateLdcI4(40)); il.Add(Instruction.Create(OpCodes.Stfld, importer.Import(Source("Cell").Fields.First(f => f.Name == "Value"))));
        il.Add(Instruction.Create(OpCodes.Ldloca, local)); il.Add(Instruction.CreateLdcI4(2)); il.Add(Instruction.Create(OpCodes.Ldftn, Method("Cell", "Add", 1)));
    }
    else
    {
        var local = new Local(receiver.ToTypeSig()); body.Variables.Add(local);
        il.Add(Instruction.CreateLdcI4(40)); il.Add(Instruction.Create(OpCodes.Newobj, Method(name == "Virtual" ? "DerivedReceiver" : "Receiver", ".ctor", 1))); il.Add(Instruction.Create(OpCodes.Stloc, local));
        il.Add(Instruction.Create(OpCodes.Ldloc, local)); il.Add(Instruction.CreateLdcI4(name == "Virtual" ? 1 : 2));
        if (name == "Virtual") { il.Add(Instruction.Create(OpCodes.Ldloc, local)); il.Add(Instruction.Create(OpCodes.Ldvirtftn, Method("Receiver", "V", 1))); }
        else il.Add(Instruction.Create(OpCodes.Ldftn, Method("Receiver", "Add", 1)));
        if (name == "ExplicitThis") { sig.ExplicitThis = true; sig.Params.Insert(0, receiver.ToTypeSig()); }
    }
    il.Add(Instruction.Create(OpCodes.Calli, sig)); il.Add(Instruction.Create(OpCodes.Ret));
}
var native = new MethodDefUser("NativeInventory", MethodSig.CreateStatic(module.CorLibTypes.Int32, module.CorLibTypes.IntPtr), MethodImplAttributes.IL | MethodImplAttributes.Managed, MethodAttributes.Public | MethodAttributes.Static) { Body = new CilBody { MaxStack = 2 } };
type.Methods.Add(native);
native.Body.Instructions.Add(Instruction.CreateLdcI4(41)); native.Body.Instructions.Add(Instruction.Create(OpCodes.Ldarg_0));
native.Body.Instructions.Add(Instruction.Create(OpCodes.Calli, new MethodSig(CallingConvention.C, 0, module.CorLibTypes.Int32, new[] { module.CorLibTypes.Int32 }))); native.Body.Instructions.Add(Instruction.Create(OpCodes.Ret));
module.Write(args[1]);

if (args.Length > 2 && args[2] == "verify")
{
    System.Runtime.Loader.AssemblyLoadContext.Default.LoadFromAssemblyPath(Path.GetFullPath(args[0]));
    var assembly = System.Runtime.Loader.AssemblyLoadContext.Default.LoadFromAssemblyPath(Path.GetFullPath(args[1]));
    foreach (var method in assembly.GetType("CalliILTests.Program").GetMethods(System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Static))
    {
        if (method.GetParameters().Length != 0) continue;
        if ((int)method.Invoke(null, null) != 42) throw new Exception(method.Name);
        Console.WriteLine("CLR comparison passed: " + method.Name);
    }
}
