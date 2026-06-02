using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public interface IStateOwner
{
    Task TranslateState<TState>(object[] args);
}
public abstract class State<TOwner> where TOwner : IStateOwner
{
    public abstract Task OnEnter();

}
