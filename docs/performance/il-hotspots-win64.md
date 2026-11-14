# Delegate / Cast / NewArray：Win64 热点原因与方案

正式实现和复测已完成，见 [正式优化报告](il-hotspots-optimized-win64.md)。本文件保留优化前的原因分析与诊断原型记录。

日期：2026-11-14。对应 [四后端基准报告](il-families-win64.md)。结论来自实际生成 C++、LeanCLR/Unity 本地运行时源码，以及另外 63 个隔离实验样本；不是仅根据耗时推测。

## 结论

| 用例 | 上轮 LeanAOT / IL2CPP，ns/迭代 | 已确认的问题 | 建议 |
|---|---:|---|---|
| Delegate | 11.408 / 4.003（2.85×） | 委托 Invoke 经通用参数缓冲区、RuntimeImpl invoker、绑定分类和目标 invoker，再还原返回值 | 生成按签名的委托调用 thunk；单播 AOT 快速路径直接走 typed ABI，复杂绑定保留通用回退 |
| Cast | 13.358 / 1.036（12.89×） | 普通类及 sealed 类的检查都进入完整 assignability 算法 | 按目标类型在生成阶段选择 sealed 精确比较、普通类 hierarchy 检查或通用检查 |
| NewArray | 74.896 / 35.552（2.11×） | 每次解析数组类、初始化与布局计算；之后仍有显著分配及堆增长成本 | 缓存数组类/元素尺寸，分离已初始化 SZArray 快速分配入口，再优化 allocator；另补 GC 稳态评测 |

这三项不是同一类问题，也不能归因为“LeanAOT 回退解释器”。Delegate 的目标 Step 是 AOT，Invoke 本身是 native RuntimeImpl；Cast 和 NewArray 也是本机辅助函数路径。

## 隔离实验

复用原基准编译出的 `leanclr.lib` 和同一份托管 DLL，仅在另一个目录中替换诊断版生成代码；没有修改生产运行时、生产生成器和原始基准生成文件。三个实验可执行文件均使用 `/O2 /Ob2 /DNDEBUG`，同一个 host；固定逻辑 CPU 2、随机顺序、每组 7 次。Delegate/Cast 各 2000 万次，NewArray 200 万次。全部 63 个样本均与相同 n 的 .NET 参考 checksum 一致。

下表是本轮对照，不把上一轮数字当作同时测得的控制组。`array-class` 和 `array-layout` 两个构建都包含相同的 Delegate/Cast 快速路径，只在 NewArray 的实现上不同；每个样本是独立进程，三个 kernel 的状态互不混用。

| 实验路径 | Delegate 中位数 | Cast 中位数 | NewArray 中位数 |
|---|---:|---:|---:|
| 同轮未改生成代码 base | 11.082 | 13.590 | 74.916 |
| 快速调用/类型检查 + 缓存数组类 array-class | 3.057 | 2.661 | 68.784 |
| 快速调用/类型检查 + 缓存类与固定布局 array-layout | 3.152 | 2.628 | 55.423 |

单位 ns/迭代。Delegate 的简单 guarded typed 调用已约快 3.5 倍，Cast 的普通类/sealed 类快速检查约快 5.2 倍。NewArray 单独缓存数组类只减少约 6.1ns；继续缓存初始化后的固定长度布局，累计减少约 19.5ns（26%）。这些差值是端到端对照，不是每个 helper 的精确独占 CPU 时间；缓存、内联、代码布局会共同变化。

诊断原型只验证了当前三个 kernel；并未覆盖所有委托绑定、类型系统边界或任意数组尺寸，不能直接当成生产补丁。原始样本见 [results.json](data/il-hotspots-win64/results.json)，源码见 `src/benchmarks/il-families/probes.h`，驱动见 `scripts/benchmarks/probe_il_hotspots.py`。库及源文件散列见 [evidence.json](data/il-hotspots-win64/evidence.json)。

## Delegate：通用调用桥接过重

当前实际调用链：

```text
AOT kernel
  → generated_direct_call_bridge(Func<uint,uint>.Invoke)
  → RtStackObject 参数缓冲区 / 返回缓冲区
  → Runtime::invoke_stackobject_arguments_without_run_cctor
  → Delegate::invoke_delegate_invoker             [RuntimeImpl，本机函数]
  → 单播/多播及开放/封闭绑定分类、实参位置调整
  → target_method->invoke_method_ptr             [目标的 AOT invoker]
  → 还原 typed 参数 → AOT Step
  → 全局临时返回缓冲区 → memcpy → 返回缓冲区 → uint
```

证据位置：`src/runtime/bcl/mono/delegate.cpp:86`、`src/runtime/vm/runtime.cpp:765`。生成器 `MethodWriterBase.CallIntrinsic.cs` 对委托目前专门处理了构造，没有为 Invoke 生成对应 typed intrinsic。生成报告中的 `unsupported-body` 因而是走 RuntimeImpl 的信号，并不表示执行了 IL 解释循环。

