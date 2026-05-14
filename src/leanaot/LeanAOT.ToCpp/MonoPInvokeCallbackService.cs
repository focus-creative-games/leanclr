using dnlib.DotNet;
using System;

namespace LeanAOT.ToCpp
{
    public class MonoPInvokeCallbackInfo
    {
        public readonly string name;
        public readonly MethodDef method;
        public readonly MethodDetail methodDetail;

        public MonoPInvokeCallbackInfo(string name, MethodDef method, MethodDetail methodDetail)
        {
            this.name = name;
            this.method = method;
            this.methodDetail = methodDetail;
        }
    }

    public class MonoPInvokeCallbackService
    {
        private readonly MetadataService _metadataService;
        private readonly Dictionary<MethodDef, MonoPInvokeCallbackInfo> _monoPInvokeCallbackInfos;

        public MonoPInvokeCallbackService(MetadataService metadataService)
        {
            _metadataService = metadataService;
            _monoPInvokeCallbackInfos = new Dictionary<MethodDef, MonoPInvokeCallbackInfo>(MethodEqualityComparer.CompareDeclaringTypes);
        }


        public MonoPInvokeCallbackInfo GetMonoPInvokeCallbackInfo(MethodDef method)
        {
            if (_monoPInvokeCallbackInfos.TryGetValue(method, out var info))
            {
                return info;
            }
            MethodDetail methodDetail = _metadataService.GetMethodDetail(method);
            var newInfo = new MonoPInvokeCallbackInfo(
                GetMonoPInvokeCallbackName(methodDetail),
                method,
                methodDetail
            );
            _monoPInvokeCallbackInfos[method] = newInfo;
            return newInfo;
        }

        private string GetMonoPInvokeCallbackName(MethodDetail methodDetail)
        {
            return $"{methodDetail.UniqueName}__monopinvokecallback";
        }
    }
}