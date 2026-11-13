# 第03条修复方案：支持 managed / unmanaged calli

状态：待审阅，尚未实施。适用于 mono-minimal、mono-full、coreclr-minimal 的共享执行层。

建议采用**签名驱动的静态 C++ ABI 桥接**：managed calli 调运行时 invoker；unmanaged calli 调按完整签名生成的原生桥。解释器和 AOT 共用签名与桥接注册协议，不要求 JIT，不把 libffi 设为 minimal 的必需依赖。

## 当前代码核实

- `src/runtime/interp/transformer.cpp` 的 `add_calli` 已明确拒绝非 Default 调用约定；不是把所有 unmanaged 地址继续当方法描述符执行。
- `interpreter_handlers.inc` 的 `CalliInterp` 仍把目标解释为 `RtMethodInfo*`，只比较参数数量。需要补齐目标来源、完整签名及泛型上下文验证。
- `ldftn/ldvirtftn` 产生运行时方法描述符；它们不是可直接传给原生库的机器码地址。
- `LeanAOT.GenerationPlan/Manifest.cs`、`Manifest.Generics.cs` 遇到 calli 会回退整个方法，原因统一写为 `managed-calli`，未区分两种调用。
- `SharedTests/Instructions/Funcs/TC_calli.cs` 目前只有 TODO，不能作为已有支持的验收证据。

因此，第03条应从“维持拒绝”升级为一项独立的调用协议实现。移除 NotSupported 检查不足以完成支持。

## 调用协议

### 1. 保留两种目标表示，拆分执行指令

引入 managed / unmanaged 两种 calli IR。转换时依据 StandAloneSig、调用约定及 custom modifiers 决定分派，不能依据地址范围或指针低位猜测目标类型。

managed 路径继续使用方法描述符与现有 invoker，保留实际实例、实例化泛型上下文、AOT/解释器目标选择。对可逃逸的 ldftn/ldvirtftn 描述符建立按需注册的有效目标索引，在解引用前检查；不为所有方法额外建一张常驻表。签名校验包含返回类型、参数类型、byref、HasThis/ExplicitThis、调用约定和实例化上下文，不能只校验参数数目。

unmanaged 路径将地址作为真实原生函数指针，调用该签名对应的编译器生成桥。绝不转换为 RtMethodInfo 指针。非空任意原生地址的有效性仍由 unsafe 调用方保证，运行时不通过探测内存页来伪造地址验证。

### 2. 由 C++ 编译器实现平台 ABI

按签名生成明确类型的函数指针 typedef 和调用包装：

```cpp
using NativeFunction = NativeReturn (CALL_CONV *)(NativeArg0, NativeArg1);
// 包装层从运行时参数槽读取精确类型，调用 target，再写回返回槽。
```

复用 LeanAOT 现有 P/Invoke 的调用约定宏、原生类型布局和编译工具链，但不执行具名 DLL 查找；target 来自 IL 求值得到的函数指针。普通原生返回 ABI 与 LeanCLR 的 Result/轻量错误协议是两层协议，不能直接混用。

不得用“按参数个数统一 cast”的调用器。float/double、混合整数浮点、结构体返回、隐藏返回地址和 x86 调用约定，都交给对应目标的 C++ 编译器处理。

Wasm 桥必须使用与目标完全匹配的函数类型及模块/宿主函数表入口，不能把 JavaScript 对象、其他模块的裸表索引或托管描述符当函数地址。

### 3. 解释执行也能使用预生成桥

签名采集独立于“是否为方法生成 AOT 函数体”。收集输入程序集和泛型闭包中的 calli 签名，生成去重的全局 ABI 桥表。即使某程序集只解释执行，也可以在构建宿主时生成其 calli 桥。

允许宿主显式注册同协议的签名桥，以覆盖后加载程序集。缺少桥时在方法转换/绑定阶段给出可诊断的 NotSupported，并列出缺失签名；不需要等到执行原生地址后才发现 ABI 不支持。

签名 key 包含调用约定、返回/参数 ABI 类型、结构体布局和影响调用的 modifiers。哈希必须做完整 key 冲突比较。不得只使用程序集内 token 或参数数量。

