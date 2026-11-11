# Core C++17 升级实施报告

日期：2026-11-11。按升级评估的 A–C 范围实施：统一最低标准、删除旧标准兼容实现、验证边界与性能/内存。没有混入运行时架构或分配器重构。

## 实施内容

- Core、host、Mono BCL object targets 及最终库均以 PUBLIC `cxx_std_17` 导出使用要求；所有仓库自有 CMake 消费入口及独立编译探针一起升级。C 源文件及第三方源码保持原来的语言要求。
- 非 CMake 集成在 `build_config.h` 检查最低标准，MSVC 使用 `_MSVC_LANG`，其他编译器使用 `__cplusplus`。低于 C++17 明确报错。不会强行覆盖宿主 C++20 标准。
- `stl_compat.h` 保留包含路径，删除约 380 行自行添加到 `namespace std` 的 optional/string_view 实现，只包含标准库头文件。补齐原先通过兼容头间接获得的依赖。
- 更新编辑器参数、Wasm ABI 探针、README 与架构文档。CI 新增 Windows/Linux C++20 配置，以及 C++17 运行时供 C++20 使用者链接的检查。
- 工具/样例构建检查修复了此前遗留的接入问题：leanrun 与原生 P/Invoke 样例改用已有异常格式化 API，保留 StackTrace 输出，避免访问不透明的 BCL 异常字段；P/Invoke 文件加载器采用当前 bool + out 回调及明确的缓冲区所有权。正常执行和抛异常两条实际托管路径均验证。
- 测量发现并修复 C++17 聚合初始化导致的 Wasm 体积回归，具体证据见下节。只调整解释器临时操作数的构造方式，不改变字段默认值、字节码编码或内存分配。
- C++ 异常、RTTI、线程模型、GC、宿主 MemoryCallbacks、对齐分配、Result ABI 和持久化泛型键均未改动。并未引入 filesystem、并行算法或默认 STL 分配器替换。

## 使用审计

现有 optional `.value()` 调用由有值检查控制。生命周期探针覆盖构造、移动、reset 和空值；不依赖移动后源 optional 变为空的错误假设。string_view 的所有权不变：名称解析保存 owning string，临时视图不进入长期缓存；trim 等操作遵守长度，类型键允许空名称的路径使用空视图。C 字符串哈希键仍遵守原有非空且 NUL 终止的接口契约。

运行时标准哈希仅用于内部容器；托管 String.GetHashCode 和持久化泛型注册键采用各自已有算法，没有依赖实现相关的标准库哈希。不能把不同标准库之间的哈希值当作稳定序列化数据。

## 验证

验证入口：`scripts/test/cpp17.py`、`scripts/test/linux-standard.py`、现有 AOT/PGO/返回协议脚本及 `scripts/benchmarks/compare_cpp_standard.py`。本地工具链为 MSVC、Emscripten/Clang 与 WSL GCC 13.3；详细版本、命令和原始测量在随附 JSON。

| 配置 | 本地结果 |
| --- | --- |
| Windows x64 Native Release / Debug，Win32 Release，Portable Release | 各 42/42，含 C++20 使用者 |
| Custom / Windows DLL | 4/4、3/3；公共 C/C++/C++20 边界 |
| Wasm32 Portable + Node | 19/19 独立运行时检查 |
| 整个运行时采用 C++20 | 三个 object targets 的有效编译参数及探针 `202002` 通过 |
| 低于最低标准 | Native/Wasm C++14 均按预期拒绝，并输出 C++17 诊断 |
| 默认 AOT 泛型/EH | Native/Wasm 共 8 个构建配置；含禁用泛型、解释循环、filter/fault 回退和规划器 |
| PGO + 全返回协议 + 引用共享 | 4 个采样配置 + 12 个 guided/stale/Debug/Win32 配置，每次 71 个正式用例及 PGO 场景 |
| 返回边界 | Native Release/Debug、Wasm 的独立测试；Wasm LLVM IR 验证 64 位整数标量返回无 sret |
| GNU x64 | 完整 Native/Mono BCL 静态库，18 个回归检查与探针，最终代码 ASan/UBSan 通过 |
| 工具和样例 | leanrun、startup、原生 P/Invoke、两个 Wasm 样例均构建通过；leanrun 和原生 P/Invoke 覆盖正常/异常输出 |
| Mono 同步后的 Native + Mono BCL | 41/41；所有运行时代码包含 C++17 和紧凑初始化修复 |

