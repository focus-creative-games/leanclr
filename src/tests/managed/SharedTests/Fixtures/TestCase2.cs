using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public interface IReference { void Clear(); }
public abstract class base1 : IReference { public abstract void Clear(); }
public abstract class base2 : base1 { public abstract void Fun(); }
