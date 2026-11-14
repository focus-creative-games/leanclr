# NewArray 分配器优化：Win64 实测

2026-11-14。上一轮 Cast/Delegate/NewArray 优化已先提交为 **c64a033d**（Optimize AOT casts, delegate calls, and array allocation setup）。本轮基于该提交，直接复用其已保存的 AOT 二进制作同轮对照；不把最初未优化的 74ns 版本当作本轮起点。

## 结果

四后端均使用相同输入 DLL、seed=19 和 **200 万次 NewArray 迭代**，7 次随机顺序采样。单位 ns/迭代，中位数，越低越好。

| 后端 | 中位数 | 最小值 | 最大值 |
|---|---:|---:|---:|
| c64a033d AOT | 53.449 | 50.176 | 55.404 |
| 新分配器 AOT | 40.776 | 40.129 | 42.189 |
| Unity 2022.3.62f3 IL2CPP | 31.912 | 31.328 | 32.319 |
| .NET 10.0.8 JIT | 7.773 | 7.327 | 7.863 |

相对已提交版本，NewArray **加速 1.31×，耗时下降 23.7%**。相对 IL2CPP 的耗时比由 **1.67× 降为 1.28×**，仍有约 27.8% 的差距。此结论针对本用例，不能换算成完整应用/Unity 帧率。

验证数据包括 **910 个全量样本 + 84 个等迭代数对照 + 36 个增长扫描样本**；另有 6 个独立内存测量进程，不将其轮询干扰后的耗时混入延迟表。全量样本逐项校验 .NET oracle；等迭代数对照和增长扫描也全部通过跨后端 checksum 校验。

## 正式改动

1. **arena 指针表改为几何扩容。**此前每次 refill 使用 `try_reserve(size+1)`，Vector 每次重新分配并复制旧指针，A 个 arena 的累计复制为 O(A²)。现在容量按 8、16、32…增长，先完成 fallible reserve，arena 成功分配/登记后才发布指针。该指针表的累计扩容复制降为 O(A)；OOM 时不发布半初始化 arena，也不会泄漏已取得的 payload。
2. **新块游标 + 回收 free list。**新 arena 仍整体清零，但不再预先遍历所有块建立链表；从未使用的块通过索引游标取得，回收块继续从 free list 获取。新块不再做地址差除以运行时 block_size 的索引除法。位图仍准确标记已分配槽位，未使用尾部不被视为对象；quarantine 块不重用。
3. **内联小块热路径。**SmallHeapArena 和 SizeClassPool 的成功分配在头文件内联，arena refill 保留冷函数，避免每次块分配跨越两层外部调用。没有增加按数组长度建表的缓存。
4. **数组专用分配入口。**数组本身没有 finalizer，跳过数组的 per-object finalizer 注册检查；普通对象仍走注册路径，数组中的可终结元素照常存活和终结。GC_DEBUG 的传入 GcAllocSite 现在正确转发，补上旧数组入口忽略 site 的问题。
5. **合并压力计费调用。**一次调用同时更新 used size 和既有的 aligned allocation/pressure/lifetime counters；没有省略统计、推迟计费或改变 GC 触发策略。

主要代码位于 `src/runtime/gc/mark_sweep/small_heap_arena.h/.cpp`、`mark_sweep_heap.cpp` 和 `gc_pressure.h/.cpp`。以上是组合实现的端到端收益，未用 profiler 将每一项独占耗时分摊，因此不为某个单独改动声称固定 ns 收益。

## 内存代价与实测

- Managed 对象/数组布局及大小不变；NewArray 的对象计费仍为每次 96 字节，另有每次 host 调用的 24 字节返回值装箱。前后计时区 GC 次数均为 0，保留原“分配增长”口径。
- SmallHeapArena 的 count/cursor 使用有界 uint16（最多 2048 块），重用旧 count 的空间；Win64 **arena 元数据仍为 296 字节**，native static_assert 验证。arena backing 大小、大小类和清零要求不变，上一轮的 96 字节静态数组元数据缓存也不增加。
- 几何扩容允许更多空闲指针槽：大池容量小于 2A，小池首批 8 个指针（64 字节）。它减少分配/复制次数，代价是少量可复用的 metadata capacity；不是零空间代价。在 200 万个 96 字节数组的量级，额外容量是几十 KiB，远小于 payload。OOM 后允许保留已经成功预留的容量供重试。
- Native 可执行文件 **898560 → 898048 字节**；静态库 **6599032 → 6598826 字节**。这是当前构建产物大小，不是完整发布体积。

