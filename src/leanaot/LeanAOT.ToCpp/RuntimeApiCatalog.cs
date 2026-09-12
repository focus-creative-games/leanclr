using dnlib.DotNet;
using LeanAOT.Core;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Text.Json;

namespace LeanAOT.ToCpp
{
    public sealed class RuntimeApiEntry
    {
        public string Name { get; set; }

        public string Func { get; set; }

        public string Header { get; set; }

        public MethodKind MethodKind { get; set; }
    }

    internal sealed class PInvokesConfigFile
    {
        public Dictionary<string, List<string>> StaticLinkedMethods { get; set; }

        public List<string> RuntimeApiNames { get; set; }
    }

    internal sealed class RuntimeApiProfileConfig
    {
        public string Name { get; set; }

        public List<string> CoreLibraryModules { get; set; }
    }

    public sealed class RuntimeApiCatalog
    {
        private readonly IReadOnlyDictionary<string, RuntimeApiEntry> _icalls;
        private readonly IReadOnlyDictionary<string, RuntimeApiEntry> _intrinsics;
        private readonly IReadOnlyDictionary<string, RuntimeApiEntry> _icallsNewobj;
        private readonly IReadOnlyDictionary<string, RuntimeApiEntry> _intrinsicsNewobj;
        private readonly IReadOnlyDictionary<string, HashSet<string>> _staticLinkedMethods;
        private readonly HashSet<string> _coreLibModules;

        private RuntimeApiCatalog(
            string profileName,
            string sourceDirectory,
            HashSet<string> coreLibModules,
            IReadOnlyDictionary<string, RuntimeApiEntry> icalls,
            IReadOnlyDictionary<string, RuntimeApiEntry> intrinsics,
            IReadOnlyDictionary<string, RuntimeApiEntry> icallsNewobj,
            IReadOnlyDictionary<string, RuntimeApiEntry> intrinsicsNewobj,
            IReadOnlyDictionary<string, HashSet<string>> staticLinkedMethods)
        {
            ProfileName = profileName;
            SourceDirectory = sourceDirectory;
            _coreLibModules = coreLibModules;
            _icalls = icalls;
            _intrinsics = intrinsics;
            _icallsNewobj = icallsNewobj;
            _intrinsicsNewobj = intrinsicsNewobj;
            _staticLinkedMethods = staticLinkedMethods;
        }

        public string ProfileName { get; }
        public string SourceDirectory { get; }
        public int IcallCount => _icalls.Count;
        public int IntrinsicCount => _intrinsics.Count;
        public int IcallNewobjCount => _icallsNewobj.Count;
        public int IntrinsicNewobjCount => _intrinsicsNewobj.Count;
        public int StaticLinkedPInvokeDllCount => _staticLinkedMethods.Count;
        public int StaticLinkedPInvokeMethodCount => _staticLinkedMethods.Values.Sum(m => m.Count);
        public int CoreLibraryModuleCount => _coreLibModules.Count;

        public bool TryGetIcall(string name, out RuntimeApiEntry entry) => _icalls.TryGetValue(name, out entry);
        public bool TryGetIntrinsic(string name, out RuntimeApiEntry entry) => _intrinsics.TryGetValue(name, out entry);
        public bool TryGetIcallNewobj(string name, out RuntimeApiEntry entry) => _icallsNewobj.TryGetValue(name, out entry);
        public bool TryGetIntrinsicNewobj(string name, out RuntimeApiEntry entry) => _intrinsicsNewobj.TryGetValue(name, out entry);

        /// <summary>
        /// Returns true when the P/Invoke target is statically linked (from <c>pinvokes.json</c> or internal DLL).
        /// </summary>
        public bool IsStaticLinked(string dllName, string methodName)
        {
            if (string.IsNullOrEmpty(dllName) || string.IsNullOrEmpty(methodName))
            {
                return false;
            }

            if (dllName == ConstStrings.InternalDllName)
            {
                return true;
            }

            return _staticLinkedMethods.TryGetValue(dllName, out var methods) && methods.Contains(methodName);
        }

        public bool IsCoreLibraryModule(ModuleDef module) => MetaUtil.IsCoreLibraryModule(module, _coreLibModules);

        private string GetFullMethodName(MethodDef methodDef)
        {
            return NameUtil.GetICallFullMethodName(methodDef);
        }

        private string GetBriefMethodName(MethodDef methodDef)
        {
            return $"{methodDef.DeclaringType.FullName}::{methodDef.Name}";
        }

