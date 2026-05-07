namespace LeanAOT.GenerationPlan
{
    /// <summary>
    /// Thrown when two or more AOT rule files assign conflicting <c>aot</c> values (1 vs 0) to the same method.
    /// </summary>
    public sealed class AotRuleConflictException : Exception
    {
        public AotRuleConflictException(string message)
            : base(message)
        {
        }
    }
}
