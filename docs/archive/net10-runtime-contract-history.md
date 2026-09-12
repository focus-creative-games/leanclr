# LeanCLR .NET 10 Runtime Contract Plan

状态：第七版执行基准（2026-06-30，第一阶段 gate sweep 已关闭；其后由 NKG/Odin 真实 workload 驱动追加一批增量 contract，见“第一阶段基线之后的真实 workload 增量”一节，这批改动仍在工作区，需在下一轮 gate sweep 固化）

本文档是 `coreclr-net10` / `minimal-net10` 的 contract-first 改造清单。它回答两个问题：

1. .NET 10 `System.Private.CoreLib` 期望 runtime 暴露什么形状。
2. LeanCLR 应该如何把这些形状映射到已有 VM，而不是继续复用 Mono-era 适配或重写整套 CLR。

## 目标

当前阶段的目标是让 LeanCLR 稳定运行项目自己的、受控的纯逻辑 `net10.0` DLL。实现方式不是按旧 managed / Mono 测试失败点继续补丁，而是先重建 .NET 10 主路径的 runtime contract / model，再用测试和真实 workload 验收。

第一阶段完成标准：

- 每个 net10 runtime 入口都能说明来源：CoreLib 调用链、runtime pack extern、smoke 入口或真实 workload。
- 每个 façade 都有明确的 LeanCLR 内部映射：`RtClass`、`RtMethodInfo`、`RtFieldInfo`、`RtModuleDef`、metadata cache、interpreter、GC 或 host bridge。
- `coreclr-net10` 不隐式命中 Mono-era 入口；仓库不再保留 `mono45` runtime API profile、`mono-4.5` BCL 和旧 Mono-only internal call 注册面。
- 白名单外 API 输出明确诊断或 `NotSupported`，不以宽松签名匹配继续前进。

## 当前执行批次

本轮不再把旧用例失败点当作设计入口，而是按 `.NET 10` CoreLib 真正会读取的 runtime façade 逐层收敛。当前 P0 已经从 `RuntimeType` 身份推进到 `RuntimeFieldHandle`、`RuntimeModule`、`ValueType`、delegate 相关 `MethodTable*` façade、multicast delegate allocation 和 legacy `RunAll` 基线；P1 已经覆盖 `CustomAttributeData` metadata-only、NKG/Odin 轻量 workload、单线程同步 / file I/O façade、RuntimeHelpers / Span / RVA 最小语义、AssemblyLoadContext / Reflection.Emit 受限 façade，以及 handle/reflection consolidation。P2 mock host 已覆盖 ABI skeleton、opaque handle registry、主线程 dispatcher 和 event/callback bridge；P3 已覆盖托管 wrapper contract、engine binding minimal adapter、value marshal / property-call 最小交互面和 host diagnostics / failure policy。P4.1 已完成第一阶段完成审计和全量 gate sweep，第一阶段关闭为受控 `net10.0` 纯逻辑 runtime contract 与 mock host bridge 基线。P4.2 在所有 leanclr 自有 gate 通过后执行 Mono profile 物理清理：删除 `src/libraries/mono-4.5`、旧 `src/libraries/LeanCLR` profile、`runtime-apis/mono45`、LeanAOT 根目录旧 catalog、Mono-only icall 实现、旧 mono 测试构建脚本和旧样例入口；`src/tests/managed` 下仍被 `ManagedNet10.LegacyTests` 通过 `Link` 复用的源码素材暂留。P4.3 已把 `src/tests` 原仓库测试资产在当前 `.NET 10` runner/profile 下跑通，并用 strict audit 防止旧 managed 测试体遗漏。

已验证切片：

