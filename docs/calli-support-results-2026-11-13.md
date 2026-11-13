# 第03条：calli 实施与验收报告（2026-11-13）

03-A 至 03-F 已实现，覆盖 mono-minimal、mono-full、coreclr-minimal。Windows x64 与 Wasm32 为本轮实测平台；Linux x64/ARM64 已配置 CI ABI 探针，尚未运行，生产能力仍关闭。没有使用此前受限的 Linux 机器。

## 实现与源码入口

- `src/runtime/vm/calli_targets.cpp`：只为逃逸的托管目标维护可失败扩容的索引；检查目标来源后才解引用。原生地址与托管方法描述符分开使用。原生调用守卫独立成编译单元，避免只使用 ABI 桥时链接整个托管调用层。
- `src/runtime/vm/calli.cpp`：完整托管签名校验、引用返回协变/参数逆变、实例/装箱接收者、实际泛型调用上下文及 AOT 调用点缓存。解释器也使用单目标校验缓存；目标变化才重新检查。空 this 的合法 calli 通过解释器处理，避免进入假定 this 非空的普通 AOT 函数体。
- `src/runtime/vm/calli_native.cpp`：按约定、完整类型、大小、对齐、字段偏移及嵌套布局建索引；哈希命中后完整比较。转换/首次调用绑定，热调用不解析签名、不查找桥表、不分配临时原生内存。
- `src/leanaot/LeanAOT.ToCpp/CalliBridgesCppCodeFile.cs`：扫描输入 IL 和闭合泛型调用，包括被 AOT 属性排除的函数，生成全局去重的 C++ 类型桥和 JSON 清单。`--calli-bridges-only` 可只为解释执行生成桥。输入/已规划的泛型闭包限制为深度 32、2048 个实例，超预算明确报错；动态生成的新签名需要宿主补充静态桥。
- `src/runtime/interp/transformer_emit.cpp`、`interpreter_handlers.inc`：分开的 CalliInterp / CalliNative IR；原生路径缓存已绑定桥。没有把原生函数地址转换为方法描述符。
- `src/runtime/codegen/leanclr_common.*`、`gc/`、Mono 的 `interp/machine_state.*` / `vm/runtime.cpp`：AOT 和解释器根保护。GC 用例在收集并重新分配后验证对象字段，覆盖原生回入托管、内部指针、求值栈对象和帧释放。Mono 使用有边界的保守帧扫描，经堆分配索引确认对象归属。

调用仍遵守“正常路径 AOT，异常后解释器接管”，测试检查托管及原生副作用均不重放。C++ 异常不能穿过 C ABI；通用反向 P/Invoke、UnmanagedCallersOnly 和动态闭包不属于本轮能力。

## ABI 能力

| 能力 | Windows x64 | Wasm32 | Linux x64 / ARM64 |
|---|---|---|---|
| 托管 static/instance/virtual、闭合泛型、ref/out、返回值 | 解释器与实际 AOT 通过 | 解释器与实际 AOT 通过 | 待 CI |
| unmanaged Cdecl/Stdcall 固定标量、指针、可直接传递的 ref/out | 通过；两种约定在此 ABI 等价 | 通过函数表类型检查 | 默认关闭，CI 显式探针 |
| 无引用且布局匹配的结构体参数/返回/byref | 通过 | 通过 | 待 CI |
| bool | 32 位 BOOL，输入/返回规范化 | 同一真实 C 库验证 | 待 CI |
| char | 默认 ANSI 字节转换；选择正确的 Windows UTF-8 ACP 解码标志 | 明确拒绝；显式 16 位原生值使用 ushort | 默认关闭 |

结构体覆盖小/大返回、浮点与双精度字段、嵌套 byref、Pack=1、显式 padding、重叠字段。AutoLayout、引用字段、MarshalAs 转换、bool/char 字段、特殊 SIMD/inline-array、未验证对齐、bool/char byref、嵌套函数指针 ABI、varargs、ThisCall/FastCall 及未实现 modifiers 均保留明确拒绝。结构布局递归限制为 16，单次绑定嵌套字段预算 128。缺桥诊断含 ABI 布局；AOT 还输出调用方和 IL 位置。

