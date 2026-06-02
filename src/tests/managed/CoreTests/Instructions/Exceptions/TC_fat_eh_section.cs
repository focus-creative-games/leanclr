using System;
using System.Reflection;
using test;

namespace Tests.Instruments.Exceptions
{
    /// <summary>
    /// Exercises fat-format EH sections whose Data Size exceeds 255 bytes (24-bit size in the header).
    /// </summary>
    internal class TC_fat_eh_section : GeneralTestCaseBase
    {
        private const int ClauseCount = 12;

        private sealed class EhEx00 : Exception
        {
        }
        private sealed class EhEx01 : Exception
        {
        }
        private sealed class EhEx02 : Exception
        {
        }
        private sealed class EhEx03 : Exception
        {
        }
        private sealed class EhEx04 : Exception
        {
        }
        private sealed class EhEx05 : Exception
        {
        }
        private sealed class EhEx06 : Exception
        {
        }
        private sealed class EhEx07 : Exception
        {
        }
        private sealed class EhEx08 : Exception
        {
        }
        private sealed class EhEx09 : Exception
        {
        }
        private sealed class EhEx10 : Exception
        {
        }
        private sealed class EhEx11 : Exception
        {
        }

        public static int MethodWithManyCatchClauses(int selector)
        {
            try
            {
                // Large try region so the compiler emits a fat EH table (section size > 255).
                int acc = 0;
                for (int i = 0; i < 300; i++)
                {
                    acc += i;
                    if ((acc & 3) == 0)
                    {
                        acc ^= i;
                    }
                }
                throw ExceptionForSelector(selector);
            }
            catch (EhEx00)
            {
                return 0;
            }
            catch (EhEx01)
            {
                return 1;
            }
            catch (EhEx02)
            {
                return 2;
            }
            catch (EhEx03)
            {
                return 3;
            }
            catch (EhEx04)
            {
                return 4;
            }
            catch (EhEx05)
            {
                return 5;
            }
            catch (EhEx06)
            {
                return 6;
            }
            catch (EhEx07)
            {
                return 7;
            }
            catch (EhEx08)
            {
                return 8;
            }
            catch (EhEx09)
            {
                return 9;
            }
            catch (EhEx10)
            {
                return 10;
            }
            catch (EhEx11)
            {
                return 11;
            }
        }

        private static Exception ExceptionForSelector(int selector)
        {
            switch (selector)
            {
                case 0:
                    return new EhEx00();
                case 1:
                    return new EhEx01();
                case 2:
                    return new EhEx02();
                case 3:
                    return new EhEx03();
                case 4:
                    return new EhEx04();
                case 5:
                    return new EhEx05();
                case 6:
                    return new EhEx06();
                case 7:
                    return new EhEx07();
                case 8:
                    return new EhEx08();
                case 9:
                    return new EhEx09();
                case 10:
                    return new EhEx10();
                case 11:
                    return new EhEx11();
                default:
                    return new Exception("unexpected selector");
            }
        }

        [UnitTest]
        public void run_method_with_fat_eh_section()
        {
            for (int i = 0; i < ClauseCount; i++)
            {
                Assert.Equal(i, MethodWithManyCatchClauses(i));
            }
        }

        [UnitTest]
        public void reflection_reads_all_exception_clauses()
        {
            MethodInfo method = typeof(TC_fat_eh_section).GetMethod(
                nameof(MethodWithManyCatchClauses),
                BindingFlags.Public | BindingFlags.Static);
            Assert.NotNull(method);

            MethodBody body = method.GetMethodBody();
            Assert.NotNull(body);
            Assert.Equal(ClauseCount, body.ExceptionHandlingClauses.Count);
        }
    }
}