| 批次 | 已落地 contract | 证据 | 后续要求 |
| --- | --- | --- | --- |
| P0.1 RuntimeType 身份与 Type equality | `typeof(T)`、`Object.GetType()`、`Type.GetTypeFromHandle()`、`Signature.Init` / `MethodInfo.ReturnType` 对同一 `RtClass` 走同一套 net10 façade；解释器 `bool` 返回写满栈槽，避免高位脏值影响 `Type::op_Inequality` | `ManagedNet10.LegacyTests.Program::RunLegacyDiscoverySmoke` 通过 | 保留为第一道 reflection discovery gate |
| P0.2 RuntimeModule / RuntimeFieldHandle | `RuntimeFieldHandleInternal` 可从 direct field desc、栈槽、boxed handle、`RtFieldInfo` 与 runtime field info stub 解码；`GetApproxDeclaringMethodTable` 返回 net10 MethodTable façade | `ManagedNet10.LegacyTests.Program::RunCorlibReflectionRuntimeModule` 通过 | 将所有 field handle 入口纳入同一 decode helper，继续禁止宽松签名误命中 |
| P0.3 ValueType MethodTable façade | `ValueType` QCall/PInvoke 入口不再把 `System.Runtime.CompilerServices.MethodTable*` 当 raw `RtClass*`；边界处统一解析到 LeanCLR `RtClass` | `RunCorlibValueTypeEqualsStructValueTypes`、`RunCorlibValueTypeGetHashCodeStructIsStable` 通过 | 所有 `MethodTable*` native 边界必须复用同一解析规则 |
| P0.4 Delegate MethodTable façade | `System.Delegate::GetInvokeMethod(MethodTable*)` 和 `GetMulticastInvoke(MethodTable*)` 已改为解析 net10 MethodTable façade | `ManagedNet10.LegacyTests.Program::RunRuntimeDelegateDynamicInvoke` 通过 | 所有 `MethodTable*` native 边界必须复用同一解析规则 |
| P0.5 Delegate allocation / multicast | `MulticastDelegate.NewMulticastDelegate` 触发的 `RuntimeTypeHandle.InternalAllocNoChecks_FastPath(MethodTable*)` 解析 net10 MethodTable façade，而不是 raw `RtClass*` | `ManagedNet10.LegacyTests.Program::RunRuntimeDelegateDynamicInvoke` 通过；未发现 `[delegate-dyn]` 临时日志残留 | 保留为 delegate multicast allocation gate |
| P0.6 RunAll 重新分层 | delegate 切片通过后重新跑 `ManagedNet10.LegacyTests.Program::RunAll`，把后续失败归类为 contract、façade、VM 通用语义或白名单外 API | `ManagedNet10.LegacyTests.Program::RunAll` 通过 | 保留为 P0 回归基线，不作为设计来源 |
| P1.1 CustomAttributeData metadata-only | `RuntimeCustomAttributeData` 最小路径可从 assembly / module / type / field / method / parameter / property / event target 的 metadata blob 解出 constructor arguments、property named argument 和 field named argument，不需要实例化 attribute；同时覆盖 enum、Type、int[]、Type[]、object、object[]、named enum/type/array/object 等 Odin/NKG 常见 blob 形状 | `ManagedNet10.Smoke.Program::TestCustomAttributeDataOnly` 纳入默认 smoke；`interp-smoke.ps1 -Configuration Release` 通过 | 继续接 NKG/Odin 轻量 attribute 枚举作为真实 workload gate |
| P1.2 NKG/Odin light workload | `ManagedNet10.NkgSmoke` 使用真实 NKGGameFramework、OdinSerializer 与 UniTask `net10.0` 输出目录验证轻量反射、attribute 枚举和白名单 API 边界；默认 sampler/Hosting 完整面仍在第一阶段外 | `scripts/dotnet10/api-scan.ps1` 扫描 NKG core/Odin/UniTask：unsupported `0`；`scripts/dotnet10/nkg-smoke.ps1` 通过 | 保留为真实 workload gate；后续失败按 contract、façade、VM 通用语义或白名单外 API 归类 |
| P1.3 Single-thread sync façade | 单线程 profile 下补齐 `CurrentManagedThreadId`、Monitor fast path / wait PInvoke、ManualResetEvent / WaitAny 最小路径；不扩展多线程调度、ThreadPool worker 或 OS thread affinity | `ManagedNet10.LegacyTests.Program::RunCorlibThreading` 通过；`ManagedNet10.LegacyTests.Program::RunCorlibMonitor` 通过 | 保留为 single-thread sync regression gate；Task/dispatcher 只接可控 continuation |
| P1.4 System.IO / Kernel32 platform façade | .NET 10 `FileStream` / `File` / `Path` 触发的 Kernel32 generated PInvoke 统一落到 LeanCLR cross-platform `platform::Kernel32` / `os::File` / `os::Path`，非 Windows 走 POSIX fallback 并写回 Win32-style last error | `ManagedNet10.LegacyTests.Program::RunCorlibIO` 通过 | 保留为 file I/O regression gate；完整 watcher、ACL、reparse point、overlapped I/O 后置 |
| P1.5 RuntimeHelpers / Span / RVA | `RuntimeHelpers.RunClassConstructor` / `RunModuleConstructor`、stack check、`CompileMethod` / `PrepareMethod`、RVA `InitializeArray` / `CreateSpan`、inline array span helper 和 bitwise/reference checks 统一映射到 LeanCLR class/module/interpreter metadata；method preparation 在解释 profile 中是 no-op façade | `ManagedNet10.LegacyTests.Program::RunCorlibRuntimeHelpers`、`ManagedNet10.Smoke.Program::TestRuntimeHelpers`、`ManagedNet10.Smoke.Program::TestSpan`、`ManagedNet10.LegacyTests.Program::RunNet10SpanBinaryPrimitives` 通过 | 保留为 RuntimeHelpers/Span regression gate；byref-like escape、任意 function pointer 和动态 native codegen 不属于目标 |
| P1.6 AssemblyLoadContext / Reflection.Emit limited façade | `AssemblyLoadContext` 初始化、已加载程序集枚举、release bookkeeping、`RuntimeAssemblyBuilder.CreateDynamicAssembly` 和 `ModuleHandle.GetDynamicMethod` 只承诺 LeanCLR metadata / interpreter 能消费的受限动态程序集与 light lambda 路径；collectible ALC、LoaderAllocator、unload 和任意动态 IL native codegen 不属于目标 | `ManagedNet10.LegacyTests.Program::RunCorlibLightLambda` 通过 | 保留为 Reflection.Emit / light lambda regression gate；后续动态方法失败先区分 metadata façade 缺口和动态 codegen 非目标 |
| P1.7 Handle / reflection consolidation | type / method / field / module / assembly handle 入口统一经 `vm::Reflection` 边界解码；`MethodTable*` 走 net10 façade registry；QCall object/slot 参数先做 GC allocated-object guard，再映射到 LeanCLR metadata；unsupported handle shape 输出明确错误，不靠宽松指针猜测前进 | `RunLegacyDiscoverySmoke`、`RunCorlibReflectionRuntimeModule`、`RunRuntimeDelegateDynamicInvoke`、`RunCorlibValueTypeEqualsStructValueTypes`、`RunCorlibValueTypeGetHashCodeStructIsStable`、`RunCorlibRuntimeHelpers`、`RunNet10SpanBinaryPrimitives`、`api-scan.ps1`、`nkg-smoke.ps1` 通过 | 保留为 reflection/handle regression gate；新增 CoreLib handle 入口必须复用同一 helper 或显式说明例外 |
| P1.8 GC collection / finalizer cleanup | `.NET 10` `GC._Collect(int,int,byte)` 桥接到 `vm::GC::collect(generation)`，`GC.CollectionCount(int)` 走两参数 `_CollectionCount` façade；真实 mark-sweep 后触发的 CoreLib cleanup 入口 `Thread.InternalFinalize()` 和 `RuntimeMethodHandle.Destroy(RuntimeMethodHandleInternal)` 已纳入 net10 façade | `ManagedNet10.LegacyTests.Program::RunGcCollection` 整类旧 `TC_GC_Collection` 通过；`RunAll`、默认 `interp-smoke.ps1`、`api-scan.ps1`、`nkg-smoke.ps1` 和签名 gate 通过 | 保留为 GC collection regression gate；分代、blocking/compacting 模式和完整 CoreCLR GC 模型不属于当前 profile |
| P1.9 GC roots / handles | 旧 GC roots / handles 用例直接验证静态根、实例字段链、继承字段链、strong/weak/pinned handle、同对象多 weak handle、强 handle 传递标记，以及 static GC bitmap bit 0 边界，复用 LeanCLR 当前显式 GCHandle 保活模型 | `ManagedNet10.LegacyTests.Program::RunGcRootsAndHandles` 整类旧 `TC_GC_Roots` / `TC_GC_Handles` 通过；`ManagedNet10.LegacyTests.Program::RunGcStaticBitmapBitZero` 通过；`RunAll` 和默认 `interp-smoke.ps1` 通过 | 保留为 GC root scanning 与 handle sweep regression gate；托管局部变量自动栈根仍不作为当前 LeanCLR profile 承诺 |
| P1.10 GC object graph scan | 旧 graph scan 用例验证链、断链、环、自引用、菱形共享叶子、宽图、深链 suffix 和重复 collect 的 mark-sweep 可达性语义 | `ManagedNet10.LegacyTests.Program::RunGcScanGraph` 整类旧 `TC_GC_Graph` 通过；`RunAll` 和默认 `interp-smoke.ps1` 通过 | 保留为对象图扫描 regression gate；后续数组、value type 和 instance field scan 分节点继续迁移 |
| P1.11 GC sweep | 旧 sweep 用例验证 small object / big byte array 的回收与保活、small/big 混合批量 sweep，以及静态数组根清除后的对象回收 | `ManagedNet10.LegacyTests.Program::RunGcSweep` 整类旧 `TC_GC_Sweep` 通过；`RunAll` 和默认 `interp-smoke.ps1` 通过 | 保留为 mark-sweep reclaim regression gate；后续更复杂 scan 类型分节点继续迁移 |
| P1.12 GC finalizer | 旧 finalizer 用例验证不可达 finalizable 对象会执行 finalizer、`SuppressFinalize` 阻止执行、`WaitForPendingFinalizers` drain pending 队列、`ReRegisterForFinalize` 可让 resurrected 对象再次 finalization，以及普通 object 不影响自定义计数器；`.NET 10` public `GC.SuppressFinalize(object)` / `GC.ReRegisterForFinalize(object)` 直接落到 LeanCLR finalizer registry，private `SuppressFinalizeInternal` 与 QCall `_WaitForPendingFinalizers` / `ReRegisterForFinalize(ObjectHandleOnStack)` 也登记到 net10 façade | `ManagedNet10.LegacyTests.Program::RunGcFinalizer` 整类旧 `TC_GC_Finalizer` 通过；`RunAll`、默认 `interp-smoke.ps1`、`api-scan.ps1`、`nkg-smoke.ps1` 和签名 gate 通过 | 保留为 finalizer regression gate；完整 CoreCLR finalizer worker、CriticalFinalizerObject 和 SafeHandle release ordering 不属于当前 profile |
| P1.13 GC array scan | 旧 array scan 用例验证 reference SZArray slot、slot clear、空 reference array、primitive array 不误标记、struct array 嵌套引用、宽 reference array、jagged object array、multidimensional object array，以及 big byte array pinned/unrooted reclaim 语义 | `ManagedNet10.LegacyTests.Program::RunGcScanArrays` 整类旧 `TC_GC_Arrays` 通过；`RunAll` 和默认 `interp-smoke.ps1` 通过 | 保留为 array scanning regression gate；non-sealed element、instance field 和 value type scan 继续分节点迁移 |
| P1.14 GC non-sealed array element scan | 旧 non-sealed array element 用例验证静态元素类型为开放 base class 或 interface 时，GC 仍按运行时实例对象扫描 derived/implementor 的引用字段；同时覆盖 multidimensional base array 和 primitive-only sealed struct array 不误标记 | `ManagedNet10.LegacyTests.Program::RunGcScanArrayNonSealedElement` 整类旧 `TC_GC_ArrayNonSealedElement` 通过；`RunAll` 和默认 `interp-smoke.ps1` 通过 | 保留为 array slot runtime-object scanning regression gate；instance field 和 value type scan 继续分节点迁移 |
| P1.15 GC instance field scan | 旧 instance field scan 用例验证直接 reference field、双字段 container 保活、container 不可达后 child 回收、string field 保活与 clear、继承 base reference field、derived reference field、嵌套 struct reference field、object 上的 array field、primitive-only object、替换 reference field 后旧 child 回收，以及 null reference field 不误标记无关对象 | `ManagedNet10.LegacyTests.Program::RunGcScanInstanceFields` 整类旧 `TC_GC_InstanceFields` 通过；`RunAll` 和默认 `interp-smoke.ps1` 通过 | 保留为 object instance field scanning regression gate；value type scan 继续分节点迁移 |
| P1.16 GC value type scan | 旧 value type scan 用例验证深层嵌套 struct reference、静态深层嵌套 struct root、struct array 每个元素的嵌套 reference、primitive-only value type array 不误标记、实例 struct field at offset zero，以及静态 struct field at offset zero | `ManagedNet10.LegacyTests.Program::RunGcScanValueTypes` 整类旧 `TC_GC_ValueTypes` 通过；`RunAll` 和默认 `interp-smoke.ps1` 通过 | 保留为 value type reference scanning regression gate；继续清点剩余旧 managed 测试资产并分节点迁移 |
| P1.17 Runtime interface implementation dispatch | 旧 interface default method 用例在当前非 Unity net10 profile 下覆盖普通接口实现调用和继承接口转基类接口调用；该节点验证 LeanCLR interface dispatch 对直接 interface slot 和 inherited interface contract 的解析稳定 | `ManagedNet10.LegacyTests.Program::RunRuntimeInterfaceDefaultMethod` 整类旧 `TC_InterfaceDefaultMethod` 通过；`RunAll` 和默认 `interp-smoke.ps1` 通过 | 保留为 interface dispatch regression gate；真正 C# default interface method body 仍需单独 contract/test 后置 |
| P1.18 Marshal size/layout façade | `.NET 10` `Marshal.SizeOf` 通过 generated P/Invoke `Marshal.<SizeOfHelper>g____PInvoke|3_0(QCallTypeHandle,int)` 进入 runtime，LeanCLR 将 `QCallTypeHandle` 解回 `RtClass` 并复用现有字段布局初始化与 instance-size 计算；旧 `TC_Marshal` 覆盖 value type reference field、explicit layout、pack=1/2/4/8、explicit size、sequential pack 和 fixed buffer size | `ManagedNet10.LegacyTests.Program::RunRuntimeMarshal` 整类旧 `TC_Marshal` 通过；`RunAll`、默认 `interp-smoke.ps1`、`api-scan.ps1`、`nkg-smoke.ps1` 和签名 gate 通过 | 保留为 marshal layout regression gate；完整 `StructureToPtr` / `PtrToStructure` 与复杂 marshaling 后置 |
| P1.19 AOT/interpreter managed call compatibility | 旧 AotTests call / callvirt 用例在 net10 解释 profile 下覆盖 `[AotMethod(false)]` 标注方法的普通调用、struct instance 调用、tuple array 初始化、class virtual/override dispatch、interface dispatch、boxed struct interface dispatch、struct `ToString` override 和 boxed `IDisposable.Dispose` 更新 | `ManagedNet10.LegacyTests.Program::RunAotCallInterop` 组合旧 `TC_Call_AotInterp` / `TC_CallVir_AotInterp` 通过；`RunAll` 和默认 `interp-smoke.ps1` 通过 | 保留为 managed call/callvirt compatibility regression gate；真正 LeanAOT native/interpreter 混跑仍按 AOT 专用 gate 后置 |
| P1.20 AOT misc managed semantics | 旧 AotTests misc 用例在 net10 解释 profile 下覆盖多维数组创建与负长度溢出、`ldsflda` static/thread-static byref 写入、静态构造函数执行顺序、条件分支后的 eval stack 返回、`Stopwatch` 非负 elapsed ticks/ms，以及 `MonoPInvokeCallback` 标记方法携带 ref struct 参数的普通托管调用 | `ManagedNet10.LegacyTests.Program::RunAotMisc` 组合旧 `TC_NewMdArray`、`TC_ldsflda`、`TC_StaticCtorOrder`、`TC_EvalStackNotEmpty`、`TC_System_Diagnostics_StopWatch` 和 `TC_MonoPInvokeCallback` 通过；`RunAll` 和默认 `interp-smoke.ps1` 通过 | 保留为 AOT misc managed-semantics regression gate；真正 reverse P/Invoke / native callback 仍按 AOT/native 专用 gate 后置 |
| P1.21 Inherited interface implementation dispatch | 旧 `TC_AOTTypeImplInterpInterface` 用例验证派生类型声明接口、但接口方法实现来自基类 virtual method 时，LeanCLR interface dispatch 能沿继承链解析到基类实现 | `ManagedNet10.LegacyTests.Program::RunRuntimeAotTypeImplInterpInterface` 通过；`RunRuntimeLanguageFeatures`、`RunAll` 和默认 `interp-smoke.ps1` 通过 | 保留为 inherited interface implementation regression gate；更复杂 AOT/native interface bridge 仍按专用 gate 后置 |
| P1.22 IL conv.ovf overflow semantics | 旧 `ILTests.Wrappers.TC_conv_ovf` 用例通过手写 IL native assembly 验证 `conv.ovf.i1` 和 `conv.ovf.u1.un` 的边界值与溢出异常；net10 侧生成同名 `ILTests.Native`，仅把 corlib reference 调整为 `System.Runtime` reference contract，保留原 IL opcode 语义 | `ManagedNet10.LegacyTests.Program::RunConvertOverflowIl` 通过；`RunConvertOverflow`、`RunAll` 和默认 `interp-smoke.ps1` 通过 | 保留为 hand-written IL overflow conversion regression gate；继续迁移其他 IL-only 用例时复用同一 ILAsm 输出模型 |
| P1.23 Legacy placeholder inventory closure | 旧 `TC_dynamic` 的测试体在原素材中已全部注释，旧 `TC_Syste_Span` 是带 `[IgnoreTest]` 的 blocked placeholder；net10 legacy 项目按原 runner 语义纳入这两个库存项，但不把它们解释为 dynamic binder 或完整 `System.Span<T>` runtime 支持 | `ManagedNet10.LegacyTests.Program::RunRuntimeDynamicPlaceholder`、`RunCorlibIntrinsicSpanPlaceholder`、`RunRuntimeLanguageFeatures`、`RunCorlibArrayBufferIntrinsics`、`RunAll` 和默认 `interp-smoke.ps1` 通过；旧 managed `[UnitTest]` 扫描仅剩已由 net10 本地替代覆盖的 `TC_sub` | 保留为旧测试资产清点记录；后续若要承诺 dynamic binder 或完整 Span，需要新增真实行为 contract 和非占位测试 |
| P1.24 Core bootstrap direct-call coverage | 旧 `CoreTests/Bootstrap` basic-tester 直连用例迁入 `ManagedNet10.LegacyTests`：覆盖 int/string/struct 返回、int/string 参数传递和同类型静态方法调用，保留旧原子 bootstrap 能力检查但改由 net10 解释 profile 执行 | `ManagedNet10.LegacyTests.Program::RunCoreBootstrap` 通过；`RunAll`、默认 `interp-smoke.ps1`、`api-scan.ps1` unsupported `0` 通过 | 保留为 managed bootstrap regression gate；后续清点 old direct-runner 方法时继续区分真实 runtime 能力与旧 native harness 形态 |
| P1.25 Core App direct-runner coverage | 旧 `CoreTests/App.cs` basic runner 直调方法迁入 `ManagedNet10.LegacyTests`：覆盖分支/goto/switch、ref local、unchecked byte cast、boxing/unboxing、SZArray 创建和元素读写、typed reference、stackalloc、实例/静态字段 byref 写入、私有静态调用、虚调用，以及嵌套 try/catch/finally/rethrow 控制流；项目显式设置 `StartupObject` 以允许链接旧空 `Main` | `ManagedNet10.LegacyTests.Program::RunCoreAppDirect` 通过；`RunAll`、`api-scan.ps1` 和默认 `interp-smoke.ps1` 复验 | 保留为旧 basic runner managed semantics regression gate；`CallCustomPInvoke` 只作为旧源码依赖保留，真实 native P/Invoke harness 另按 host/native gate 归类 |
| P1.26 Interp callvirt / vtable coverage | 恢复旧 `TC_callvir_interp.cs` 真测试体，并链接解释侧虚调用 fixture：覆盖 class virtual call、interface dispatch、struct interface boxing/constrained call、泛型约束调用、协变/逆变泛型接口、`GetType()` / `GetHashCode()` / `ToString()` constrained 路径和 big struct duplicate assignment | `ManagedNet10.LegacyTests.Program::RunFunctionCallvirtInterp` 通过；`RunAll`、默认 `interp-smoke.ps1`、`api-scan.ps1` unsupported `0` 复验 | 保留为解释器 vtable/interface/constrained-call regression gate；后续不再把整文件注释的虚表用例视为可跳过占位 |
| P1.27 Core App2 direct-runner coverage | 旧 `CoreTests/App2.cs` direct runner 迁入 `ManagedNet10.LegacyTests`：覆盖 object hash/type 查询、`MemberwiseClone`、`char[]` / `char*` / `sbyte*` / `Encoding` 字符串构造、`string.Concat`、`string.Intern`、`string.IsInterned` 和 `string.Length` | `ManagedNet10.LegacyTests.Program::RunCoreApp2Direct` 通过；`RunAll`、默认 `interp-smoke.ps1`、`api-scan.ps1` unsupported `0` 复验 | 保留为 core object/string managed semantics regression gate；`TestPinvoke` 仍作为旧源码 metadata 保留但不在纯 managed gate 中调用，真实 native P/Invoke harness 另按 host/native gate 归类 |
| P1.28 Legacy managed asset audit closure | 旧 managed 源清点中仅剩原始 `SharedTests/Instructions/Arithmetic/TC_sub.cs` 未按原路径 `Link`：该文件已由 net10 本地替代版执行，并修正 `UIntPtr` subtraction 语义；其他含 `[UnitTest]` / `[AotMethod]` / 断言的旧 managed 测试体均必须链接或被 audit 明确拒绝 | 旧 managed active-ish 扫描仅剩原始 `TC_sub.cs :: UnitTest`；替代版 20 个 `[UnitTest]` 方法与原始文件匹配；`ManagedNet10.LegacyTests.Program::RunArithmeticSub`、`RunAotCallInterop`、`RunAotMisc`、`RunAll`、默认 `interp-smoke.ps1`、`api-scan.ps1` unsupported `0` 均通过 | 视为 LeanCLR 自有 managed 测试资产清点闭环；不把 net10 替代过的源码副本当作未迁移测试体 |
| P1.29 Nullable enum boxing fixture coverage | 旧 `TestNotInitedNullableType.AOTBoxNotInitializedGenericStruct` 虽位于 fixture 目录且没有 `[UnitTest]`，但包含泛型 nullable enum boxing 断言；net10 侧链接原 fixture，并用 `RuntimeNullableBoxingTests` wrapper 纳入 `RunAll` | `ManagedNet10.LegacyTests.Program::RunRuntimeNotInitializedNullableBoxing` 通过；`RunAll`、默认 `interp-smoke.ps1`、`api-scan.ps1` unsupported `0` 复验 | 保留为 nullable value type boxing regression gate；继续清点无 `[UnitTest]` 但含断言/可执行入口的旧 fixture 方法 |
| P1.30 Legacy managed audit gate | `scripts/dotnet10/legacy-managed-audit.ps1` 展开 `ManagedNet10.LegacyTests.csproj` 的旧 managed `Link` / `LinkBase` / wildcard，扫描未链接旧源码中的未注释 `[UnitTest]`、`[AotMethod]`、`Assert`、旧 runner `Main` 和 public static helper；除已验证的原始 `TC_sub.cs` net10 本地替代外，出现新 active-ish marker 即失败 | `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\legacy-managed-audit.ps1` 通过：374 个旧 managed 文件已链接，1 个未链接 active-ish 文件为原始 `TC_sub.cs`，替代版 20 个 `[UnitTest]` 方法匹配 | 作为 LeanCLR 自有 managed 测试资产库存 gate；后续新增旧源码断言或取消注释测试体必须链接进 net10 legacy 项目或显式更新分类 |
| P1.31 Thread-static field instruction coverage | 恢复旧 `TC_ldsfld_interp_threadstatic.cs` / `TC_stsfld_interp_threadstatic.cs` 真测试体，并把 `TypeThreadStaticFields` / `TypeThreadStaticFields2` fixture 纳入 net10 legacy 项目；同时恢复 `TestCaseBase.SetUp/TearDown` 和 `LegacyTestRunner` 每测试方法生命周期调用，保证 thread-static 初始化按旧 runner 语义执行 | `ManagedNet10.LegacyTests.Program::RunFieldThreadStatic` 通过；`RunAll` 通过；`legacy-managed-audit.ps1` 通过，链接旧 managed 文件增至 374 个 | 保留为 `[ThreadStatic]` primitive / enum / object / value type static field load/store regression gate；生命周期钩子作为旧实例测试 runner contract 保留 |
| P2.1 Host Bridge ABI skeleton | 新增 `LeanClrHostBridgeFunctions` C ABI、ABI version、capability flags、状态码、错误字符串和最小 managed entry callback；mock host 只验证初始化、函数表校验和托管静态入口回调，不加载真实 Unity/Godot 或完整 runtime hosting API | `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario AbiSkeleton` 通过 | 保留为 host bridge ABI regression gate；后续 P2.2-P2.4 在同一脚本上扩展 scenario |
| P2.2 Opaque handle registry | 在 host function table 中定义宿主对象 handle 创建、retain/release、判活和销毁通知；mock host 维护 opaque handle table，托管侧只接收 `LeanClrHostHandle`，不暴露真实宿主对象指针 | `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario HandleRegistry` 通过 | 保留为 handle lifecycle regression gate；后续 dispatcher/event callback 必须复用同一 handle 失效诊断语义 |
| P2.3 Main-thread dispatcher | 在 host function table 中定义主线程投递、next-frame pump、同步调用和 reentry guard；mock host 只承诺 FIFO 队列、受控 pump、callback 结果回传和托管异常诊断，不扩展后台 worker 调度 | `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario Dispatcher` 通过 | 保留为 dispatcher regression gate；event/callback bridge 必须通过同一主线程队列回到托管调用边界 |
| P2.4 Event / callback bridge | 在 host function table 中定义 event subscription token、取消订阅和宿主事件触发；mock host 触发事件时先投递到主线程 dispatcher，再由 pump 进入托管回调边界，取消订阅后触发为 no-op | `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario EventCallback` 通过 | 保留为 event/callback regression gate；真实 UnityEvent / Godot Signal 和复杂 Variant marshal 后置 |
| P3.1 Managed host wrapper contract | 新增 `ManagedNet10.Smoke.HostBridgeWrapperSmoke`，定义托管侧 `HostObject` / `HostEventSubscription` / `HostDispatcher` 最小 wrapper contract；wrapper 只保存 opaque handle / subscription token，通过 mock bridge 把 host status 转成 `ObjectDisposedException` 或 `HostBridgeException` | `scripts/dotnet10/interp-smoke.ps1 -Configuration Release -Entry "ManagedNet10.Smoke.HostBridgeWrapperSmoke::Run"` 通过；`scripts/dotnet10/api-scan.ps1 -Configuration Release` unsupported `0`；P2 host bridge 四场景仍通过 | 保留为 managed wrapper regression gate；后续真实 Unity/Godot 接入必须先落到同一 wrapper contract，再连接具体引擎对象模型 |
| P3.2 Engine binding minimal adapter | 新增 native `EngineAdapter` mock 场景和 `ManagedNet10.Smoke.EngineBindingSmoke`，把引擎节点创建、事件订阅、事件触发、主线程 pump 和宿主销毁诊断串到 P2/P3.1 contract；托管 `EngineNode` 只持有 `HostObject`，不接触真实引擎指针 | `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario EngineAdapter` 通过；`scripts/dotnet10/interp-smoke.ps1 -Configuration Release -Entry "ManagedNet10.Smoke.EngineBindingSmoke::Run"` 通过；`scripts/dotnet10/api-scan.ps1 -Configuration Release` unsupported `0` | 保留为 engine binding adapter regression gate；后续属性/方法调用必须继续经过 opaque handle、dispatcher 和明确 value marshal |
| P3.3 Value marshal / property-call adapter | 在 host bridge ABI 中新增 `LeanClrHostValue` / `LeanClrHostVector3`、value marshal capability，以及 property get/set / command invoke 入口；native `ValueMarshal` 场景和 managed `ValueMarshalSmoke` 覆盖 bool/int/float/string/vector3、属性 round trip、`MoveBy` / `Damage` command、错误类型诊断和销毁后访问 | `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario ValueMarshal` 通过；`scripts/dotnet10/interp-smoke.ps1 -Configuration Release -Entry "ManagedNet10.Smoke.ValueMarshalSmoke::Run"` 通过；`scripts/dotnet10/api-scan.ps1 -Configuration Release` unsupported `0` | 保留为 value marshal regression gate；复杂 Variant、数组 view、UnityEngine.Object / Godot Object 语义后置 |
| P3.4 Host diagnostics / failure policy | 在 host bridge ABI 中新增 log level enum、diagnostics capability 和 `report_managed_exception` 回调；native `Diagnostics` 场景和 managed `DiagnosticsSmoke` 覆盖 info/warning/error 日志、托管异常类型/消息/栈字符串回传、不完整诊断拒绝，以及 host failure 到 `HostBridgeException` 的转换 | `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario Diagnostics` 通过；`scripts/dotnet10/interp-smoke.ps1 -Configuration Release -Entry "ManagedNet10.Smoke.DiagnosticsSmoke::Run"` 通过；`scripts/dotnet10/api-scan.ps1 -Configuration Release` unsupported `0` | 保留为 diagnostics regression gate；完整 debugger、符号解析和宿主异常对象跨 ABI 后置 |
| P4.1 First-stage completion audit | 对照第一阶段完成标准复核 P0-P3 已验证切片，补齐 consolidated verification log，并确认 contract evidence、catalog evidence、runtime evidence 都有当前证据 | `check_runtime_api_signatures.py --profile coreclr-net10` 通过；`api-scan.ps1 -Configuration Release` 两组扫描 unsupported `0`；`nkg-smoke.ps1 -Configuration Release` 通过；`ManagedNet10.LegacyTests.Program::RunAll` 通过；P2/P3 host bridge 七个 scenario 与 P3 managed wrapper 四个入口全通过 | 第一阶段关闭为 minimal `net10.0` 纯逻辑 DLL + mock host bridge 基线；真实 Unity/Godot SDK 绑定、完整 sampler/Hosting、ALC/debugger 仍是后置边界；动态 native codegen 和多线程 runtime 不进入计划 |
| P4.2 Mono profile physical cleanup | 删除 `mono-4.5` BCL、`mono45` runtime API profile、LeanAOT 旧根 catalog、Mono-only internal call 注册面和依赖这些资产的旧脚本/样例；LeanAOT 默认 runtime API profile 改为 `coreclr-net10`，runtime corlib 加载不再回退 `mscorlib` | 清理前 `RunAll`、默认 `interp-smoke.ps1`、`api-scan.ps1`、`nkg-smoke.ps1`、签名 gate、host bridge 七场景和 P3 managed wrapper 四入口均通过；清理后完整复跑上述 gate：`check_runtime_api_signatures.py --profile coreclr-net10`、默认 `interp-smoke.ps1`、`ManagedNet10.LegacyTests.Program::RunAll`、`api-scan.ps1` unsupported `0`、`nkg-smoke.ps1`、host bridge 七场景和 P3 managed wrapper 四入口均通过 | 保留 `src/tests/managed` 中被 net10 legacy 项目 `Link` 复用的源码素材；后续若要彻底删除旧测试目录，需要先把这些源码迁入 `src/tests/managed-net10` |
| P4.3 LeanCLR self-test gate sweep | 将当前 LeanCLR 自有 gate 固化为解释器默认 smoke、legacy `RunAll`、legacy managed 资产审计、API whitelist、runtime API signature、NKG/Odin workload、host bridge 七场景、P3 managed wrapper 四入口、interpreter smoke matrix，以及 LeanAOT net10 smoke native build/run | `interp-smoke.ps1` 默认入口通过；legacy `RunAll` 通过；`legacy-managed-audit.ps1` 通过：374 个旧 managed 文件已链接，唯一未链接 active-ish `TC_sub` 由 20 个匹配 `[UnitTest]` 的 net10 替代版覆盖；`interp-smoke-matrix.ps1 -Configuration Release` 默认全量通过 65 个入口；`api-scan.ps1` 两组扫描 unsupported `0`；`nkg-smoke.ps1` 通过；`aot-smoke.ps1 -Configuration Release -NativeRun` 生成 C++、构建 `aot-tester.exe` 并运行 `ok!`；host bridge 七场景全通过；runtime API signature gate 全部匹配 | 作为 Mono 清理后的 LeanCLR 自有测试完成口径；`src/tests` 原仓库测试资产已在当前 `.NET 10` runner/profile 下跑通，但不把 Mono profile、旧 Mono 脚本或整文件注释的虚表测试纳入可接受路径 |