Unity 生成的 `Func<uint,uint>.Invoke` 内联实现直接通过 `invoke_impl` 调用，以 `method_code` 和 MethodInfo 携带绑定信息。调用站点无需每次重新走通用栈参数协议。

**方案：**

1. 在生成器增加 Delegate.Invoke 识别及按签名共享的 typed thunk，避免为每个调用点复制完整绑定处理。
2. 首先优化单播、静态、非泛型、已初始化的 AOT 目标；使用与目标 `aot_return_protocol` 相符的 ABI，保留 null、错误返回和异常传播。原型在不取消这些基本 guard 的情况下已降到约 3.1ns。
3. 然后扩展 closed instance、open instance、closed static、值类型 this、泛型 MethodInfo 与各返回协议。不能对任意 delegate 简单把 method_ptr 强转成当前的 `uint(uint)`。
4. 将绑定方式在委托构造/绑定时确定；可在核对 Mono 布局及 clone/combine/remove 契约后使用已有入口字段，优先避免增加每个委托实例的体积。泛型 thunk 需要去重，避免 native 代码膨胀。
5. 多播保持调用顺序、最后一个结果、遇异常停止；解释器/icall/PInvoke 等目标保留兼容回退。简单调用先走快路径，复杂绑定不为微基准牺牲语义。

主要收益来自减少 ABI 往返、间接调用层数和重复分类。这里的参数缓冲区在栈上，临时返回区是静态存储；**不是每次调用额外分配托管对象**。

验收应覆盖上述绑定组合、null、嵌套/重入委托、多播异常、大结构体和带引用的返回值，以及 AOT→解释器目标。

## Cast：缺少普通类与 sealed 类的生成特化

用例每次执行 `(Box)v` 和 `v is OtherBox`。Box 是普通基类，OtherBox 是 sealed 类。

当前 `EmitCastClass/EmitIsInst`（`src/leanaot/LeanAOT.ToCpp/MethodWriterBase.cs:2555`）统一调用 `codegen::cast_class/is_inst`。两个包装最终都进入 `Object::cast_class/is_inst` → `Class::is_assignable_from`。

`src/runtime/vm/class.cpp:2367` 中，非同一类型还会进行递归检查栈扫描、push/pop、双方 super_types 初始化检查、双方 interfaces 初始化检查，最后才进入具体类/接口算法。经过预热后这些检查大多立即成功，但函数调用和分支仍在。递归 Vector 稳定后可复用容量，不应把该开销误称为每次堆分配。

本用例中 `(Box)OtherBox` 必须走非同类路径；`OtherBox` 的 isinst 在遇到 Box 时也走非同类路径。每个迭代都会付出至少一次完整非同类检查。相反，Unity 生成 `CastclassClass` 和 `IsInstSealed`，分别使用父类表和 klass 指针相等判断。

LeanCLR **已经具备** `Class::has_class_parent_fast`（`src/runtime/vm/class.h:461`）：以 hierarchy_depth 做范围判断，再比较 super_types 的对应槽位，时间复杂度 O(1)。问题在于 AOT 普通 cast 没有直接使用它，而不是需要新建一套继承关系缓存。

**方案：**

- 编译时已确定的 sealed 普通引用类：`obj == null || obj->klass == target`；castclass 失败走现有 InvalidCast 错误出口，isinst 失败返回 null。
- 编译时已确定的普通引用类：在建立初始化前提后，内联 hierarchy_depth/super_types 检查。
- interface、数组协变、泛型参数、Nullable/装箱兼容和共享泛型等情况继续使用完整算法。不能把“sealed”一条规则直接套到数组类型上，也不能为优化去掉开放泛型的递归保护。
- 将类型类别的选择尽量放在生成阶段，不在每次循环里再读目标类型类别。此次原型仍保留运行时类别与初始化 guard，2.6ns 不是生产版本的性能下限；进一步接近 IL2CPP 需要实际实现及复测，不能预先承诺。

该方案复用已有元数据，不增加每对象内存；比为每种源/目标组合维护全局类型检查缓存更容易控制占用。验收覆盖 null、成功/失败 cast、深继承、接口与泛型方差、数组协变、值类型/Nullable 和开放泛型约束。

## NewArray：元数据准备 + 分配器，且需要修正 GC 测量口径

当前生成的 `new uint[16]` 传入 **UInt32 元素类**，调用 `Array::__new_szarray_from_ele_klass`（`src/runtime/vm/rt_array.cpp:66`）。每次都会：

1. 检查长度；通过元素 typesig 查 pooled SZArray typesig，再查 ArrayClass 的缓存，得到 UInt32[] 类。
2. 调用 `Class::initialize_all`，然后 `get_array_layout`，读取元素大小并进行乘法/加法/对齐的边界计算。
3. `GarbageCollector::allocate_array` → 通用 `allocate_object_impl` → size-class pool → arena freelist；设置 klass、length，做 finalizer 快速检查与内存压力统计。

两个数组元数据缓存的命中都不代表零开销。Unity 的生成代码则预解析 UInt32[] 类型并直接调用 `SZArrayNew(arrayClass, 16)`。它也必须做实际分配，不能认为 IL2CPP 消除了逃逸数组。

