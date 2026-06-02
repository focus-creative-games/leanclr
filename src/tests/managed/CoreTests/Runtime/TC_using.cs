using test;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tests.CSharp
{
    class DisposeA : IDisposable
    {
        public bool disposed;

        public void Dispose()
        {
            disposed = true;
        }
    }

    internal class TC_using : GeneralTestCaseBase
    {
        [UnitTest]
        public void disposed()
        {
            var a = new DisposeA();
            int b = 0;
            using (a)
            {
                b = 1;
            }
            Assert.True(a.disposed);
            Assert.Equal(1, b);
        }
    }
}
