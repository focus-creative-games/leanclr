# LeanCLR 测试工程

当前测试入口以 `scripts/dotnet10` 和 `coreclr-net10` profile 为准。旧 Mono `scripts/test`、`basic-tester` 和 `managed.sln` 已随 Mono profile 清理删除；`src/tests/managed` 只作为 `ManagedNet10.LegacyTests` 的源码素材保留。

## 快速验证

| 目标 | 命令 |
| --- | --- |
| 默认解释 smoke | `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\interp-smoke.ps1 -Configuration Release` |
| Legacy net10 回归 | `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\interp-smoke.ps1 -Configuration Release -AssemblyName ManagedNet10.LegacyTests -Entry "ManagedNet10.LegacyTests.Program::RunAll"` |
| 旧 managed 素材覆盖审计 | `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\legacy-managed-audit.ps1` |
| 解释 smoke 矩阵 | `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\interp-smoke-matrix.ps1 -Configuration Release` |
| API 白名单扫描 | `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\api-scan.ps1 -Configuration Release` |
| NKG/Odin workload | `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\nkg-smoke.ps1 -Configuration Release` |
| Core runtime assemblies AOT 编译 workload | `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\corelib-aot-smoke.ps1 -Configuration Release` |
| Legacy net10 AOT 单入口 | `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\legacy-aot-smoke.ps1 -Configuration Release -Entry "ManagedNet10.LegacyTests.Program::RunLegacyDiscoverySmoke"` |
| Legacy net10 AOT 逐入口矩阵 | `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\legacy-aot-smoke-matrix.ps1 -Configuration Release -Entries RunLegacyDiscoverySmoke` |
| NKG/Odin 全量 AOT workload | `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\nkg-aot-smoke.ps1 -Configuration Release` |
| LeanAOT net10 native smoke | `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\aot-smoke.ps1 -Configuration Release -NativeRun` |
| Runtime API 签名 gate | `python src\generator\check_runtime_api_signatures.py --profile coreclr-net10 --repo-root .` |

## Host Bridge

Host bridge 使用 `scripts\dotnet10\host-bridge-smoke.ps1 -Configuration Release -Scenario <scenario>` 验收。

当前场景：

- `AbiSkeleton`
- `HandleRegistry`
- `Dispatcher`
- `EventCallback`
- `EngineAdapter`
- `ValueMarshal`
- `Diagnostics`

完整本地 sweep：

```powershell
$scenarios = @('AbiSkeleton','HandleRegistry','Dispatcher','EventCallback','EngineAdapter','ValueMarshal','Diagnostics')
foreach ($s in $scenarios) {
  powershell -ExecutionPolicy Bypass -File scripts\dotnet10\host-bridge-smoke.ps1 -Configuration Release -Scenario $s
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
```

## 口径

上述 gate 通过表示 LeanCLR 自有测试资产、当前 `.NET 10` runtime contract、API 白名单、真实轻量 workload、`.NET 10` core runtime assemblies AOT、LegacyTests AOT、NKG/Odin 全量 AOT workload 和 mock host bridge 在当前 profile 下保持绿色。

AOT 生成产物收束在 `artifacts\net10-aot\core\generated`、`artifacts\net10-aot\smoke\generated`、`artifacts\net10-aot\legacy\generated` 和 `artifacts\net10-aot\nkg\generated`。脚本每次运行都会清理自己的 `generated` 目录，旧的 `artifacts\net10-aot-*` 本地目录不属于当前门禁。`aot-smoke.ps1`、`legacy-aot-smoke.ps1` 和 `nkg-aot-smoke.ps1` 默认先跑 core runtime assemblies AOT 编译门禁，再跑各自的 native run。

这不表示完整 `Microsoft.NETCore.App`、真实 Unity/Godot SDK binding、完整 debugger、动态 native codegen、Socket 网络栈或多线程 runtime 已完成。

避免并行运行会竞争同一 CMake build tree 的 smoke 命令；需要完整 sweep 时按顺序执行。