本地版本：MSVC 19.44.35229.0，Emscripten 4.0.12（Clang 22.0.0），GCC 13.3.0，Node v22.12.0。两个 Wasm 样例只完成编译/链接，浏览器初始化及 JS 宿主没有在此轮执行。


Linux 的可用 WSL 环境没有 CMake，因此使用与 runtime CMake 对应的源码组成，直接用 GCC/gcc/ar 构建完整 Native + Mono BCL 静态库，再运行 18 个独立回归检查及标准库/容器探针。ASan/UBSan 开启且禁止恢复；此轮禁用泄漏检测，不代表已证明无泄漏。旧 C++11 与新 C++17 的非 sanitizer 探针用于核对布局和分配计数；Linux 样本采集时存在并发构建，不用于性能结论。

ARM64、本机 Linux CMake、移动平台、控制台和 Unity/UE/Godot 集成没有在此次本地执行；已有 CI 的 Linux ARM64/GCC、Clang sanitizer/fuzzer 与 Wasm 配置仍保留。CI 配置更新不等于远端工作流已经运行。

## 新旧标准测量

原始基线对应 `1685fb67`，MSVC 有效语言模式为 C++14（此前 CMake 请求 C++11），Wasm 为 C++11；升级后有效模式均为 C++17。两次使用相同编译器、优化设置、托管 DLL 字节及默认 AOT 返回/缓存配置。每组 7 次独立进程采样，构建完成后再测量；CPU 频率与后台负载未严格隔离。

进一步保留两个版本的原始二进制，将 AOT 循环提高到 5,000,000 次，7 轮随机交错旧/新进程，降低采样时段和短循环的影响。下表采用交错重测中位数，单位 ns/迭代：

| AOT 工作负载 | Native 旧 | Native C++17 | Wasm 旧 | Wasm C++17 |
| --- | ---: | ---: | ---: | ---: |
| arithmetic | 0.791 | 0.788 | 0.107 | 0.110 |
| array | 0.795 | 0.789 | 0.793 | 0.799 |
| calls | 2.581 | 2.577 | 4.387 | 4.387 |
| exceptions | 28.394 | 27.259 | 30.600 | 30.814 |
| generics | 1.010 | 0.989 | 2.799 | 2.810 |
| int32-returns | 3.558 | 3.605 | 5.028 | 5.028 |
| interfaces | 4.942 | 4.002 | 6.948 | 6.984 |
| properties | 0.791 | 0.805 | 0.101 | 0.101 |
| reference-returns | 1.554 | 1.568 | 2.997 | 2.903 |
| structs | 8.567 | 8.616 | 9.561 | 9.527 |
| uint32-returns | 3.552 | 3.568 | 5.023 | 5.107 |
| void-returns | 2.200 | 2.172 | 3.412 | 3.400 |

本次交错采样中，Native 接口循环约快 19%，异常循环约快 4%；其他 AOT 项目的旧/新最小—最大样本区间重叠。Wasm 所有 12 项均重叠，未建立可靠的统一加速或变慢结论；这些区间不是统计置信区间。Native string_view map 查询 55.887 → 36.568 ns（约快 35%），Wasm 40.468 → 40.903 ns（区间重叠）。不能将特定编译器/布局的观测外推为所有平台收益。

| 产物 | 旧字节数 | C++17 字节数 | 差值 |
| --- | ---: | ---: | ---: |
| native_aot | 896,512 | 892,416 | -4,096 |
| wasm_aot | 845,621 | 845,159 | -462 |
| native_probe | 32,768 | 33,280 | +512 |
| wasm_probe | 28,895 | 28,918 | +23 |

完整 Wasm code section 为 678,178 → 677,669 字节，gzip 为 270,022 → 269,464 字节；探针自身小幅增长也完整列出，没有将探针缩小当作整库结论。

下列指标在对应平台的新旧标准之间保持一致：

| 平台 | 分配/释放次数 | 峰值及稳态 payload 字节 | 累计请求字节 | view / optional uint64 / optional method body / Result uint32 / method info 字节 |
| --- | ---: | ---: | ---: | --- |
| MSVC x64 | 4103/4103 | 229416 | 287912 | 16 / 16 / 64 / 8 / 112 |
| Wasm32 | 4108/4108 | 107604 | 133108 | 8 / 16 / 36 / 8 / 64 |
| GCC x64 | 4105/4105 | 204536 | 240472 | 16 / 16 / 64 / 8 / 112 |

基准的托管输入 hash、所有旧/新工作负载校验和一致。初始采样、交错采样、布局、回调分配、编译器、命令、验证记录与产物 hash 见 [升级数据](data/core-cpp17-upgraded.json)，旧标准完整基线见 [基线数据](data/core-cpp17-baseline.json)。


