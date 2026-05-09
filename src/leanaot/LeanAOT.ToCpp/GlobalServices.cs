namespace LeanAOT.ToCpp
{

    public class GlobalServices
    {
        public static GlobalServices Inst { get; set; }

        public GlobalConfig Config { get; set; }

        public TypeNameService TypeNameService { get; set; }

        public ManifestService ManifestService { get; set; }

        public InvokerService InvokerService { get; set; }

        public DirectCallBridgeService DirectCallBridgeService { get; set; }

        public MetadataService MetadataService { get; set; }

        public RuntimeApiCatalog RuntimeApiCatalog { get; set; }
    }
}
