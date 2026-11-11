# CoreCLR .NET 10 迁移计划与验收标准

执行进展见 [开发状态](coreclr-net10-development-status.md)。下列“本次集成实测”保留首次合并时的基线结果。

日期：2026-11-11。目标分支：`coreclr-net10`。本文用于审阅后续 minimal .NET 10 开发范围。

## 分支与合并方式

按用户指定顺序，先将 LeanCLR 自有 `coreclr` 改名为 `coreclr-net10`，再接入最新 Mono 基线，最后合入 `coreclr-wqaetly` 的差异。自有旧分支以 `backup/coreclr-before-mono-integration-20261111` 保留。

- 原自有 CoreCLR：`489cb5fc`。
- Core/Mono 基线：`771b1d0a`，合并提交 `52be5c03`，该提交的文件树与最新 Mono 相同。
- 第三方迁移来源：`2e68d746`。最终合并保留它的 Git 祖先关系，但按最新 Core 的接口重新移植适配层，不能把第三方历史测试结果直接视为本分支结果。
- 不重写 `core`、`mono`、`coreclr-wqaetly`，不修改第三方检出目录，不推送远端。

选择 merge 而不是重写公开分支的 rebase，保留两条来源及冲突处理记录；其效果是先建立最新共享运行时，再接入 .NET 10 差异。

## 架构与取舍

共用 Core 的 C++17、单一 IR、分配器、GC、泛型全局注册表、AOT 返回协议和“正常路径 AOT、异常后解释器接管”机制。BCL 通过 `LEANCLR_BCL_ADAPTER` 选择，默认 `MONO`，显式 `CORECLR_NET10` 才链接新适配器。两者不同时注册，不以 Mono BCL 填补 .NET 10 API。

.NET 10 的私有托管布局、icall/intrinsic/QCall、反射及平台包装位于 `src/runtime/bcl/coreclr-net10`；共享核心只能引用公开 VM/BCL/Host 契约。新增层依赖检查禁止核心包含该目录的私有头文件。

第三方旧解释器、旧 IR、AOT bridge/varargs 协议不覆盖当前实现。新 API catalog 被迁入独立 `coreclr-net10` profile；工具默认 `mono-4.5`，保留 Mono catalog。第三方的 AOT 帧根、varargs、部分内联布局及类型句柄改动尚未完整迁移，属于后续独立开发项。

当前仍是单线程 profile；线程启动、线程同步完整语义、完整 Microsoft.NETCore.App、完整反射/互操作与引擎集成不属于本次合并完成的能力。

## 本次集成实测

环境：Windows x64、MSVC、Release、.NET SDK 10.0.204，CoreLib 来自 Microsoft.NETCore.App 10.0.8。验收脚本输出实际 CoreLib 路径及 SHA256，避免混用补丁版本结果。

| 验证项 | 本次结果 |
| --- | --- |
| 默认 Mono 原生回归 | 41/41 通过 |
| CORECLR_NET10 leanrun 原生编译及链接 | 通过 |
| LeanAOT 工具编译与双 profile 资源 | 通过 |
| 默认 Mono 原生泛型 AOT / 禁用 AOT 对照 / EH 回退及 planner 检查 | 通过 |
| .NET 10 解释器基础入口 | 20/20 通过 |
| 依赖句柄契约 | 1/1 通过：条件可达、无根环、弱清理、传递闭包、rehash 后 slot 稳定 |

20 个入口覆盖：基础算术、值类型、装箱、GetType/typeof、类型名称和比较、泛型对象、静态委托、基础异常及消息、嵌套捕获、TargetInvocationException、异常筛选与 false/true/nested/finally 控制流、Span 及 stackalloc。完整名单在验收脚本中。

本次修复了静态接口 MethodImpl 不应占实例虚表槽、SZArrayHelper 的数组接口模板、元数据 accessor 解析过早触发虚表初始化、异常筛选入口及流程状态。依赖句柄采用条件标记与固定点扫描，不将 key/value 都视为强根；句柄记录复用现有 fallible ID 索引，无额外 side map。