第一阶段完成审计结果：

- contract evidence：P0-P1 将 CoreLib 入口收敛到 type / handle / reflection / delegate / RuntimeHelpers / Span / IO / single-thread sync / ALC / Reflection.Emit 的 LeanCLR façade 映射；P2-P3 将 engine integration 限定在 opaque handle、dispatcher、event、value marshal、diagnostics 的 host bridge contract。
- catalog evidence：`coreclr-net10` active profile 由 runtime pack extern gate 校验，`icalls.json`、`intrinsics.json`、`pinvokes.json` 当前全部匹配 extern 签名；`mono45` profile 与旧根 catalog 已删除，不再作为 fallback。
- runtime evidence：legacy `RunAll`、smoke 子入口、NKG/Odin 轻量 workload、host bridge native scenario 和 managed wrapper scenario 均有独立命令可复验。
- 第一阶段边界：当前目标是受控纯逻辑 `net10.0` DLL 与 mock host bridge，不承诺完整 `Microsoft.NETCore.App`、真实 Unity/Godot SDK binding、完整 sampler/Hosting、完整 CoreCLR ALC/debugger、复杂 Variant / engine object 语义、动态 native codegen 或多线程 runtime。

当前结论：`src/tests` 下除 `src/tests/managed-net10` 外的原仓库测试资产，已通过当前 `.NET 10` runner/profile 验收。`src/tests/managed-net10` 是验证入口和少量 net10 适配层，不作为原仓库测试资产本身计数。

