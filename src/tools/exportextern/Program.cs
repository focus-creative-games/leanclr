using dnlib.DotNet;
using System.Text;

namespace Analysis
{
    enum ExternalMethodType
    {
        InternalCall,
        PInvoke,
        Intrinsic,
        All,
    }

    internal class Program
    {
        static ExternalMethodType GetMethodType(string type)
        {
            switch (type)
            {
            case "internalcall": return ExternalMethodType.InternalCall;
            case "pinvoke": return ExternalMethodType.PInvoke;
            case "intrinsic": return ExternalMethodType.Intrinsic;
            case "all": return ExternalMethodType.All;
            default: throw new Exception($"unknown method type:{type}");
            }
        }

        private static bool HasIntrinsicAttribute(MethodDef method)
        {
            if (!method.HasCustomAttributes)
                return false;
            foreach (var ca in method.CustomAttributes)
            {
                if (ca.TypeFullName == "System.Runtime.CompilerServices.IntrinsicAttribute")
                {
                    return true;
                }
            }
            return false;
        }

        static void Main(string[] args)
        {
            if (args.Length != 3)
            {
                Console.WriteLine("Usage: <exe> {dll} {internalcall|pinvoke|intrinsic|all} {output}");
                return;
            }
            string dll = args[0];
            ExternalMethodType methodType = GetMethodType(args[1]);
            string outputFile = args[2];

            ModuleDefMD module = ModuleDefMD.Load(dll);

            var icallsLines = new List<string>();
            var pinvokeLines = new List<string>();
            var intrinsicLines = new List<string>();
            foreach (TypeDef type in module.GetTypes())
            {
                foreach (MethodDef method in type.Methods)
                {
                    if (method.IsInternalCall)
                    {
                        if (methodType == ExternalMethodType.InternalCall || methodType == ExternalMethodType.All)
                            icallsLines.Add($"[InternalCall] {method.FullName}");
                    }
                    else if (method.IsPinvokeImpl)
                    {
                        if (methodType == ExternalMethodType.PInvoke || methodType == ExternalMethodType.All)
                            pinvokeLines.Add($"[PInvokeImpl ] {method.FullName}");
                    }
                    else if (HasIntrinsicAttribute(method))
                    {
                        if (methodType == ExternalMethodType.Intrinsic || methodType == ExternalMethodType.All)
                            intrinsicLines.Add($"[Intrinsic   ] {method.FullName}");
                    }
                }
            }
            var compareFunc = new Comparison<string>((a, b) => a.Substring(a.IndexOf(' ', 16)).CompareTo(b.Substring(b.IndexOf(' ', 16))));
            icallsLines.Sort(compareFunc);
            pinvokeLines.Sort(compareFunc);
            intrinsicLines.Sort(compareFunc);
            Directory.CreateDirectory(Path.GetDirectoryName(outputFile));
            var totalLines = icallsLines.Concat(pinvokeLines).Concat(intrinsicLines).ToList();
            System.IO.File.WriteAllLines(outputFile, totalLines, Encoding.UTF8);
            Console.WriteLine($"Found {totalLines.Count} methods, saved to {outputFile}");
        }
    }
}