已知失败：`TestRuntimeHelpers` 抛出 TypeInitializationException；`TestCommandLineArguments` 出现原生进程异常退出。它们不在通过名单中，也不能被解释器回退掩盖。其他历史大矩阵、平台、Wasm、.NET 10 AOT 和 host bridge 尚未完整复验。上述原生崩溃是下一阶段阻断项，当前分支仅为开发集成基线，不能作为稳定 .NET 10 发布版本。

布局门禁目前验证对象头及原生访问的 Exception `_message`、`_innerException`、`_stackTrace` 偏移；尚非全 CoreLib ABI 验证。依赖句柄测试验证标记协议，仍需完整堆回收、终结器和 GC 压力用例。

## 后续开发顺序与验收

| 阶段 | 具体工作 | 必须满足的验收标准 |
| --- | --- | --- |
| P0：基线稳定化 | 修复命令行参数崩溃与 RuntimeHelpers 初始化；定位到具体字段/句柄/IR；给失败根因添加最小回归 | 两个已知失败全部转绿；Release/Debug 无断言弹窗与原生崩溃；默认 Mono 41 项及 AOT 回归继续通过 |
| P0：CoreLib ABI 与类型句柄 | 为每个原生读写的私有字段建立名字、类型、偏移校验；统一 ldtoken、RuntimeTypeHandle、MethodTable、TypedReference；检查 ctor token | 支持字段均有验证；不兼容 CoreLib 明确报错；typeof/GetType/数组/泛型/反射及 typedref 往返测试通过；不靠旧 Mono 布局猜测 |
| P0：GC 与异常完整性 | Native/解释器帧根、byref/值类型内引用、异常根；依赖句柄真实堆测试和终结器闭包；补齐 filter/rethrow/finally | 连续 GC 压力下无丢根、悬空指针与无根环泄漏；ASan/UBSan 通过；EH 结果与 .NET 10 对照一致 |
| P1：minimal 解释器 API 清单 | 将 whitelist 分成已验证、明确不支持、待实现；数组接口、Span、反射基础、字符串和集合；不支持 API 稳定失败 | 每项支持能力有正/负用例；unsupported 不伪造成功；单线程边界可测试；导入历史矩阵逐项复验并公布失败名单 |
| P1：.NET 10 AOT 迁移 | 逐项适配新 profile 的句柄 ABI、icall 返回、根保存、Span/inline array；varargs 独立设计；保留最新泛型共享与返回协议 | .NET 10 与 Mono 都跑泛型/接口/委托/EH 回退矩阵；manifest 标明 AOT/回退原因；异常路径不重复执行正常路径副作用；所有生成 C++ 可编译且无缺失 helper |
| P1：跨平台与资源预算 | Windows/Linux/ARM64/Wasm 验收；限制 facade 注册及布局缓存；优化 GC membership 查询与 slot 反查 | 各目标原生构建和最小测试通过；Wasm 对照耗时、二进制大小、峰值内存；无未经解释的基线退化；热路径不按程序集/方法总量线性搜索 |
| P2：引擎最小接入 | 完善自有 host bridge、Godot/UE 宿主生命周期及错误契约 | 独立宿主重复初始化/失败/退出可复现；宿主 API 不暴露 BCL 私有对象；引擎测试与 runtime 测试分开 |

每阶段交付实现、边界说明、可复现命令和英文 commit；每阶段复杂度足够时拆成多个提交。性能、内存与稳定性必须同时评估。对 GC 对象归属的候选检查目前可能扫描 arena，slot 反查可能扫描 handle 索引；先记录实际规模及热点采样，再设计索引，不将本次功能通过视为性能验收。

## 可复现入口

```powershell
# 默认 Mono，配置后构建和执行 CTest
cmake -S src/tests/runtime-regressions -B out/net10-merge-mono -A x64
cmake --build out/net10-merge-mono --config Release --parallel 4
ctest --test-dir out/net10-merge-mono -C Release --output-on-failure

# .NET 10 集成门禁：20 个 managed 入口 + 独立句柄契约
./scripts/dotnet10/integration-baseline.ps1

# 最新 Mono 泛型 AOT、禁用 AOT 对照及 EH 回退
python scripts/test/aot-features.py --target native --config Release
```

CI 增加独立 net10 基线 job，同时保留原有 Mono/native/Wasm/AOT 检查。其他 `scripts/dotnet10` 脚本与来源文档保留用于后续迁移，未重新验收的脚本不能视为当前门禁。