## 非目标

- 不重写 LeanCLR 的 metadata loader、类型系统、对象模型、解释器、GC、异常、委托和泛型底座。
- 不复制 CoreCLR 的 MethodTable、loader、GC、PAL 和完整 VM 实现。
- 不实现动态 native codegen、多线程 runtime 或 ThreadPool worker 调度模型。
- 不承诺第一阶段完整承载 `Microsoft.NETCore.App`。
- 不再把 extern diff 归零作为近期目标；extern diff 只作为定位工具。
- 不保留 `mono45` / `mono-4.5` 兼容 profile；Unity/Godot 接入必须走 P2/P3 host bridge contract，而不是复用 Mono BCL profile。

## 输入来源

| 来源 | 用途 | 输出 |
| --- | --- | --- |
| .NET 10 `System.Private.CoreLib` 源码 | 确认托管类型、私有字段、QCall/PInvoke/Intrinsic 调用链 | contract map |
| CoreCLR VM / native entry 源码 | 理解入口语义，但不照搬实现 | LeanCLR 映射说明 |
| .NET 10 runtime pack extern 清单 | 确认签名和调用入口是否仍存在 | `coreclr-net10` catalog 差异 |
| `ManagedNet10.Smoke` | 稳定触发小能力入口 | 每个领域的最小验收 |
| 原作者 managed 测试源码素材 | 证明 LeanCLR runtime 能力没有退化；当前通过 `ManagedNet10.LegacyTests` 链接到 `net10.0` 项目 | 分阶段回归验收 |
| NKGGameFramework 纯逻辑 DLL | 证明真实项目结构能跑 | workload 验收 |