独立内存测量采用相同 n=2000000，每版本 3 个新进程，逻辑 CPU 2，约 1ms 轮询 Windows K32GetProcessMemoryInfo。Peak working set 使用内核累计峰值字段；private bytes 是轮询观察峰值。下表均为三个进程的中位数（MiB，1MiB=1048576B）：

| 指标 | c64a033d AOT | 新分配器 AOT |
|---|---:|---:|
| Peak working set | 242.68 | 215.11 |
| Observed peak private bytes | 239.43 | 211.98 |

本场景峰值工作集观察到下降 **27.57MiB（11.4%）**。managed 对象计费完全相同，这个差异可能包含系统堆的容量/碎片、地址布局等影响；不能把全部节省字节归给一个函数，也不能据此推导所有应用的 RSS。采样精度和系统环境仍会影响结果。原始 6 个进程及 stdout 保存在数据目录。

## 增长扫描与剩余差距

每格为 3 次中位数，ns/迭代；三个后端使用相同 n。

| n | c64a033d AOT | 新分配器 AOT | IL2CPP |
|---:|---:|---:|---:|
| 65536 | 44.334 | 40.189 | 32.877 |
| 262144 | 52.213 | 40.048 | 34.395 |
| 1048576 | 53.521 | 42.398 | 33.352 |
| 4194304 | 53.451 | 44.069 | 33.445 |

短循环收益小于长循环，符合减少 arena 增长额外成本的方向，但该扫描不是复杂度拟合。原有按地址排序的 heap-range Vector 登记仍可能移动 O(A) 项，backing 分配/清零、页提交、位图维护和压力计费也仍在；本轮没有把整个分配器变成全局摊销 O(1)，也没有改变 arena 大小来换吞吐。

没有启用自动 GC。长期、带存活对象和 GC pause 的稳态仍应单独衡量，当前数字不证明与 IL2CPP/.NET 的完整 GC 行为等价。

## 边界验证

默认返回 ABI、全部返回协议、引用泛型共享和 GC_DEBUG 四组配置共 **8 项 CTest 全通过**，既有运行时回归 **42/42 通过**。新增直接验证：

- 16/24/96/256/8192 字节块，包含 64 位图字边界、最大 2048 槽位、不可使用的尾部字节；部分 sweep 后混用回收块和 virgin tail，再耗尽、完整 sweep 和 refill。
- 幸存 payload 不变、所有新块/回收块全零、不重叠；未使用槽位不成为对象。GC_DEBUG 下 quarantine 块保持不可重用，并验证 codegen 分配站点。
- 指针表预留失败、payload 已取得后 arena 元数据失败、失败回滚释放 payload、重试成功和随后 sweep 的 ownership/计费正确。
- OOM 不增加对象占用、压力或 lifetime bytes；普通成功分配仍由既有 GC accounting 回归验证。
- 引用/含引用 struct 数组跨 GC 存活、释放后有界对象量；具有 finalizer 的数组元素不会在数组存活时提前终结，释放后正确终结一次。

所有修改过的 C++ 文件均用仓库 clang-format 与 .clang-format 格式化后构建/测试。测试日志、构建清单、二进制散列、源码散列和全部样本见 [数据目录](data/newarray-allocator-win64/results.json)。

## 全部 26 项复测

此表按每个后端校准约 100ms，7 次中位数；迭代数可不同，因此三项热点还另做上述等迭代数控制。近似 1× 的小差值应结合 CSV 范围看待。

