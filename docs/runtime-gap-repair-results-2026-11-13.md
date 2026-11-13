# 运行时差距逐项修复结果（2026-11-13）

本轮按授权完成第02条后跳过03，连续处理04–12。共享问题同步到 mono-minimal、mono-full、coreclr-minimal；仅某适配器存在的问题只修改对应实现，其余分支增加同组回归测试。未 push。

## 提交索引

| 编号 | 内容/结论 | coreclr-minimal | mono-full | mono-minimal |
|---|---|---|---|---|
| 02、11 | 默认 ALC 的 LoadFromStream；缓冲区/PDB 所有权与失败清理；文件流句柄类型修正 | cce65a51 | 19280761（共享加载器） | 4de58c37（共享加载器） |
| 04 | 反射 ref/out 回写、原装箱值隔离、null/nullable、GC 根；异常时不回写 | 104896f8 | 36c2731c | c4892eec |
| 05 | CoreCLR RuntimeMethodHandle_InvokeMethod 的 byref 转换与回写，原生 typed-slot 根 | 80842a58、d6d0298e | 无该入口，04覆盖公共路径 | 同左 |
| 06 | 按虚表槽消除 override 重复；保留 newslot 与重载 | 3227f081 | 9bfa5f19 | ddc16f6a |
| 07 | 泛型定义、闭合/未闭合实例、嵌套类型、限定名与显示名 | 76dce850 | fc0f2a13 | f23aef7f |
| 08 | Mono CustomAttributeData 不实例化用户特性；Module token；CoreCLR 原实现回归通过 | 874cf8a6（测试） | 79f3dd2c | d11fe4c8 |
| 09 | Console.Out/SetOut 虚调用；Portable 标准流句柄及 UTF-8 编码 | a6d9dce0 | 5fc2ff1e（测试） | e788061a（测试） |
| 10 | 基元 Marshal 矩阵；修正无符号拆箱；标量转换免 staging 分配 | d98f6d60 | 2bc0b710 | 2f22a1a9 |
| 12 | 当前代码未复现 ctor 替换对象；直接原生测试锁定 existing-this 契约 | a1efc04b、3102b84c | db8e52ac、812f83b6 | f879c542、e848b418 |

第05条补充提交 d6d0298e 在首个参数装箱前准备所有参数布局，保证后续未装箱结构体的 GC 位图可用，并缓存类型供回写复用。

第01条已实现结构体/基元封送引擎，本轮10不重复另建实现；扩展测试实际发现并修复 unsigned unbox 符号扩展和 BOOL 逐次 staging 分配。第11条与02是同一条默认 ALC 数据加载链路，按一个修复提交。

## 测试证据

- Assembly stream：Native 24/24、MSVC ASan 24/24、Wasm32 22/22；包含原生分配失败、重复/冲突程序集、符号数据、缺失依赖后重试、默认上下文限制与文件流。AOT 调用方10组通过。
- Reflection invoke：CoreCLR Native、MSVC ASan、Wasm32、Mono Full、Mono Minimal 的公共8组 + 原生契约1组均通过9/9。CoreCLR 原生契约直接调用 QCall，包含首参数装箱时触发 GC、后续原始结构体参数引用仍存活的场景；三分支均测试 reference/value 构造函数的 existing-this、字段、次数和身份，并额外链接实际 AOT 构造函数，断言 AOT invoker 后通过同组原生契约。
- Reflection metadata：13组（虚函数6、泛型名4、特性3），对照 .NET 10，并在 Mono 两分支与 CoreCLR 检查；CoreCLR ASan/Wasm 同组通过。特性组包含 type/method/field/property/event/parameter/return/assembly/module 九类目标。
- Console：6组通过 CoreCLR Native、ASan、Wasm，Mono Full 与 Native 宿主的 Mono Minimal；检查实际默认输出内容，避免把“没有报错但丢失输出”记为通过。
- Struct marshal：三分支 Native、CoreCLR ASan/Wasm 套件各7/7。基元矩阵包含极值、NaN/infinity、指针宽度、非对齐地址和越界哨兵；原生计数验证缓存后的 int/BOOL 各10,000次转换无新增原生分配。
- 最终 .NET 10 集成门禁：77 managed smoke + 15 real-heap GC + 3个原生契约组通过。
- 实际 LeanAOT：本轮四个测试程序集生成并编译 C++ 后，12组关键入口通过；覆盖 ref/struct/nullable/异常、虚函数、泛型名、特性、Console 和 Marshal。不是仅把解释器执行记为 AOT 验证。

测试源码位于 `src/tests/assembly-stream`、`reflection-invoke`、`reflection-metadata`、`console-output`、`struct-marshal`。构建产物保存在本地 `out`，不提交。Wasm 使用匹配指针宽度的32位 CoreLib；本机64位 CoreLib 不作为其替代。

## 边界

- ALC 仍是 minimal 的默认、不可卸载上下文；自定义/collectible 上下文明确拒绝，拒绝路径不泄漏构造时创建的 GCHandle。不宣称支持隔离加载或卸载。
- 反射目标抛异常时维持现有直接传播行为；本轮验证不回写参数，不把 TargetInvocationException 包装行为列为已实现。
- Mono Minimal 默认 Portable 宿主未提供 Mono Console 初始化所需 OS 查询，Console 套件明确跳过该组合；通过 Native 或具备相应能力的自定义宿主运行。
- 第12条是当前实现的回归锁定，没有为了符合旧报告而引入重复分配或改写已正确的构造函数行为。
- Linux/ARM64 未在本轮实机运行，不记为已通过。没有使用被限制的 Linux 机器。

第03条已获批准并完成本轮实现，见 [calli 实施与验收报告](calli-support-results-2026-11-13.md)。