## Contract 领域

### 1. Type / RuntimeType

外部形状：

- `object.GetType()` 返回 CoreLib 可接受的 `System.RuntimeType` / `System.Type` 对象。
- `RuntimeType` 的 `Name`、`FullName`、`BaseType`、`IsAssignableFrom`、`IsSubclassOf` 等基础查询稳定。
- `typeof(T)`、boxed value type、array type、generic type、byref-like type 的 type handle 能被区分。

内部映射：

- `RuntimeType` façade 持有或可解析到 `metadata::RtClass*`。
- 类型名、namespace、assembly、metadata token 从 `RtClass` / `RtModuleDef` 派生。
- assignable / subclass / interface 判断优先复用 `vm::Class` 的现有语义。

验收入口：

- `Object.GetType()` on reference type / boxed value type。
- `RuntimeType.Name` / `FullName`。
- `RuntimeTypeHandle.GetBaseType` / `is_subclass_of`。

### 2. RuntimeTypeHandle

外部形状：

- `RuntimeTypeHandle` / `RuntimeTypeHandleInternal` / QCall handle 参数能在 CoreLib 调用链中往返。
- 支持 array rank、element type、generic definition、generic instantiation、metadata token、module、assembly 等查询。

内部映射：

- handle 只表示 LeanCLR 中稳定的 `RtClass` 身份，不暴露 CoreCLR MethodTable 假设。
- `QCallTypeHandle` 等 façade 应在边界处转换为 `RtClass*`，并集中校验空值、类型不匹配和 unsupported 状态。

验收入口：

- `RuntimeTypeHandle.GetArrayRank`。
- `RuntimeTypeHandle.GetElementType`。
- `RuntimeTypeHandle.GetModule` / `GetAssembly`。
- `RuntimeTypeHandle.GetGenericTypeDefinition`。

### 3. RuntimeMethodHandle / RuntimeFieldHandle

外部形状：

- 反射方法、字段和 handle 能互相转换。
- `GetFunctionPointer`、`GetValueDirect`、`SetValueDirect`、RVA field 查询等入口有明确支持或明确拒绝。

内部映射：

- method handle 映射到 `metadata::RtMethodInfo*`。
- field handle 映射到 `metadata::RtFieldInfo*`。
- function pointer 只在 LeanCLR 已有 AOT/native bridge 能表达时支持；解释路径默认不伪造不可调用指针。
- field get/set 复用现有 field offset、static data、boxed value type 和 GC write barrier 规则。
- direct metadata pointer、栈槽、boxed handle、runtime reflection object 和 stub object 必须先进入同一 decode helper，再交给具体业务入口。
- `MethodTable*` 参数必须先解析为 net10 MethodTable façade registry 中的类型身份，不允许在 native 边界直接当作 `RtClass*` 使用。

验收入口：

- 反射枚举字段/方法。
- private field 基础读写。
- RVA initializer / static readonly data。

### 3.1 Net10 handle boundary consolidation

外部形状：

- CoreLib 传入的 `RuntimeTypeHandle`、`RuntimeMethodHandleInternal`、`RuntimeFieldHandleInternal`、`QCallTypeHandle`、`QCallModule`、`QCallAssembly` 和 `MethodTable*` 都有集中解析路径。
- QCall 参数可以是 direct managed object、object handle slot 或 native handle；slot/object 形状必须先通过 GC allocated-object guard 和 runtime reflection class 校验。
- unsupported handle 形状返回 `ArgumentNull`、`BadImageFormat`、`Argument` 或明确 `NotSupported`，不静默退回 Mono-era 宽松签名或 raw pointer 猜测。

内部映射：

- `RuntimeTypeHandle` / `MethodTable*` 经 `get_net10_method_table`、`get_type_sig_from_net10_method_table`、`get_class_from_net10_method_table` 和 `get_type_sig_from_qcall_type_handle` 落到 `RtTypeSig` / `RtClass`。
- `RuntimeMethodHandleInternal` 经 `get_method_info_from_handle_arg` 解析 reflection method/constructor object、runtime method info stub、direct metadata pointer 或栈槽。
- `RuntimeFieldHandleInternal` 经 `get_field_info_from_handle_arg` 解析 reflection field object、boxed runtime field handle、direct metadata pointer、栈槽或 runtime field info stub。
- `QCallModule` / `QCallAssembly` 经 `get_module_from_qcall_module` / `get_assembly_from_qcall_assembly` 映射到 LeanCLR module / assembly registry。

验收入口：

- `ManagedNet10.LegacyTests.Program::RunLegacyDiscoverySmoke`。
- `ManagedNet10.LegacyTests.Program::RunCorlibReflectionRuntimeModule`。
- `ManagedNet10.LegacyTests.Program::RunRuntimeDelegateDynamicInvoke`。
- `ManagedNet10.LegacyTests.Program::RunCorlibValueTypeEqualsStructValueTypes`。
- `ManagedNet10.LegacyTests.Program::RunCorlibValueTypeGetHashCodeStructIsStable`。
- `ManagedNet10.LegacyTests.Program::RunCorlibRuntimeHelpers`。
- `ManagedNet10.LegacyTests.Program::RunNet10SpanBinaryPrimitives`。
- `scripts/dotnet10/api-scan.ps1` 与 `scripts/dotnet10/nkg-smoke.ps1`。

### 4. RuntimeAssembly / RuntimeModule

外部形状：

- `RuntimeAssembly.GetFullName`、manifest module、module list、module type 枚举稳定。
- `RuntimeModule.GetTypes` / token resolve 只覆盖 minimal profile 需要的查询。

内部映射：

- assembly façade 映射到 LeanCLR 加载的 assembly / module 集合。
- module façade 映射到 `RtModuleDef`。
- token resolve 必须走 metadata table，不做字符串猜测。

验收入口：

- `Assembly.FullName`。
- `Assembly.GetTypes()`。
- `RuntimeModule.InternalGetTypes`。
- `RuntimeModule.ResolveTypeToken` / `ResolveMethodToken` 的白名单子集。