首次 C++17 构建的完整 Wasm 产物为 859,087 字节，比基线增加 13,466 字节。使用同一 llvm-nm/objdump 对比保留的旧、新对象定位到 `InstructionOperands ir{}`：带默认成员初始化器的类型在 C++17 成为聚合，Clang 将初始化展开为多条 store；旧标准的非聚合值初始化使用紧凑的 memory.fill。未链接对象中的 `Transformer::transform_body` 从 33,954 增至 47,252 字节，`add_stloc` 从 224 增至 352 字节。

为 `InstructionOperands` 添加显式默认化的默认构造函数，阻止其成为聚合，但构造函数仍非 user-provided，保留标准值初始化及所有有类型的成员默认值。没有用手写 memset 假设空指针表示。静态断言保护非聚合约束。修复后这两个函数分别恢复到 33,954 / 224 字节，objdump 确认恢复 memory.fill；最终体积和重测结果见上表。首次增长版本的体积/hash 也随最终数据保留，没有将它作为已接受的回归。

容器内存探针统计通过宿主回调申请的 HashMap payload 字节，输入字符串在 map 外持有；排除回调头、CRT/RSS、其他运行时堆和对齐分配路径。所有记录的分配均配对释放。该数据不能外推为整个 CLR 的常驻内存变化。

本轮性能对照覆盖现有 12 组 AOT 工作负载与字符串视图容器，并保留进程总耗时。未建立独立的解释器吞吐、冷启动/BCL 加载、GC/句柄及整个堆/RSS 的前后性能对照；这些行为的正确性由回归检查覆盖，局部基准不能证明整个 CLR 在所有场景都没有性能回归。

## 集成与回退

源码集成和非 CMake 的生成 AOT 构建均须启用 C++17 或更高标准。CMake 使用者链接 `leanclr` 可继承最低要求。C++20 消费者和全运行时 C++20 配置单独验证，Core 实现仍只要求 C++17。

C 公共边界未改动；涉及 C++ 标准库类型的内部 ABI/布局不承诺跨旧兼容实现或不同标准库稳定。升级时同时重编运行时、生成的 AOT、宿主 C++ 对象和原生插件，不能混用旧对象文件。暂不能升级工具链的使用者可固定在升级前的 `1685fb67`；此次不保留一套旧 optional/string_view 继续维护。

`Result::cast` 的 if constexpr 简化、分批 nodiscard 等属于评估的后续 D 阶段候选，本轮没有把它们作为升级必要条件，也没有宣称仅改变语言模式便能自动提高 AOT 性能。

## 本轮 AOT 路线图状态

五项均已实现并分别测量/验证；“实现试验协议”与“默认启用”是不同决策。

实现提交：整数返回 `6f324934`（此前已提交）、泛型 EH `56d7cbfc`、宽值协议 `91e69f9a`、引用共享 `7d603b97`、PGO `1685fb67`。升级分为基线 `77a230e8`、构建要求 `dc4775b6`、兼容层移除 `43dd51e7`、工具修复 `bb0c7969`、体积回归修复 `ec14b33b`、验证 `b4b99ad8` 和交错测量工具 `86d673dd`，便于独立审阅和回退。

| 任务 | 交付与决策 |
| --- | --- |
| int/uint 标量返回 | 完整 32 位值与错误码打包为 64 位标量，接入实际 AOT 及混合协议边界；保留 opt-in。见 [int32](aot-int32-returns.md)。 |
| 泛型 EH 正常路径 | 安全的闭合泛型使用真实泛型上下文执行 AOT；异常后携带快照交给解释器。危险形状继续回退。见 [generic EH](aot-generic-eh.md)。 |
| long/double/struct 返回 | 真正的 status + out ABI 及所有边界适配；测量不支持普遍默认开启。见 [wide returns](aot-wide-returns.md)。 |
| 引用泛型代码共享 | 无类型相关元数据操作的引用实例共享 Object 规范函数体，实际身份/上下文不变；含 token/EH 等复杂形状继续特化。见 [reference sharing](aot-reference-sharing.md)。 |
| PGO 决策 | 有界接收者/逐调用复用观测、方法与 IL 根预算、缓存容量与代码生成预算、过期 IL 指纹拒绝。见 [PGO caches](aot-pgo-caches.md)。 |

宽值协议、引用共享和 PGO 的适用范围均有明确边界；这些交付没有把“所有泛型形状共享”或“完全原生异常处理”扩入已同意的范围。
