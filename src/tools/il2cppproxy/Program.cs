using System.Diagnostics;
using System.Runtime.InteropServices;

namespace il2cppproxy
{
    internal class Program
    {

        private const string s_logFile = "il2cpp.log";


        static string[] ParseRspArguments(string rspFile)
        {
            string rspFileContent = File.ReadAllText(rspFile, System.Text.Encoding.UTF8).Trim();
            var rspArgs = new List<string>();

            while(true)
            {
                int argStart = rspFileContent.IndexOf("--", 2);
                if (argStart > 0)
                {
                    string arg = rspFileContent.Substring(0, argStart).Trim();
                    rspArgs.Add(arg);
                    rspFileContent = rspFileContent.Substring(argStart);
                }
                else
                {
                    rspArgs.Add(rspFileContent);
                    break;
                }
            }
            return rspArgs.ToArray();
        }

        static void Main(string[] args)
        {
            string cmdline = string.Join(" ", args);

            AppendLog(cmdline);

            Console.WriteLine(cmdline);
            string baseDir = AppDomain.CurrentDomain.BaseDirectory;

            if (args.Length  == 1 && args[0].EndsWith(".rsp"))
            {
                // rsp file format : @xxxxx.rsp
                string rspFileName = args[0].Substring(1);
                string rspFileContent = File.ReadAllText(rspFileName, System.Text.Encoding.UTF8);
                string[] rspArgs = ParseRspArguments(rspFileName);
                string rspCmdLine = string.Join(" ", rspArgs);
                AppendLog($"run rsp {rspCmdLine}");
                if (rspArgs.Length == 0 || rspArgs[0] != "--convert-to-cpp")
                {
                    int exitCode = RunProcessWithOutputRedirection(GetExeNameByPlatform($"{baseDir}/il2cpp-origin"), args);
                    Environment.Exit(exitCode);
                }
                else
                {
                    RunConvertToCppRsp(baseDir, args, rspFileName, rspArgs);
                }
            }
            else
            {
                if (args.Length == 0 || args[0] != "--convert-to-cpp")
                {
                    int exitCode = RunProcessWithOutputRedirection(GetExeNameByPlatform($"{baseDir}/il2cpp-origin"), args);
                    Environment.Exit(exitCode);
                }

                RunConvertToCpp(baseDir, args);
            }
        }

        private static void AppendLog(string log)
        {
            File.AppendAllText(s_logFile, log + "\n");
        }

        private static string GetExeNameByPlatform(string exeName)
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                return $"{exeName}.exe";
            }
            else
            {
                return exeName;
            }
        }

        private static void RunConvertToCpp(string baseDir, string[] args)
        {
            int ret = RunProcessWithOutputRedirection(Path.Combine(baseDir, "leanaot", GetExeNameByPlatform("LeanAOT")), args);
            if (ret != 0)
            {
                Environment.Exit(ret);
            }
        }

        private static void RunConvertToCppRsp(string baseDir, string[] args0, string rspFile, string[] rspArgs)
        {
            int ret = RunProcessWithOutputRedirection(Path.Combine(baseDir, "leanaot", GetExeNameByPlatform("LeanAOT")), args0);
            if (ret != 0)
            {
                Environment.Exit(ret);
            }
        }

        public static int RunProcessWithOutputRedirection(string fileName, string[] arguments)
        {
            string argStr = string.Join(" ", arguments.Select(s => $"\"{s.Replace('\\', '/').TrimEnd('/')}\""));
            string cmdline = $"{fileName} {argStr}";
            AppendLog(cmdline);

            var startInfo = new ProcessStartInfo
            {
                FileName = fileName,
                Arguments = argStr,
                UseShellExecute = false,          // 必须禁用 Shell 执行才能重定向
                RedirectStandardOutput = true,    // 重定向标准输出
                RedirectStandardError = true,     // 重定向错误输出
                CreateNoWindow = true,             // 不创建新窗口
            };

            using (var process = new Process { StartInfo = startInfo })
            {
                // 绑定输出接收事件
                process.OutputDataReceived += (sender, e) =>
                {
                    if (!string.IsNullOrEmpty(e.Data))
                        Console.WriteLine(e.Data);
                };

                process.ErrorDataReceived += (sender, e) =>
                {
                    if (!string.IsNullOrEmpty(e.Data))
                        Console.Error.WriteLine(e.Data);
                };

                process.Start();

                // 开始异步读取输出
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();

                // 等待进程退出（同时保持输出响应）
                process.WaitForExit();

                // 确保所有输出处理完成
                Thread.Sleep(100); // 可选，防止极少数情况下的输出截断

                return process.ExitCode;
            }
        }
    }
}
