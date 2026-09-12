# LeanCLR .NET 10 支持状态

本文是当前状态页，不再记录逐日迁移步骤。历史审查与迁移日志保存在 [`archive/dotnet10-support-review-history.md`](archive/dotnet10-support-review-history.md)。

## 结论

LeanCLR 当前 `.NET 10` 目标是 **minimal net10 profile**：稳定运行项目自己的、受控的、纯逻辑 `net10.0` DLL，并对越界 API 给出清晰诊断。

这不是完整承载 `Microsoft.NETCore.App`，也不是复刻 CoreCLR。LeanCLR 会使用 `.NET 10` runtime pack / `System.Private.CoreLib` 作为 BCL 输入，但只承诺当前 profile 白名单内的 runtime contract。

## 当前基线

| 领域 | 当前状态 |
| --- | --- |
| Profile | `coreclr-net10` / `minimal-net10` 为活跃 `.NET 10` 方向 |
| Corlib | `System.Private.CoreLib` 作为 `.NET 10` core library |
| 用户程序集 | 可加载并解释执行受控 `net10.0` 纯逻辑程序集 |
| Runtime contract | 以 [`net10-runtime-contract.md`](net10-runtime-contract.md) 为准 |
| 旧测试资产 | 原仓库 managed 测试资产已通过 `.NET 10` runner/profile 收口 |
| 真实 workload | NKG/Odin/UniTask 轻量 workload 已作为真实项目 gate |
| Host bridge | mock host 已覆盖 ABI、handle registry、dispatcher、event/callback、value marshal、diagnostics |
| Mono profile | 旧 Mono profile 主路径已清理，不再作为 `.NET 10` 兼容兜底 |

## 支持范围

| 能力 | 说明 |
| --- | --- |
| 基础对象模型 | `object`、string、array、value type、enum、boxing/unboxing |
| IL 与调用 | 基础 IL、泛型、虚调用、接口调用、异常、委托 |
| 反射 | `RuntimeType`、method/field/module/assembly handle、基础 member 查询 |
| Attribute | `CustomAttributeData` metadata-only 主路径，覆盖真实 workload 常见形态 |
| RuntimeHelpers / Span | cctor、RVA data、inline array、必要 `Span<T>` / `Unsafe` 路径 |
| GC | mark-sweep、root / handle / object graph / array / value type scan、finalizer 基础语义 |
| 单线程同步 | `Monitor` / `lock` / 受控 wait / 基础 continuation，边界见 contract 文档 |
| 基础 IO | `File` / `FileStream` / `Path` 的最小跨平台 façade |
| ALC / Reflection.Emit | 只支持 light lambda / metadata façade 等受限路径 |
| Host bridge | 引擎对象通过 opaque handle、主线程 dispatcher 和稳定 ABI 暴露 |

## 非目标与受限能力

以下能力当前不属于 `minimal net10 profile` 的承诺范围。触发时应返回明确诊断或 `NotSupported`，不能用静默 stub 掩盖。

| 能力 | 当前处理 |
| --- | --- |
| 完整 `Microsoft.NETCore.App` | 后置，不作为当前完成标准 |
| 完整 CoreCLR 行为 | 后置，不复制 CoreCLR VM / PAL / hosting API |
| 多线程 runtime | `ThreadPool` worker、后台线程、复杂 `WaitHandle`、跨线程 `Monitor` 不承诺 |
| Socket / 网络 / HTTP | 不在当前 profile；如后续推进，应先建立跨平台 PAL 和 Socket façade contract |
| 完整 `Reflection.Emit` | 动态 native codegen、任意 IL emit 后置 |
| 完整 ALC / debugger | collectible ALC、unload、完整调试器协议后置 |
| 高级文件 IO | watcher、ACL、reparse point、overlapped IO 后置 |
| COM / 复杂 marshaling | 后置，只保留当前 workload 需要的最小 marshal |

## 后续扩展原则

新增能力必须按 contract-first 方式推进：

1. 先写清楚 API 来源、`.NET 10` BCL 调用形态和 LeanCLR 内部映射。
2. 更新 `coreclr-net10` catalog / whitelist / façade。
3. 明确 unsupported 边界和诊断策略。
4. 增加一个可稳定触发的 smoke、legacy 或真实 workload gate。
5. 通过 [`../src/tests/TESTING.md`](../src/tests/TESTING.md) 中相关验证后再标为支持。

如果后续决定补多线程或 Socket 网络能力，应作为新的 PAL / profile 扩展立项，而不是在当前单线程 minimal profile 中隐式扩大语义。