### 4.1 AssemblyLoadContext / Reflection.Emit 受限 façade

外部形状：

- `AssemblyLoadContext` 的初始化入口返回稳定的非空 runtime handle，让 CoreLib 的上下文登记和默认上下文 bookkeeping 可以继续执行。
- `AssemblyLoadContext.GetLoadedAssemblies` 返回 LeanCLR 当前已加载 assembly façade 数组，支撑 `AppDomain.GetAssemblies()` 和反射枚举。
- `PrepareForAssemblyLoadContextRelease` 可被 CoreLib 调用，但第一阶段只做 release bookkeeping，不承诺 collectible ALC 或 unload。
- `RuntimeAssemblyBuilder.CreateDynamicAssembly` 可创建 LeanCLR 可持有的动态 assembly façade，支撑 expression / light lambda 生成过程中的 metadata 对象。
- `ModuleHandle.GetDynamicMethod` 可把受限 dynamic resolver 解析为 runtime method info stub，供解释 profile 的 lambda 调用路径继续执行。

内部映射：

- ALC handle 是 LeanCLR 自有稳定 token，不暴露 CoreCLR LoaderAllocator / AssemblyLoadContextNative 结构。
- 已加载程序集枚举复用 LeanCLR appdomain / module registry，不从 CoreCLR loader graph 推导。
- 动态 assembly 只映射到 metadata façade 和可解释方法入口；不构建完整 Reflection.Emit module builder、ILGenerator、native code buffer 或 collectible allocator。
- `GetDynamicMethod` 优先从 resolver 托管对象读取可识别 method 信息，失败时输出具体 unsupported diagnostics，而不是宽松吞掉动态 codegen 缺口。

验收入口：

- `ManagedNet10.LegacyTests.Program::RunCorlibLightLambda`。
- `CorlibLightLambdaNet10Semantics` 中的 static call lambda、`in` 参数 lambda 和 reflection invoke 语义。

### 5. CustomAttribute / RuntimeCustomAttributeData

外部形状：

- `GetCustomAttribute<T>()`、`GetCustomAttributes(Type, inherit)` 和 `CustomAttributeData` 的最小路径可运行。
- constructor arguments、named arguments、inherit lookup 有明确支持边界。

内部映射：

- 从 metadata custom attribute blob 解码；不要复用 Mono `MonoCustomAttrs` 入口作为 net10 主路径。
- attribute 实例化走现有 `newobj` / interpreter 调用。
- unsupported blob 形状输出具体 attribute type 和 owner。

验收入口：

- `ManagedNet10.Smoke` 中的自定义 attribute 查询。
- `ManagedNet10.Smoke.Program::TestCustomAttributeDataOnly` 中 assembly / module / type / field / method / parameter / property / event target 的 metadata-only constructor arguments、property named argument 和 field named argument。
- `ManagedNet10.Smoke.Program::TestCustomAttributeDataOnly` 中的 Odin/NKG 常见 blob 形状：enum、Type、int[]、Type[]、object、object[]、named enum/type/array/object。
- NKG / Odin 会触发的轻量 attribute 枚举，当前由 `scripts/dotnet10/nkg-smoke.ps1` 验收。

### 6. RuntimeHelpers / Unsafe / Span

外部形状：

- `RuntimeHelpers` 的类型初始化、array data、object identity、generic helper 有明确映射。
- `Unsafe` / `Span<T>` 的 byref、stackalloc、RVA initializer 和 ref reinterpret 路径可解释执行。
- `SufficientExecutionStack` / `TryEnsureSufficientExecutionStack` 在解释 profile 下提供可前进的栈检查结果。
- `RuntimeHelpers.CompileMethod(RuntimeMethodHandleInternal)` / `PrepareMethod(RuntimeMethodHandleInternal, IntPtr*, int)` 可被 CoreLib / reflection 路径调用；解释 profile 中它只确认入口可接受，不触发 native code generation。

内部映射：

- `Span<T>` 不创建 CoreCLR 内部对象模型；它按 .NET 10 layout 在解释器栈和托管对象数据区中表达。
- `Unsafe` 只支持已有解释器和内存模型能保证的路径。
- `RuntimeHelpers.InitializeArray` / `CreateSpan` 从 `RtFieldInfo` 的 RVA blob 读取数据，使用同一 field handle decode 规则。
- `RuntimeHelpers.RunClassConstructor` / `RunModuleConstructor` 进入 LeanCLR class / module cctor 路径，不复制 CoreCLR loader lock 模型。
- inline array helper 映射到托管对象或栈上缓冲区的元素地址，不允许 byref-like 结果逃逸到 heap。
- byref-like 类型不得逃逸到 boxed object 或 heap field。

验收入口：

- Span stackalloc。
- RVA initializer。
- `Unsafe.As` / `Unsafe.Add` 的白名单子集。
- inline array first element / indexed element helper。
- `RuntimeHelpers.RunClassConstructor` / `RunModuleConstructor`。
- `RuntimeHelpers.SufficientExecutionStack` / `TryEnsureSufficientExecutionStack`。
- `RuntimeHelpers.CompileMethod` / `PrepareMethod` no-op façade。

### 7. Exception / Delegate

外部形状：

- .NET 10 CoreLib 触发的异常构造、throw/catch、stack trace 最小查询可运行。
- delegate 创建、闭包 target、static method、instance method、multicast invoke 最小路径可运行。

内部映射：

- 复用 LeanCLR 现有异常和委托实现。
- 需要补齐的是 CoreLib 对 exception/delegate façade 的字段和 helper 调用期待。

验收入口：

- throw/catch/finally。
- delegate invoke。
- simple event / callback。

### 8. Single-thread Sync / Monitor / Task

外部形状：

- 单线程 profile 下，`Thread.CurrentThread`、`Environment.CurrentManagedThreadId`、`Monitor.Enter/Exit/TryEnter/IsEntered/Wait/Pulse` 和基础 `Task` continuation 有最小兼容语义。
- `ManualResetEvent` / `WaitHandle.WaitOne` / `WaitHandle.WaitAny` 只支持 runtime 自建事件 handle 和受控 timeout。
- `ThreadPool`、Timer、后台 worker、跨线程取消和真实 OS thread affinity 不进入支持计划；触发时应给出明确 `NotSupported` 或白名单诊断。
- `Task` continuation 只映射到 LeanCLR frame pump 或宿主主线程 dispatcher，不扩展为并行 scheduler。

内部映射：

- 第一阶段以单线程 frame scheduler / host dispatcher 为边界。
- `Monitor.Wait` / CoreLib generated PInvoke path 映射到 `vm::Monitor::monitor_wait`，只保证当前单线程 runtime 可控场景。
- `WaitHandle` generated PInvoke path 映射到 runtime 自建 `platform::EventHandle`，不接任意宿主 OS handle。
- async continuation 优先映射到 LeanCLR frame pump 或宿主主线程 dispatcher。

验收入口：

- `lock` / `Monitor.Enter` / `Monitor.Exit`。
- `Monitor.TryEnter` / `Monitor.IsEntered` / `Monitor.Wait` / `Pulse`。
- `ManualResetEvent.WaitOne(0)` 和 `WaitHandle.WaitAny(..., 0)`。
- `Task.Yield()` 的可控 continuation。
- `System.Threading.Monitor::<Wait>g____PInvoke|24_0`、`System.Threading.WaitHandle::<WaitOneCore>g____PInvoke|0_0`、`System.Threading.WaitHandle::<WaitMultipleIgnoringSyncContext>g____PInvoke|2_0`。

### 9. Host Bridge

外部形状：

- 托管代码通过稳定 ABI 调用 Unity/Godot 宿主服务。
- 引擎对象通过 opaque handle 暴露，不让托管对象持有真实引擎指针。
- 托管 wrapper 可表现为普通 C# 对象、属性、事件和 delegate，但跨边界只传递基础值、字符串、托管对象引用、opaque handle 和明确 layout 的小 struct。
- 引擎回调托管代码必须重新进入 LeanCLR 调用边界，由 runtime 负责异常捕获、返回值 marshal 和 GC root 生命周期。

内部映射：

- 第一阶段主路径采用 host function table：宿主在 runtime 初始化时注册版本号、能力 flags 和函数表；PInvoke / internal call 只作为 façade，不直接绑定 Unity/Godot C++ 对象布局。
- handle registry 由宿主侧持有真实引擎对象，LeanCLR 只保存不透明整数 / 指针形 handle；访问时必须校验 handle 是否仍有效。
- 主线程 dispatcher 由宿主侧提供，LeanCLR 把需要引擎主线程的调用、托管回调和 `Task` continuation 投递到同一队列。
- 错误转换采用显式状态码 + 可选错误字符串；host bridge 返回失败时，LeanCLR 转成托管异常，不把宿主异常对象跨 ABI 泄露。
- 生命周期释放分为托管 wrapper 释放、GCHandle / delegate 订阅释放和宿主对象销毁三类；任意一侧销毁后 registry 必须让 handle 失效。

ABI 分组：