        public bool TryGetIcallOrIntrinsic(MethodDef methodDef, out RuntimeApiEntry entry, out MethodKind methodKind)
        {
            entry = null;
            methodKind = MethodKind.Normal;
            if (methodDef == null || methodDef.HasGenericParameters || methodDef.DeclaringType.HasGenericParameters)
            {
                return false;
            }
            if (!IsCoreLibraryModule(methodDef.Module))
            {
                return false;
            }
            string fullMethodName = GetFullMethodName(methodDef);
            if (_icalls.TryGetValue(fullMethodName, out entry))
            {
                methodKind = MethodKind.ICall;
                return true;
            }
            else if (_intrinsics.TryGetValue(fullMethodName, out entry))
            {
                methodKind = MethodKind.Intrinsic;
                return true;
            }
            string briefMethodName = GetBriefMethodName(methodDef);
            if (_icalls.TryGetValue(briefMethodName, out entry))
            {
                methodKind = MethodKind.ICall;
                return true;
            }
            else if (_intrinsics.TryGetValue(briefMethodName, out entry))
            {
                methodKind = MethodKind.Intrinsic;
                return true;
            }
            return false;
        }

        public bool TryGetIcallOrIntrinsicNewobj(MethodDef methodDef, out RuntimeApiEntry entry, out MethodKind methodKind)
        {
            Debug.Assert(methodDef.IsConstructor, "methodDef must be a constructor");
            entry = null;
            methodKind = MethodKind.Normal;
            if (!IsCoreLibraryModule(methodDef.Module))
            {
                return false;
            }
            string fullMethodName = GetFullMethodName(methodDef);
            if (_icallsNewobj.TryGetValue(fullMethodName, out entry))
            {
                methodKind = MethodKind.ICallNewObj;
                return true;
            }
            else if (_intrinsicsNewobj.TryGetValue(fullMethodName, out entry))
            {
                methodKind = MethodKind.IntrinsicNewObj;
                return true;
            }
            string briefMethodName = GetBriefMethodName(methodDef);
            if (_icallsNewobj.TryGetValue(briefMethodName, out entry))
            {
                methodKind = MethodKind.ICallNewObj;
                return true;
            }
            else if (_intrinsicsNewobj.TryGetValue(briefMethodName, out entry))
            {
                methodKind = MethodKind.IntrinsicNewObj;
                return true;
            }
            return false;
        }

        public static RuntimeApiCatalog LoadFromDirectory(string baseDirectory, string profileName = null)
        {
            if (string.IsNullOrWhiteSpace(baseDirectory))
            {
                throw new ArgumentException("Base directory cannot be null or empty.", nameof(baseDirectory));
            }

            var catalogDirectory = ResolveCatalogDirectory(baseDirectory, profileName);
            var effectiveProfileName = GetEffectiveProfileName(profileName, catalogDirectory);

            var options = new JsonSerializerOptions
            {
                PropertyNameCaseInsensitive = true
            };

            var coreLibraryModules = LoadCoreLibraryModules(catalogDirectory, effectiveProfileName, options);
            var icalls = LoadEntries(Path.Combine(catalogDirectory, "icalls.json"), options, MethodKind.ICall);
            var intrinsics = LoadEntries(Path.Combine(catalogDirectory, "intrinsics.json"), options, MethodKind.Intrinsic);
            var icallsNewobj = LoadEntries(Path.Combine(catalogDirectory, "icalls_newobj.json"), options, MethodKind.ICallNewObj);
            var intrinsicsNewobj = LoadEntries(Path.Combine(catalogDirectory, "intrinsics_newobj.json"), options, MethodKind.IntrinsicNewObj);
            var staticLinkedMethods = LoadStaticLinkedMethods(Path.Combine(catalogDirectory, "pinvokes.json"), options);

            return new RuntimeApiCatalog(
                effectiveProfileName,
                catalogDirectory,
                coreLibraryModules,
                icalls,
                intrinsics,
                icallsNewobj,
                intrinsicsNewobj,
                staticLinkedMethods);
        }

        private static string ResolveCatalogDirectory(string baseDirectory, string profileName)
        {
            var normalizedBaseDirectory = Path.GetFullPath(baseDirectory);
            var runtimeApisRoot = Path.Combine(normalizedBaseDirectory, "runtime-apis");

            if (string.IsNullOrWhiteSpace(profileName))
            {
                var coreClrProfileDirectory = Path.Combine(runtimeApisRoot, "coreclr-net10");
                if (!Directory.Exists(coreClrProfileDirectory))
                {
                    throw new DirectoryNotFoundException($"Default runtime API profile 'coreclr-net10' was not found under: {runtimeApisRoot}");
                }

                return coreClrProfileDirectory;
            }

            var profileDirectory = Path.Combine(runtimeApisRoot, profileName.Trim());
            if (!Directory.Exists(profileDirectory))
            {
                throw new DirectoryNotFoundException($"Runtime API profile '{profileName}' was not found under: {runtimeApisRoot}");
            }

            return profileDirectory;
        }

        private static string GetEffectiveProfileName(string profileName, string catalogDirectory)
        {
            if (!string.IsNullOrWhiteSpace(profileName))
            {
                return profileName.Trim();
            }

            return Path.GetFileName(catalogDirectory);
        }

