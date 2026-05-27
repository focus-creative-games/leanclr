using System;
using System.Diagnostics;
using System.Reflection;

namespace AotTests
{

    public class App
    {
        public static Action<string> s_logger = msg => Debugger.Log(0, "", msg + "\n");

        [AotMethod(false)]
        public static void Main()
        {
            int totalSuccess = 0;
            int totalFailed = 0;

            foreach (var type in typeof(App).Assembly.GetTypes())
            {
                foreach (var method in type.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static))
                {
                    if (method.GetCustomAttribute<UnitTestAttribute>() == null)
                        continue;
                    if (method.GetParameters().Length > 0)
                    {
                        s_logger($"[Skipped] {method.DeclaringType.Name}.{method.Name} => Method has parameters, skipping.\n");
                        continue;
                    }
                    try
                    {
                        var thisObj = method.IsStatic ? null : Activator.CreateInstance(type);
                        var result = method.Invoke(thisObj, null);
                        totalSuccess++;
                    }
                    catch (Exception ex)
                    {
                        string exMessage = ex.InnerException != null ? ex.InnerException.Message : ex.Message;
                        s_logger($"[Failed] {method.DeclaringType.Name}.{method.Name} => {exMessage} {(ex.InnerException != null ? ex.InnerException : ex)}\n");
                        totalFailed++;
                    }
                }
            }
            s_logger($"Total Tests: {totalSuccess + totalFailed}\n");
            s_logger($"Total Success: {totalSuccess}, Total Failed: {totalFailed}\n");
        }
    }
}