原生非空任意地址仍由 unsafe 调用方保证有效性；运行时只拒绝空地址和已知托管描述符。托管内存必须保持存活及地址稳定。本轮缓存与注册遵守现有 SINGLE 线程模型，并未引入多线程能力。

## 验证

- 每组 16 个 CTest 入口，native 入口内部还覆盖实例/ExplicitThis/虚方法/值类型/装箱/空 this、全部原生 ABI、扩容 OOM/重试、缓存重复注册和 GC/EH 子用例。
- 三个分支：Native 解释器及真实 AOT 通过；AOT harness 断言入口确实采用 AOT invoker。CoreCLR 额外通过 MSVC ASan 与 Wasm32。`TC_calli` 已替换 TODO，真实共享测试源也由本套 fixture 编译并执行。
- 同一 C 库在 .NET 10 运行 Unmanaged、NativeStructs、NativeGeneric 等正例；6 个 dnlib 实例 IL 正例在 .NET 10 对照通过。非法描述符/错误签名用例只在 LeanCLR 执行。
- 同时开启轻量 void/reference、packed int32、wide/struct status-out 和引用泛型共享的 AOT 组合通过；共享报告实际包含 2 条引用实例绑定。
- 原有集成回归：77 个 managed smoke、15 个真实堆 GC 及 3 个 native contracts 通过。
- 10,000 次重复注册/绑定及热基准均检查无额外 GeneralAllocation 分配；失败扩容保持旧绑定有效，之后可成功重试。

可复跑入口：`src/tests/calli/verify-native.ps1 -Benchmark`；仅准备程序集使用 `build-fixtures.ps1`。CI 在 `.github/workflows/calli-contracts.yml`；Linux 探针显式设置 `LEANCLR_CALLI_ABI_PROBE=1`，不改变默认能力。这里记录的是本机结果，不代表远端 CI 已运行。

## 性能与体积

同一个不内联的 C 函数，5 轮、每轮 100,000 次，取热调用中位数。以下为本机微基准样本，具名 P/Invoke 使用静态链接，不能推导为应用整体性能。

| 路径 | calli ns/次 | 具名 P/Invoke ns/次 | 冷单次样本 |
|---|---:|---:|---:|
| Windows x64 解释器 | 约 34–42 | 约 28–29 | 约 8–13 us |
| Windows x64 AOT | 约 3.5–5.9 | 约 0.49 | 约 0.7–1.2 us |
| Wasm32 AOT / Node | 约 7–9 | 约 0.3–0.4 | 约 52–85 us |

动态 calli 仍比可静态优化的具名调用贵，当前收益是得到类型正确、稳态无分配且开销有界的调用路径；没有宣称追平直接调用。

配对二进制由 `LEANCLR_CALLI_SIZE_PROBES=ON` 构建；两者使用同一个 C fixture。native 版本保留整张桥表和调用守卫，named 版本只调用具名函数。这衡量已绑定 ABI 桥的链接增量，不包含托管签名绑定/GC 初始化，也不是单个桥的成本。

| 产物 | named | 全部签名桥 + guard | 增量 |
|---|---:|---:|---:|
| Windows x64 EXE | 11,776 B | 19,456 B | 7,680 B |
| Wasm32 模块（不含 JS） | 261 B | 3,924 B | 3,663 B |

清单共 26 个去重候选桥；Windows 编译 26 个，Wasm 去掉不支持的 ANSI char 后编译 25 个。初始 Windows 探针曾因编译单元依赖拉入运行时而达到约 907 KB，拆分后已消除该意外依赖。添加字段到 RtAotModulesData 后，运行时和生成的 C++ 需要一起重新编译。

## 提交索引

| 阶段 | coreclr-minimal | mono-full | mono-minimal |
|---|---|---|---|
| 03-A 签名与来源 | 22f4335d | 44ce4052 | a0a61b4e |
| 03-B managed AOT / GC | d9bf4886 | fb8c5948 | c9d1b685 |
| 03-C 静态桥采集/注册 | 6933b14d | c823121c | 4755b591 |
| 03-D 标量 native ABI | 32da431f | f63a2999 | 621b53ba |
| 03-E 结构体 ABI | 13f44769 | 43eb5453 | e92fbd71 |
| 03-F 压力/性能/体积与门禁 | 0f5fb882 | 070c8334 | 4e9e4358 |

本报告另行提交；上述提交及报告均使用英文提交日志。
