using dnlib.DotNet;
using LeanAOT.Core;

namespace LeanAOT.ToCpp
{

    public class DirectCallBridgeInfo
    {
        public string name;
        public MethodDetail method;
        public bool delegateInvoke;

        public string GenerateMethodDeclaring()
        {
            return $"{MethodGenerationUtil.GetResultTypeName(method.RetType)} {name}({MethodGenerationUtil.CreateMethodRelaxedArgs(method, true)}{(method.ParamCountIncludeThis > 0 ? " ," : "")}{ConstStrings.MethodInfoPtrTypeName} __method, bool __callvir) {ConstStrings.CppFunctionNoexcept}";
        }
    }

    public class DirectCallBridgeService
    {
        private readonly MetadataService _metadataService;
        private readonly Dictionary<string, DirectCallBridgeInfo> _directCallBridgeInfos;

        public DirectCallBridgeService(MetadataService metadataService)
        {
            _metadataService = metadataService;
            _directCallBridgeInfos = new Dictionary<string, DirectCallBridgeInfo>();
        }


        public DirectCallBridgeInfo GetDirectCallBridgeInfo(MethodDetail methodDetail)
        {
            bool delegateInvoke = !methodDetail.IsStatic && methodDetail.MethodDef?.Name == "Invoke" &&
                MetaUtil.IsDerivedFromMulticastDelegate(methodDetail.MethodDef.DeclaringType);
            string name = GetDirectCallBridgeName(methodDetail) + (delegateInvoke ? "_delegate" : "");
            if (_directCallBridgeInfos.TryGetValue(name, out var info))
            {
                return info;
            }
            var newInfo = new DirectCallBridgeInfo
            {
                name = name,
                method = methodDetail,
                delegateInvoke = delegateInvoke,
            };
            _directCallBridgeInfos[name] = newInfo;
            return newInfo;
        }

        private string GetDirectCallBridgeName(MethodDetail methodDetail)
        {
            // An enum can share the relaxed int32 signature but never uses the
            // packed primitive ABI. Preserve that distinction in legacy callers.
            string hash = HashUtil.CreateMd5Hash(MethodGenerationUtil.CreateRelaxedMethodFunctionTypeDeclaring(methodDetail) +
                (ReturnProtocol.IsInt32(methodDetail.RetType) ? "|packed32-capable" : "") + (ReturnProtocol.CanStatusOut(methodDetail.RetType) ? "|status-out-capable" : ""));
            return $"leanclr_generated_direct_call_bridge_{hash}";
        }

        public List<DirectCallBridgeInfo> GetDirectCallBridgeInfos()
        {
            return _directCallBridgeInfos.Values.ToList();
        }
    }
}
