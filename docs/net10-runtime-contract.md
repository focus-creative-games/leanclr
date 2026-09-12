# LeanCLR minimal net10 Runtime Contract

本文记录当前 `.NET 10` profile 对 LeanCLR runtime 的 contract 要求。历史执行表和逐项迁移日志保存在 [`archive/net10-runtime-contract-history.md`](archive/net10-runtime-contract-history.md)。

## 目标

`minimal net10 profile` 的目标是运行受控的纯逻辑 `net10.0` DLL。LeanCLR 对外满足 `.NET 10` BCL 需要的 runtime façade，对内仍映射到自己的 VM：

```text
.NET 10 BCL / System.Private.CoreLib
  -> coreclr-net10 runtime API catalog
  -> LeanCLR net10 façade
  -> RtClass / RtMethodInfo / RtFieldInfo / metadata cache / interpreter / GC / host bridge
```

## Contract 原则

- Profile 隔离：`.NET 10` 路径不得隐式回落到 Mono-era icall、P/Invoke 或宽松签名。
- 精确签名：native 边界按完整类型名、方法名和签名匹配。
- façade 优先：外部保持 `.NET 10` BCL 期待的 handle / MethodTable / QCall 形状，内部再解码到 LeanCLR 模型。
- 可诊断失败：白名单外 API、unsupported handle shape、越界平台能力必须给出明确诊断或 `NotSupported`。
- 证据闭环：每个新增 contract 都要有来源、内部映射、catalog/whitelist 记录和至少一个验证入口。

## Profile 组成

| 部分 | 说明 |
| --- | --- |
| Core library | `System.Private.CoreLib` |
| Runtime API catalog | `src/leanaot/LeanAOT/runtime-apis/coreclr-net10` |
| 运行入口 | `src/tools/leanrun` 与 `scripts/dotnet10/*.ps1` |
| 测试程序集 | `ManagedNet10.Smoke`、`ManagedNet10.LegacyTests`、`ManagedNet10.NkgSmoke` |
| 宿主边界 | mock host bridge 与后续 Unity/Godot/UE adapter |

## 当前 Contract 面

| 领域 | 外部形状 | LeanCLR 内部映射 | 边界 |
| --- | --- | --- | --- |
| Runtime type identity | `Object.GetType`、`typeof(T)`、`RuntimeTypeHandle` | `RtClass` 与 reflection type object | 不用 raw pointer 猜测类型 |
| MethodTable façade | `.NET 10` `MethodTable*` / `QCallTypeHandle` | façade registry 解码到 `RtClass` | 所有入口复用统一解码 helper |
| Assembly / Module | `RuntimeAssembly`、`RuntimeModule`、metadata token query | `RtAssembly`、`RtModuleDef`、metadata cache | 不承诺完整 CoreCLR loader / ALC |
| Method / Field handle | `RuntimeMethodHandle`、`RuntimeFieldHandle`、stub object | `RtMethodInfo`、`RtFieldInfo` | unsupported shape fail fast |
| Reflection | member 查询、invoke、custom attribute metadata | LeanCLR reflection framework | 深层私有语义按 workload 补齐 |
| CustomAttributeData | metadata-only constructor / named args | attribute blob decoder | attribute 实例化不是默认路径 |
| RuntimeHelpers / Span / RVA | cctor、RVA data、inline array、prepare no-op | class/module metadata、interpreter buffer view | byref-like escape、任意 function pointer 后置 |
| Delegate / ValueType | invoke method、multicast、equals/hash | delegate metadata、invoke helper、layout scanner | 不复制 CoreCLR MethodTable layout |
| GC | collect、roots、handles、graph scan、finalizer | LeanCLR mark-sweep / handle table / finalizer registry | 分代、blocking/compacting 模式不承诺 |
| Single-thread sync | `Monitor`、基础 event wait、controlled continuation | single-thread monitor、runtime event handle、frame pump | 不承诺 ThreadPool worker / 后台线程 |
| System.IO | `File`、`FileStream`、`Path` generated P/Invoke | `platform::Kernel32` -> `os::File` / `os::Path` | watcher、ACL、overlapped IO 后置 |
| ALC / Reflection.Emit | loaded assemblies、light lambda、dynamic metadata | 受限 assembly/module façade | collectible ALC、unload、native codegen 后置 |
| Host bridge | ABI、opaque handle、dispatcher、event/callback | host function table、handle registry、main-thread queue | 不暴露真实引擎对象布局 |
| Diagnostics | 缺 API、签名不匹配、host failure | profile-aware error / managed exception | 不允许静默 stub |

## 明确非目标

- 完整 `Microsoft.NETCore.App`。
- 完整 CoreCLR hosting、PAL、ThreadPool、debugger、ALC unload。
- 动态 native codegen 和完整 `Reflection.Emit`。
- Socket / HTTP / TLS / DNS 等网络栈。
- 后台线程、跨线程 `Monitor`、复杂 `WaitHandle`、IO completion port。
- COM interop、复杂 marshaling、高级文件系统特性。

## 新增 Contract Checklist

新增 `.NET 10` runtime 入口时，至少补齐以下信息：

- 来源：CoreLib 调用链、runtime pack extern、smoke 入口或真实 workload。
- 签名：完整托管签名、P/Invoke / QCall 形状、参数传递约定。
- 映射：LeanCLR 内部落到哪个 VM 类型、metadata、GC、解释器或 host bridge helper。
- 边界：哪些输入或平台能力明确不支持，失败时输出什么诊断。
- 验收：对应 smoke / legacy / workload 命令。

## 验证入口

当前验证命令集中在 [`../src/tests/TESTING.md`](../src/tests/TESTING.md)。常用 gate 包括：

- `check_runtime_api_signatures.py --profile coreclr-net10`
- `scripts/dotnet10/api-scan.ps1`
- `scripts/dotnet10/interp-smoke.ps1`
- `ManagedNet10.LegacyTests.Program::RunAll`
- `scripts/dotnet10/nkg-smoke.ps1`
- `scripts/dotnet10/host-bridge-smoke.ps1`
- `scripts/dotnet10/aot-smoke.ps1`