| 组 | 最小 contract | 第一阶段边界 |
| --- | --- | --- |
| Runtime | 初始化、关闭、设置搜索路径、加载 assembly、调用静态入口 | 不承诺完整 CoreCLR hosting API |
| Object / Handle | 创建、查询、判活、retain/release、销毁通知 | handle 不暴露真实引擎指针和布局 |
| Logging / Diagnostics | log、warning、error、托管异常栈回传 | 不要求宿主实现完整 debugger |
| Scheduler | 主线程投递、下一帧执行、同步调用保护 | 阻塞 wait 和后台线程调度默认不支持 |
| Event / Callback | 注册托管 delegate、取消注册、触发回调 | 订阅必须有可释放 token 或 handle |
| Value Marshal | bool/int/float/string、opaque handle、小 struct、数组 view | 复杂 Variant / UnityEngine.Object 语义后置 |

P2 mock host 节点拆解：

| 节点 | 最小产物 | 必测场景 | 非目标 |
| --- | --- | --- | --- |
| P2.1 ABI skeleton | 一个 C ABI header、mock host 初始化流程、runtime 侧注册入口 | 版本 / capability 校验、初始化失败错误字符串、托管静态入口调用成功；`host-bridge-smoke.ps1 -Scenario AbiSkeleton` 已通过 | 不复制 CoreCLR hosting API；不加载真实 Unity/Godot |
| P2.2 Opaque handle registry | 宿主侧 handle table、retain/release、destroy notification、托管 wrapper handle 字段 | 创建后查询、retain/release 平衡、宿主销毁后访问返回 ObjectDisposed / MissingReference 风格诊断；`host-bridge-smoke.ps1 -Scenario HandleRegistry` 已通过 | 不暴露真实引擎指针；不承诺跨进程 handle |
| P2.3 Main-thread dispatcher | mock 主线程队列、next-frame pump、同步 reentry guard、结果回传结构 | FIFO 投递、下一帧执行、托管异常转错误状态、阻塞 wait 明确拒绝；`host-bridge-smoke.ps1 -Scenario Dispatcher` 已通过 | 不实现后台 worker 调度；不允许从非宿主主循环直接访问引擎对象 |
| P2.4 Event / callback bridge | delegate subscription token、触发入口、取消注册入口、异常返回字段 | 注册后触发托管 delegate、托管异常回传、取消后不再触发、重复取消幂等；`host-bridge-smoke.ps1 -Scenario EventCallback` 已通过 | 不实现 UnityEvent / Godot Signal 完整语义；复杂 Variant marshal 后置 |

P2 验收命令使用独立脚本，避免混入 BCL smoke：

- `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario AbiSkeleton`（已通过）。
- `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario HandleRegistry`（已通过）。
- `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario Dispatcher`（已通过）。
- `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario EventCallback`（已通过）。

P3 托管 wrapper 验收命令：

- `scripts/dotnet10/interp-smoke.ps1 -Configuration Release -Entry "ManagedNet10.Smoke.HostBridgeWrapperSmoke::Run"`（已通过）。
- `scripts/dotnet10/interp-smoke.ps1 -Configuration Release -Entry "ManagedNet10.Smoke.EngineBindingSmoke::Run"`（已通过）。
- `scripts/dotnet10/interp-smoke.ps1 -Configuration Release -Entry "ManagedNet10.Smoke.ValueMarshalSmoke::Run"`（已通过）。
- `scripts/dotnet10/interp-smoke.ps1 -Configuration Release -Entry "ManagedNet10.Smoke.DiagnosticsSmoke::Run"`（已通过）。
- `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario EngineAdapter`（已通过）。
- `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario ValueMarshal`（已通过）。
- `scripts/dotnet10/host-bridge-smoke.ps1 -Configuration Release -Scenario Diagnostics`（已通过）。
- `scripts/dotnet10/api-scan.ps1 -Configuration Release`（已通过，unsupported `0`）。

验收入口：

- mock host 调用托管入口。
- opaque handle 创建、查询、释放。
- 主线程投递和结果回传。
- 托管 delegate 注册为宿主事件回调，触发后返回成功或托管异常诊断。
- 宿主对象销毁后再次访问 wrapper，稳定输出 ObjectDisposed / MissingReference 风格诊断。
- 托管 `HostObject` / `HostEventSubscription` wrapper 的释放、重复释放、事件订阅取消和 host status 到托管异常的转换。
- 托管 `EngineNode` binding 只通过 `HostObject`、dispatcher 和 subscription token 操作 mock engine adapter。
- bool/int/float/string/vector3 的 value marshal、属性 get/set、command 返回值和错误类型诊断。
- info/warning/error 日志级别、托管异常诊断回传和 host failure 到托管异常的分级转换。

### 10. System.IO / Platform PInvoke

外部形状：

- .NET 10 `System.IO.FileStream`、`File`、`Path` 和 `SafeFileHandle` 触发的 generated PInvoke 可在 Windows 与非 Windows profile 下链接并执行。
- `CreateFile` / `ReadFile` / `WriteFile` / `CloseHandle` / `SetFilePointer` / file attribute / file information / temp path / full path 查询有最小语义。
- `DeleteFile`、`CopyFile`、`CreateDirectory`、`MoveFile`、`RemoveDirectory`、`ReplaceFile`、`SetFileAttributes` 和 `SetFileInformationByHandle` 在可表达的平台能力内工作。
- 失败时统一写入 Win32-style last error，让 CoreLib 能走自己的异常转换路径。

内部映射：

- CoreLib generated `Kernel32` PInvoke 先进入 `platform::Kernel32` façade，再映射到 `os::File`、`os::Path`、`os::Sys` 或 POSIX fallback。
- 非 Windows 不暴露真实 Win32 handle 语义；file handle 只表示 LeanCLR / OS file descriptor 可管理的句柄。
- `GetFullPathNameW` 动态读取当前工作目录后拼接相对路径，避免固定缓冲或宿主沙盒路径假设。
- `SafeFileHandle` 只负责 runtime 文件句柄生命周期，不承诺任意 native handle、overlapped I/O 或平台 ACL。

验收入口：

- `Path.GetTempPath()`、`Path.GetFullPath()`。
- `File.Exists()`、`File.Delete()`。
- `File.OpenHandle()` 与默认 / Activator 创建的 `SafeFileHandle`。
- `FileStream` create / write / flush / read round trip。
- `ManagedNet10.LegacyTests.Program::RunCorlibIO`。

## 初始 Contract Inventory

| Contract | 来源/触发 | LeanCLR 映射 | 当前处理 |
| --- | --- | --- | --- |
| `System.Object.GetType` | 基础 CoreLib / smoke | boxed object -> `RtClass` -> `RuntimeType` façade | 已有路径，需纳入正式 façade |
| `System.Type::op_Inequality` / `bool` return slot | reflection discovery | interpreter return slot -> `0/1` full-width stack value | 已修复，`RunLegacyDiscoverySmoke` 通过 |
| `System.RuntimeTypeHandle::GetBaseType` | reflection / type query | `RtClass::base_type` 或等价查询 | 已桥接，`RunLegacyDiscoverySmoke` 通过 |
| `System.RuntimeTypeHandle::is_subclass_of` | reflection / type query | `vm::Class` assignability | 已桥接，`RunLegacyDiscoverySmoke` 通过 |
| `System.Reflection.RuntimeAssembly::GetFullName` | legacy `RunAll` blocker / .NET 10 QCall | assembly metadata -> managed string via `StringHandleOnStack` | QCall façade 已实现，smoke 通过 |
| `System.Reflection.RuntimeModule::InternalGetTypes` | reflection smoke | `RtModuleDef` type table -> `RuntimeType[]` | 已桥接，`RunCorlibReflectionRuntimeModule` 通过 |
| `System.Reflection.CustomAttributeData` minimal path | attribute smoke / Odin | metadata blob decoder -> attribute data façade | 已覆盖核心 target 与常见 blob-shape metadata-only smoke；NKG/Odin 轻量 workload gate 通过 |
| `System.RuntimeFieldHandle::GetApproxDeclaringMethodTable` | `RuntimeModule.ResolveField` / field reflection | `RtFieldInfo` -> declaring `RtClass` -> net10 MethodTable façade | 已修复，`RunCorlibReflectionRuntimeModule` 通过 |
| `System.ValueType::<CanCompareBitsOrUseFastGetHashCodeHelper>g____PInvoke|2_0` | `ValueType.Equals` / `GetHashCode` | `MethodTable*` façade -> `RtClass` | 已修复，ValueType 子入口通过 |
| `System.Delegate::GetInvokeMethod` / `GetMulticastInvoke` | delegate reflection / multicast | `MethodTable*` façade -> delegate `RtClass` -> invoke method | 已修复，`RunRuntimeDelegateDynamicInvoke` 通过 |
| `System.RuntimeTypeHandle::InternalAllocNoChecks_FastPath(System.Runtime.CompilerServices.MethodTable*)` | `MulticastDelegate.NewMulticastDelegate` | `MethodTable*` façade -> `RtClass` -> object allocation | 已修复，`RunRuntimeDelegateDynamicInvoke` 通过 |
| `System.RuntimeFieldHandle::<GetRVAFieldInfo>g____PInvoke|24_0` | Span/RVA initializer | `RtFieldInfo` RVA data | 已桥接，`RunNet10SpanBinaryPrimitives` 通过 |
| `System.Runtime.CompilerServices.RuntimeHelpers::CompileMethod` / `PrepareMethod` | reflection / dynamic method preparation | accepted no-op preparation façade in interpreter profile | 已桥接，`TestRuntimeHelpers` 通过 |
| `RuntimeHelpers.InitializeArray` / `CreateSpan` / inline array helpers | Span/RVA / inline array lowering | `RtFieldInfo` RVA blob + interpreter stack/object buffer view | 已桥接，`TestSpan` 与 `RunNet10SpanBinaryPrimitives` 通过 |
| `System.Runtime.Loader.AssemblyLoadContext::GetLoadedAssemblies` / `InitializeAssemblyLoadContext` / `PrepareForAssemblyLoadContextRelease` | CoreLib ALC bookkeeping / `AppDomain.GetAssemblies` | stable LeanCLR ALC token + loaded assembly façade registry | 已桥接，`RunCorlibLightLambda` 通过 |
| `System.Reflection.Emit.RuntimeAssemblyBuilder::CreateDynamicAssembly` | Reflection.Emit / expression light lambda setup | metadata-only dynamic assembly façade | 已桥接，`RunCorlibLightLambda` 通过 |
| `System.ModuleHandle::<GetDynamicMethod>g____PInvoke|9_0` | dynamic resolver / DynamicMethod handle lookup | restricted resolver -> runtime method info stub | 已桥接，`RunCorlibLightLambda` 通过 |
| `System.Threading.Monitor::<Wait>g____PInvoke|24_0` | Monitor wait / pulse | `vm::Monitor::monitor_wait` -> single-thread controlled wait | 已桥接，`RunCorlibMonitor` 通过 |
| `System.Threading.WaitHandle::<WaitOneCore>g____PInvoke|0_0` / `WaitMultipleIgnoringSyncContext` | ManualResetEvent / WaitAny | runtime `EventHandle` -> `Kernel32` façade wait helpers | 已桥接，`RunCorlibThreading` 通过 |
| `System.IO.FileStream` / generated `Kernel32` file PInvoke | File / Path / SafeFileHandle | `platform::Kernel32` façade -> `os::File` / `os::Path` / POSIX fallback | 已桥接，`RunCorlibIO` 通过 |

