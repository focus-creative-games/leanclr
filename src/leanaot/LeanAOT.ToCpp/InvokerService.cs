using dnlib.DotNet;
using LeanAOT.Core;
using System.Text;

namespace LeanAOT.ToCpp
{
    public class MethodInvokerInfo
    {
        public string name;
        public IMethod method;
    }

    public class InvokerService
    {
        private readonly MetadataService _metadataService;
        private readonly Dictionary<string, MethodInvokerInfo> _methodInvokerInfos;
        private readonly Dictionary<string, MethodInvokerInfo> _virtualMethodInvokerInfos;

        public InvokerService(MetadataService metadataService)
        {
            _metadataService = metadataService;
            _methodInvokerInfos = new Dictionary<string, MethodInvokerInfo>();
            _virtualMethodInvokerInfos = new Dictionary<string, MethodInvokerInfo>();
        }

        public List<MethodInvokerInfo> GetNotVirtualInvokers()
        {
            return _methodInvokerInfos.Values.ToList();
        }

        public List<MethodInvokerInfo> GetVirtualInvokers()
        {
            return _virtualMethodInvokerInfos.Values.ToList();
        }

        private string GetInvokerName(IMethod method)
        {
            MethodDetail methodDetail = _metadataService.GetMethodDetail(method);
            string hash = HashUtil.CreateMd5Hash(MethodGenerationUtil.CreateRelaxedMethodFunctionTypeDeclaring(methodDetail));
            return $"leanclr_generated_invoke_method_{hash}";
        }

        public MethodInvokerInfo GetInvoker(IMethod method)
        {
            string invokerName = GetInvokerName(method);
            if (_methodInvokerInfos.TryGetValue(invokerName, out var info))
            {
                return info;
            }
            var newInfo = new MethodInvokerInfo
            {
                name = invokerName,
                method = method,
            };
            _methodInvokerInfos[invokerName] = newInfo;
            return newInfo;
        }
    }
}
