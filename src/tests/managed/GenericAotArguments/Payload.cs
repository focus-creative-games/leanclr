namespace GenericAotArguments
{
    public sealed class ReferenceValue { public int Marker; }
    public struct ReferenceStruct { public long Marker; public object Reference; }
}
namespace GenericCollision
{
    // Same full type name as Common.GenericCollision.Payload; distinct assembly,
    // layout and TypeDef identity. These must never share a specialization.
    public struct Payload { public long Marker; public object Reference; }
}
