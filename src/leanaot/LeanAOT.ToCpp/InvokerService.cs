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

        static string CreateRelaxedMethodFunctionTypeDeclaring(IMethod method)
        {
            MethodSig methodSig = method.MethodSig;
            StringBuilder sb = new StringBuilder();
            sb.Append(MethodGenerationUtil.GetResultTypeName(methodSig.RetType));
            sb.Append(" (*)(");
            var typeNameService = GlobalServices.Inst.TypeNameService;
            bool first = true;
            if (methodSig.HasThis)
            {
                sb.Append(typeNameService.GetCppTypeNameAsFieldOrArgOrLoc(MetaUtil.GetThisType(method), TypeNameRelaxLevel.AbiRelaxed));
                first = false;
            }
            sb.Append(')');
            foreach (var param in methodSig.Params)
            {
                if (first)
                    first = false;
                else
                {
                    sb.Append(", ");
                }
                sb.Append(typeNameService.GetCppTypeNameAsFieldOrArgOrLoc(param, TypeNameRelaxLevel.AbiRelaxed));
            }
            sb.Append(')');
            return sb.ToString();
        }

        private string GetInvokerName(IMethod method)
        {
            string hash = HashUtil.CreateMd5Hash(CreateRelaxedMethodFunctionTypeDeclaring(method));
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