        private static HashSet<string> LoadCoreLibraryModules(string catalogDirectory, string profileName, JsonSerializerOptions options)
        {
            var profileConfigPath = Path.Combine(catalogDirectory, "profile.json");
            if (File.Exists(profileConfigPath))
            {
                var json = File.ReadAllText(profileConfigPath);
                var config = JsonSerializer.Deserialize<RuntimeApiProfileConfig>(json, options);
                if (config?.CoreLibraryModules == null || config.CoreLibraryModules.Count == 0)
                {
                    throw new InvalidDataException($"Runtime API profile file must define at least one core library module: {profileConfigPath}");
                }

                return CreateCoreLibraryModuleSet(config.CoreLibraryModules, profileConfigPath);
            }

            return CreateCoreLibraryModuleSet(GetDefaultCoreLibraryModules(profileName), catalogDirectory);
        }

        private static IEnumerable<string> GetDefaultCoreLibraryModules(string profileName)
        {
            return new[]
            {
                "System.Private.CoreLib",
                "System.Runtime",
                "System.Console",
                "System.Collections",
                "System.Linq",
                "System.Threading",
                "System.Runtime.InteropServices",
                "System.Reflection",
                "netstandard",
                "LeanCLR",
            };
        }

        private static HashSet<string> CreateCoreLibraryModuleSet(IEnumerable<string> moduleNames, string source)
        {
            var set = new HashSet<string>(StringComparer.Ordinal);
            foreach (var moduleName in moduleNames)
            {
                if (string.IsNullOrWhiteSpace(moduleName))
                {
                    throw new InvalidDataException($"Runtime API profile contains an invalid core library module name: {source}");
                }

                if (!set.Add(moduleName.Trim()))
                {
                    throw new InvalidDataException($"Runtime API profile contains duplicate core library module '{moduleName}': {source}");
                }
            }

            return set;
        }

        private static IReadOnlyDictionary<string, HashSet<string>> LoadStaticLinkedMethods(string path, JsonSerializerOptions options)
        {
            if (!File.Exists(path))
            {
                return new Dictionary<string, HashSet<string>>(StringComparer.OrdinalIgnoreCase);
            }

            var json = File.ReadAllText(path);
            var config = JsonSerializer.Deserialize<PInvokesConfigFile>(json, options);
            if (config?.StaticLinkedMethods == null)
            {
                return new Dictionary<string, HashSet<string>>(StringComparer.OrdinalIgnoreCase);
            }

            var map = new Dictionary<string, HashSet<string>>(StringComparer.OrdinalIgnoreCase);
            foreach (var (dllName, methodNames) in config.StaticLinkedMethods)
            {
                if (string.IsNullOrWhiteSpace(dllName))
                {
                    throw new InvalidDataException($"Invalid static-linked P/Invoke DLL name in file: {path}");
                }

                if (methodNames == null || methodNames.Count == 0)
                {
                    throw new InvalidDataException($"Static-linked P/Invoke methods for DLL '{dllName}' cannot be empty in file: {path}");
                }

                var methods = new HashSet<string>(StringComparer.Ordinal);
                foreach (var methodName in methodNames)
                {
                    if (string.IsNullOrWhiteSpace(methodName))
                    {
                        throw new InvalidDataException($"Invalid static-linked P/Invoke method name for DLL '{dllName}' in file: {path}");
                    }

                    if (!methods.Add(methodName))
                    {
                        throw new InvalidDataException($"Duplicate static-linked P/Invoke method '{methodName}' for DLL '{dllName}' in file: {path}");
                    }
                }

                if (!map.TryAdd(dllName, methods))
                {
                    throw new InvalidDataException($"Duplicate static-linked P/Invoke DLL '{dllName}' in file: {path}");
                }
            }

            return map;
        }

        private static IReadOnlyDictionary<string, RuntimeApiEntry> LoadEntries(string path, JsonSerializerOptions options, MethodKind methodKind)
        {
            if (!File.Exists(path))
            {
                throw new FileNotFoundException($"Runtime API config file not found: {path}", path);
            }

            var json = File.ReadAllText(path);
            var entries = JsonSerializer.Deserialize<List<RuntimeApiEntry>>(json, options);
            if (entries == null)
            {
                throw new InvalidDataException($"Failed to deserialize runtime API config file: {path}");
            }

            var map = new Dictionary<string, RuntimeApiEntry>(StringComparer.Ordinal);
            for (int i = 0; i < entries.Count; i++)
            {
                var entry = entries[i];
                if (entry == null || string.IsNullOrWhiteSpace(entry.Name) || string.IsNullOrWhiteSpace(entry.Func) || string.IsNullOrWhiteSpace(entry.Header))
                {
                    throw new InvalidDataException($"Invalid runtime API entry at index {i} in file: {path}");
                }
                entry.MethodKind = methodKind;

                if (!map.TryAdd(entry.Name, entry))
                {
                    throw new InvalidDataException($"Duplicate runtime API entry name '{entry.Name}' in file: {path}");
                }
            }

            return map;
        }
    }
}