查找发生在方法转换/首次绑定时，IR 缓存桥入口；正常重复调用不重新解析签名、不遍历程序集、不分配内存。AOT 已知签名可直接引用生成桥。

### 4. 类型、GC 与异常边界

首批支持：固定参数、静态 unmanaged 调用；void、整数、指针大小整数、float/double、原生指针，以及这些非托管值的 ref/out。bool/char 的位宽和返回扩展必须按 calli ABI 对照测试，不能套用 Marshal 的 BOOL=4 字节规则。

第二批支持：通过目标 ABI 验证的无托管引用结构体，含值传递、ref/out、不同大小的返回值。验证实际大小、对齐、pack、字段布局；不会把 managed struct 简单 memcpy 成任意 native struct。含引用字段、AutoLayout、需要字符串/对象自动封送的签名不在首批开放范围。

调用前后沿用原生调用的 GC 根与帧协议；托管内存地址必须具有足够存活期，并满足固定地址要求。测试原生调用回入托管并触发 GC 的场景，检查根和帧释放。UnmanagedCallersOnly、通用反向 P/Invoke 与动态原生闭包另立任务，不以 outbound calli 完成代表它们已支持。

继续遵守现有 AOT 异常规则：正常路径 AOT，托管异常后由解释器恢复；正常路径副作用不能重放。C++ 异常不得直接穿过 C ABI 边界，不能宣称 catch 一个 C++ 异常就实现了 CLR 异常语义。

varargs、未实现的 ThisCall/FastCall 目标 ABI、SuppressGCTransition 等 modifiers 必须有明确能力检查；首轮不悄悄忽略它们。Windows x64 上多个调用约定可能等价，不意味着 x86 上也等价。

## 建议提交顺序与验收

| 步骤 | 开发内容 | 验收要求 |
|---|---|---|
| 03-A | 完整签名建模、调用约定/modifier 解析、目标校验；区分 fallback 原因 | 同参数数目的不同类型签名不混用；非法目标不先解引用；unsupported 路径明确 |
| 03-B | 完善 managed calli 解释器路径及 AOT 发射 | static/instance/virtual、闭合泛型、ref/out、返回值、AOT↔解释器组合通过；无泛型上下文丢失 |
| 03-C | 独立签名采集、静态 ABI 桥生成、全局去重注册与绑定缓存 | 仅解释执行的程序集可生成桥；跨程序集相同签名复用；缺失桥可定位；稳态零额外分配 |
| 03-D | 开放固定参数 unmanaged 标量/指针调用，Native 与 Wasm | 整数/浮点混合、超过寄存器参数数量、指针/ref/out、bool/char 边界通过原生函数实测 |
| 03-E | 开放已验证平台的 blittable 结构体参数与返回 | 小/大结构体、隐藏返回、pack/显式布局、不同编译器 ABI 测试通过后逐平台启用 |
| 03-F | GC/EH 压力、性能/体积对照、平台能力表与文档 | 与具名 P/Invoke 对比；记录冷/热调用、二进制大小、桥数量；ASan、Wasm 类型检查、失败路径通过 |

优先本机 Windows x64 与 Wasm32，再通过 CI 验收 Linux x64、ARM64，x86 按实际宿主需求启用。未实测的平台保持单独的能力状态，不把交叉编译成功等同于 ABI 验收通过。不使用此前被限制的 Linux 机器。

对照测试需要一个真正的 C/C++ 原生测试库，函数指针来自导出地址/宿主注册；不能只用托管描述符伪装原生指针。CoreCLR 运行相同签名做语义对照，LeanCLR 同时跑解释器和实际 AOT 产物。补齐 TC_calli，额外增加原生契约与生成器签名去重测试。

性能门槛：稳态不做签名解析/哈希查找、无逐次桥缓存扩容或原生临时分配；解释器执行为一次缓存桥入口分派和一次类型正确的间接调用。先给出实测基线，再决定时间开销阈值，不预先承诺不存在的数据。

## 本轮待审阅决定

建议批准以上 03-A 至 03-F，以“静态签名桥”为 minimal 的正式实现路径。libffi 仅作为今后 Full/桌面版处理构建期未知签名的可选后端评估，不引入 minimal 的必需依赖。本轮只提交此方案，不开放尚未实现的 calli 能力。