## 迁移顺序

1. 建立 `net10_runtime_contract` 代码分区或等价边界，禁止 `coreclr-net10` 隐式落回 Mono-era 名称/签名。
2. 先完成 `RuntimeType` / `MethodTable*` façade 边界，确保所有 CoreLib 传入的 `System.Runtime.CompilerServices.MethodTable*` 都先解析为 net10 façade，再映射到 LeanCLR `RtClass`。
3. 保持 delegate allocation / multicast 路径绿色：`RunRuntimeDelegateDynamicInvoke` 作为 `MethodTable*` façade 回归 gate。
4. 重写并收敛 type / method / field / module handle decode helper，把 direct pointer、stack slot、boxed handle、stub object 和 managed reflection object 都纳入同一边界校验。当前解释 profile 已归档为 P1.7 handle/reflection consolidation。
5. 重写 assembly / module façade，优先解锁 `RuntimeAssembly.GetFullName`、`Assembly.GetTypes()`、`RuntimeModule.GetTypes` 和 token resolve 白名单。
6. 重写 custom attribute 最小路径，覆盖 smoke 与 NKG/Odin 会触发的读取模式。当前 metadata-only smoke 与 NKG/Odin 轻量 workload 已通过，后续只按真实失败补齐新 blob 形状或实例化路径。
7. 固化 Span / Unsafe / RuntimeHelpers contract，确保解释路径和 AOT 路径使用同一份语义说明。当前解释 profile 已覆盖 RuntimeHelpers cctor、栈检查、CompileMethod / PrepareMethod no-op façade、RVA `InitializeArray` / `CreateSpan` 与 inline array helper。
8. 固化 AssemblyLoadContext / Reflection.Emit 受限 façade，支撑 light lambda / dynamic assembly metadata 路径，同时明确 collectible ALC、unload 与完整动态 codegen 后置。
9. 固化 exception / delegate / single-thread sync / Monitor / Task continuation 的受控 façade；后台线程和 worker 调度不进入计划。
10. 固化 System.IO / platform PInvoke 最小 contract，让真实 workload 可做临时文件、路径解析和基础资源读取。
11. 建立 host bridge mock，证明 Unity/Godot 接入不需要扩大 BCL 支持面。

## 验收方式

每个领域提交前必须给出三类证据：

- contract evidence：文档中有调用链、签名、内部映射和 unsupported 边界。
- catalog evidence：`coreclr-net10` catalog / PInvoke / intrinsic 注册表只包含本领域需要的入口。
- runtime evidence：至少一个 `ManagedNet10.Smoke` 子入口或真实 workload 入口可以稳定触发并通过，或稳定输出预期 `NotSupported` 诊断。

当前第一道自动化 gate 已落在 `src/generator/check_runtime_api_signatures.py --profile coreclr-net10 --repo-root .`：它会用 .NET 10 runtime pack externs 校验 `coreclr-net10` catalog，并禁止 `Mono.*`、`System.IO.Mono*`、`System.Reflection.Mono*`、`System.Runtime.Remoting*`、`mscorlib` 以及 Mono-era implementation symbol/header 混入 active profile。

API 边界 gate 已落在 `scripts/dotnet10/api-scan.ps1`：它构建 `src/tools/net10apiscan`，用 `minimal-net10-whitelist.json` 扫描 `AssemblyRef`、`TypeRef` 和 `MemberRef`，并在白名单外引用出现时输出 `unsupported_api` 诊断。当前默认扫描 `ManagedNet10.Smoke`、`ManagedNet10.NkgSmoke` 与 NKG core/Odin/UniTask 三件套；2026-06-29 验收结果为 NKG core/Odin/UniTask 扫描 unsupported `0`。完整 sampler/Hosting 仍在第一阶段边界外。

NKG/Odin 真实 workload gate 使用 `scripts/dotnet10/nkg-smoke.ps1`。脚本默认查找仓库同级的 `NKGGameFramework`，构建 `samples/NKGGameFramework.Sampler`，再把带齐 `NKGGameFramework`、`OdinSerializer` 和 `UniTask` 的 `net10.0` 输出目录传给 `ManagedNet10.NkgSmoke`。2026-06-29 已在默认路径通过；如果本机目录不同，可显式传入 `-NkgRoot`。

第一阶段完成审计（2026-06-29）已跑完以下 gate：

- `python src\generator\check_runtime_api_signatures.py --profile coreclr-net10 --repo-root .`：3234 个 extern entry / 4675 个签名载入，`icalls.json`、`intrinsics.json`、`pinvokes.json` 全部匹配。
- `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\api-scan.ps1 -Configuration Release`：`ManagedNet10.Smoke` / `ManagedNet10.NkgSmoke` 扫描 unsupported `0`；NKG core / Odin / UniTask 扫描 unsupported `0`。
- `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\nkg-smoke.ps1 -Configuration Release`：NKG/Odin/UniTask 轻量 workload 通过。
- `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\interp-smoke.ps1 -Configuration Release -AssemblyName ManagedNet10.LegacyTests -Entry "ManagedNet10.LegacyTests.Program::RunAll"`：legacy regression 基线通过。
- `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\legacy-managed-audit.ps1`：374 个旧 managed 文件已链接，唯一未链接 active-ish 文件为原始 `TC_sub.cs`，其 net10 替代版 20 个 `[UnitTest]` 方法与原始文件匹配。
- `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\interp-smoke-matrix.ps1 -Configuration Release`：默认全量 65 个 smoke entry 全部通过。
- `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\aot-smoke.ps1 -Configuration Release -NativeRun`：LeanAOT net10 smoke C++ 生成、native `aot-tester.exe` 构建和运行均通过。
- `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\host-bridge-smoke.ps1 -Configuration Release -Scenario <scenario>`：`AbiSkeleton`、`HandleRegistry`、`Dispatcher`、`EventCallback`、`EngineAdapter`、`ValueMarshal`、`Diagnostics` 全部通过。
- `powershell -ExecutionPolicy Bypass -File scripts\dotnet10\interp-smoke.ps1 -Configuration Release -Entry <entry>`：`ManagedNet10.Smoke.HostBridgeWrapperSmoke::Run`、`EngineBindingSmoke::Run`、`ValueMarshalSmoke::Run`、`DiagnosticsSmoke::Run` 全部通过。

完整 `RunAll` 和原作者测试资产仍是长期质量线，但第一阶段不再把旧失败点作为设计入口。新增失败必须先归类：contract 缺口、façade 映射缺口、LeanCLR VM 通用语义缺口、或白名单外 API。只有前两类进入本计划的 contract/model 重写循环。

下一阶段计划：

1. 保持上述 gate sweep 作为非回归基线，任何 runtime/API/profile 变更先复跑 `src\tests\TESTING.md` 中的完整命令。
2. 把验证重心从测试资产扩到真实 workload：补 NKG/Odin/UniTask 行为级用例，包括 Odin serialize/deserialize roundtrip、UniTask continuation、资源/配置读取和更多 SampleGame 玩法/ECS 切片。
3. 继续把 host bridge 停留在 mock contract 层扩展 value marshal、event/callback 和 diagnostics，真实 Unity/Godot SDK binding 作为后续阶段单独验收。
4. 仍然不把完整 `Microsoft.NETCore.App`、完整 debugger、动态 native codegen、多线程 runtime 和完整 CoreCLR ALC 当作当前阶段目标。
