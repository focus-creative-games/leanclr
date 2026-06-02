using System.Threading.Tasks;

namespace Tests.Fixtures
{
    public interface IStateOwner
    {
        Task TranslateState<TState>(object[] args);
    }

    public abstract class State<TOwner> where TOwner : IStateOwner
    {
        public abstract Task OnEnter();
    }
}
