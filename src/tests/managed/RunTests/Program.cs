using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace RunTests
{
    internal class Program
    {
        static void Main(string[] args)
        {
            int total = 0;
            int passed = 0;
            int failed = 0;

            var asses = new Assembly[] { typeof(CoreTests.App).Assembly, typeof(CorlibTests.App).Assembly };

            foreach (var asm in asses)
            {
                var types = asm.GetTypes();
                for (int i = 0; i < types.Length; i++)
                {
                    var type = types[i];
                    var methods = type.GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static);
                    for (int j = 0; j < methods.Length; j++)
                    {
                        var method = methods[j];
                        if (!Attribute.IsDefined(method, typeof(UnitTestAttribute), inherit: true))
                        {
                            continue;
                        }

                        if (method.ReturnType != typeof(void) || method.GetParameters().Length != 0)
                        {
                            continue;
                        }

                        total++;
                        object instance = null;
                        if (!method.IsStatic)
                        {
                            try
                            {
                                instance = Activator.CreateInstance(type);
                            }
                            catch (Exception ex)
                            {
                                failed++;
                                Console.WriteLine($"[FAIL] {type.FullName}.{method.Name} - cannot create instance: {ex}");
                                continue;
                            }
                        }

                        try
                        {
                            method.Invoke(instance, null);
                            passed++;
                            //Console.WriteLine($"[PASS] {type.FullName}.{method.Name}");
                        }
                        catch (TargetInvocationException tie)
                        {
                            failed++;
                            var inner = tie.InnerException ?? tie;
                            Console.WriteLine($"[FAIL] {type.FullName}.{method.Name} - {inner}");
                        }
                        catch (Exception ex)
                        {
                            failed++;
                            Console.WriteLine($"[FAIL] {type.FullName}.{method.Name} - {ex}");
                        }
                    }
                }
            }

            if (failed == 0)
            {
                Console.WriteLine($"[SUMMARY] total={total}, passed={passed}, failed={failed}");
                Console.WriteLine("All tests passed!");
            }
            else
            {
                Console.Error.WriteLine($"UnitTest failed. total={total}, passed={passed}, failed={failed}");
            }
        }
    }
}