**已经量化的部分：**预缓存数组类为 74.9→68.8ns；预初始化并缓存固定布局为 74.9→55.4ns。因此元数据/布局准备确实值得优化，但不足以解释全部差距；剩余部分包含真正的分配、arena 增长、页提交/清零、根快照、Keep 调用、检查和循环，尚无采样 profiler 证据把这些进一步逐项分摊。

**重要更正：**本轮计数器显示三个诊断版本在 NewArray 的 200 万次计时区内，GC 次数全部是 **0**，`get_used_size()` 全部增加 **192000024 字节**。其中 192000000 = 2000000 × 96，额外 24 字节来自 host 的返回值装箱。这里的 used_size 是分配器计费对象字节，不是进程 RSS 或 arena 保留空间。

运行时的分配入口只记录压力，没有调用 maybe_collect；该 host/kernel 也未插入 GC 调度。因此开启 MARK_SWEEP/GCMode.ENABLED 不等于这个循环会自动回收。上份报告中“包含自然触发 GC”的表述已更正；分配项不能解释为四后端 GC 稳态吞吐。尤其不能把 LeanCLR 的差距归因于过多 GC pause。

当前 96 字节对象使用 16KiB arena，每个约放 170 个对象。200 万次数组意味着约 1.18 万个新增 arena，包含分配/清零、freelist 构建和地址范围登记。`MarkSweepHeap::register_arena` 使用排序 Vector：查找 O(log A)，插入移动最坏 O(A)，极端地址顺序下累计登记最坏可到 O(A²)。**这是代码确认的扩展性风险，尚未证明是本轮耗时的主因。**不能只看到这一算法就宣称它解释了 2.11 倍差距。

另做 3 次小样本 n 扫描，原始数据见 [scaling.json](data/il-hotspots-win64/scaling.json)：

| n | LeanAOT 原路径 | 缓存类与布局 | IL2CPP |
|---:|---:|---:|---:|
| 65536 | 70.407 | 45.891 | 34.186 |
| 262144 | 74.240 | 54.741 | 42.579 |
| 1048576 | 79.270 | 57.292 | 35.187 |
| 4194304 | 77.649 | 59.260 | 39.396 |

单位 ns/迭代。该扫描说明不能把所有开销视为固定 helper 成本；小样本和噪声也不足以拟合登记算法的实际复杂度。IL2CPP 的 GC 次数未在此实验采集。

**方案分三步：**

1. **先缩短代码生成路径。**在 newarr 调用点缓存完整 SZArray 类；已知元素类型缓存元素尺寸与初始化状态，提供已初始化 SZArray 分配入口。固定长度可折叠布局，变长仍保留负长度、乘加溢出、最大长度和 OOM 语义。缓存放在方法元数据或按数组类共享，避免每个对象增加字段，也避免每个调用点复制大块模板代码。
2. **再优化真实分配路径。**对小型、无引用、无 finalizer 的 SZArray 提供简短分配路径，保留 GC 计费及完整零初始化；评估 size-class 直接 freelist/bump allocation、批量 arena 提交，减少小 arena 的系统调用与地址登记。对范围索引可比较页索引/两级目录、分块排序结构与批量合并，必须同时测查找速度、碎片和元数据占用。不能只增大 arena/关闭清零换取吞吐而忽略内存与正确性。
3. **建立单独的 GC 稳态基准。**在确认 AOT 根与 safepoint 契约后明确安排 GC 调度；同时记录分配字节、GC 次数/耗时、峰值 working set、存活对象量。先验证存活引用不会丢失，再改变回收触发位置。当前“分配增长”结果继续保留，不能悄悄替换成另一种测量语义。

验收包括 uint/object/含引用 struct 数组，零/负/边界长度及溢出，数据必须全零，OOM、GC 压力、存活对象和长期有界内存。完成第 1 步后仍需预期存在分配器差距，原型没有证明可立刻追平 IL2CPP。

## 落地顺序与复现

建议先做 **Cast 的代码生成特化**（复用现有 O(1) 元数据、低额外内存），再做 **单播 Delegate 的 typed fast path**（已有明显收益，但绑定/ABI 验证更多），然后做 **NewArray 元数据快速路径**。GC 稳态口径和 allocator 优化应独立跟踪，避免混在同一个性能结论中。

```powershell
python scripts/benchmarks/probe_il_hotspots.py
python scripts/benchmarks/probe_il_hotspots.py --sweep-only
```

要求先存在原基准构建产物 `out/performance/il-families/`。诊断源码/二进制/构建日志位于 `out/performance/il-hotspots/`；记录所复用原基准库的 SHA-256，复测时可核对是否使用相同构建，而不是当前其他任务构建的运行时。C++ 原型及修改后的诊断生成文件已使用仓库 clang-format 和 .clang-format 格式化。

本文件对应的诊断阶段没有修改生产运行时或生成器；后续正式修改、边界测试和性能验证见 [正式优化报告](il-hotspots-optimized-win64.md)。原始数据保留。