| 用例 | 新解释器 | c64 AOT | 新 AOT | c64/新 AOT | IL2CPP | .NET 10 JIT |
|---|---:|---:|---:|---:|---:|---:|
| Int32 | 20.760 | 0.998 | 1.000 | 1.00× | 0.996 | 1.005 |
| Int64 | 20.800 | 0.998 | 1.015 | 0.98× | 0.995 | 1.003 |
| DivRem | 27.412 | 3.389 | 3.392 | 1.00× | 3.181 | 4.459 |
| Float32 | 19.258 | 1.399 | 1.397 | 1.00× | 1.393 | 1.416 |
| Float64 | 20.113 | 1.394 | 1.402 | 0.99× | 1.390 | 1.411 |
| Convert | 28.900 | 4.013 | 4.013 | 1.00× | 4.042 | 4.811 |
| Checked | 21.120 | 0.996 | 1.023 | 0.97× | 1.000 | 1.213 |
| Branch | 20.910 | 0.401 | 0.400 | 1.00× | 0.415 | 0.422 |
| Switch | 29.555 | 1.837 | 1.819 | 1.01× | 1.937 | 1.808 |
| ArrayLoad | 24.233 | 1.587 | 1.598 | 0.99× | 1.595 | 1.637 |
| ArrayStore | 26.871 | 0.803 | 0.803 | 1.00× | 0.797 | 0.603 |
| Field | 18.794 | 0.798 | 0.804 | 0.99× | 0.796 | 0.803 |
| StaticField | 17.201 | 0.797 | 0.798 | 1.00× | 0.794 | 0.801 |
| ByRef | 25.830 | 1.235 | 1.234 | 1.00× | 1.401 | 0.802 |
| Calls | 22.919 | 0.812 | 0.796 | 1.02× | 1.596 | 0.804 |
| Virtual | 32.320 | 4.393 | 4.093 | 1.07× | 3.863 | 1.175 |
| Interface | 33.264 | 5.227 | 5.572 | 0.94× | 4.622 | 1.786 |
| Delegate | 40.475 | 4.917 | 4.198 | 1.17× | 3.834 | 1.654 |
| Struct | 62.392 | 8.627 | 8.585 | 1.00× | 7.390 | 6.142 |
| Generic | 22.996 | 0.804 | 0.796 | 1.01× | 1.647 | 0.803 |
| Cast | 45.209 | 1.740 | 1.717 | 1.01× | 1.000 | 1.041 |
| Boxing | 55.838 | 23.353 | 18.786 | 1.24× | 27.421 | 5.898 |
| Allocation | 67.051 | 25.164 | 21.774 | 1.16× | 28.662 | 5.136 |
| NewArray | 85.144 | 51.494 | 40.375 | 1.28× | 32.965 | 7.089 |
| EhNormal | 25.692 | 0.996 | 0.998 | 1.00× | 0.994 | 1.024 |
| EhThrow | 33.704 | 31.610 | 32.420 | 0.98× | 3.445 | 1.941 |

EhThrow 仍是首次异常后解释器续执行。相同托管 DLL SHA-256 为 `586a76d1bbfe32c00a81ad84538be300a7db2d9aa2229beb56f4c4d1258ebf5c`。Win64 Release，逻辑 CPU 2，seed=19、3 次预热、随机顺序、非并行测量；Unity 2022.3.62f3 独立 IL2CPP CLI，.NET 10.0.8 JIT 关闭 tiering/ReadyToRun，沿用上一轮工具链及选项。

## 复现

必须保留 **c64a033d 版本**的构建产物在 `out/performance/il-families-optimized/`，不能先用新运行时覆盖该目录。新产物和新报告单独保存：

```powershell
python scripts/test/aot-hotspots.py --all-protocols
python scripts/test/aot-hotspots.py --gc-debug
python scripts/benchmarks/run_il_optimized.py --baseline out/performance/il-families-optimized --output out/performance/il-array-allocator
python scripts/benchmarks/profile_array_allocator.py
python scripts/benchmarks/report_array_allocator.py
```

比较驱动会重跑全量和等迭代数样本；profile 驱动另外执行增长和内存测量。完整基准的 min/median/max 在 [summary.csv](data/newarray-allocator-win64/summary.csv)，等迭代数数据在 [paired-results.json](data/newarray-allocator-win64/paired-results.json)，内存数据在 [allocator-memory.json](data/newarray-allocator-win64/allocator-memory.json)。
