# 第二步：void / 托管指针轻量返回协议试点

本步在 AOT 内部加入可选协议，普通运行时 API 和公共 C API 继续使用原有接口。
默认仍为原 `Result` 协议，通过生成器参数 `--leanaot-lightweight-returns` 启用试点。
32 位整数、64 位值、结构体、byref、原生指针和非平凡 C++ 对象均保留原协议。

## 表示与边界

| 返回类型 | 试点表示 | 限制 |
| --- | --- | --- |
| void | 平凡 `AotVoidResult`，只有一个 `RtErr` 字段 | Wasm LLVM IR 直接返回 i8，不使用 sret |
| 托管对象/字符串/数组引用 | 平凡 `AotPointerResult<T>`，一个机器字 | 仅用于有至少两字节对齐契约的引用 |
| 其他类型 | 原 `RtResult<T>` | 保留全部取值和生命周期语义 |

void 使用平凡包装保留生成代码的检查接口，机器 ABI 为标量；没有为此全局替换 `ResultVoid`。
托管指针的最低位区分成功和错误：成功 null 编码为 0，错误编码为 `(RtErr << 1) | 1`。
byref/native pointer 不应用该压缩，避免对未对齐地址作假设。

`ReturnProtocol` 统一决定方法声明、直接调用、函数指针、invoker 和解释器桥接的返回类型。
模块注册数据记录实际返回协议，动态调用桥根据目标模块选择签名，支持调用方与目标使用不同协议。
icall/intrinsic 函数指针始终使用原运行时签名，再通过适配器进入 AOT 返回协议。
反射、解释器和宿主调用继续经普通 invoker 边界，不改变外部调用约定。

轻量内部类型在 Debug 和 Release 均为平凡类型，没有“析构时必须检查结果”的诊断字段和析构函数。
生成调用端仍检查错误；普通运行时 `Result` 的 Debug 诊断保持原样，适配器会消费并检查传入的旧结果。
构造对齐、非法成功错误码和错误 unwrap 的断言继续保留。这是显式独立的 AOT 协议，不按 NDEBUG 偷换运行时 ABI。

模块注册表扩展了返回协议字段；升级时需重新编译运行时、宿主与全部 AOT 代码，不能混用旧预编译注册表。
已经重新生成的模块可选择两种协议；未知协议编号在运行时初始化时拒绝。

## 验证与测量

功能验证覆盖新旧协议、泛型开启/关闭、EH 正常路径与解释器循环对照，以及 x64 Release、Wasm32、x64 Debug、Win32。
测试实际检查双向 void/指针 ABI 桥接，成功 null、非空引用、泛型引用 invoker、OOM 错误编码与传播、指针高位、嵌套结果、异常接管和 finally。
新增测试宿主将 Windows 断言送到 stderr，避免自动测试出现阻塞弹窗。

调用基准新增 void 和托管引用两条调用链，并保留叶子调用后的可观察工作。
生成器现在实际发射 `MethodImplOptions.NoInlining`；新旧协议都采用相同设置，避免靠内联消除掩盖返回 ABI。
因此本轮调用/泛型时间不能直接与历史 P0 的不同内联行为比较。

两套协议合计 **32 组构建配置、48 项 CTest 检查全部通过**，原有运行时回归 **41/41**。
跨程序集的 void、普通引用、泛型引用调用完成最终复验。CI 已扩展为两套协议矩阵，本轮没有执行远端 CI。

### 本机结果

同源托管方法、同一运行时与编译选项，7 轮、每项 5,000,000 次，编译完成后按固定种子串行交错采样。
所有 10 项负载在两种协议和原生/Wasm 间的 checksum 一致。
本机使用 Windows x64 / MSVC 19.44；Wasm 使用本机 Emscripten/Clang 22 和 Node v22.12.0。
未锁定 CPU 频率或排除所有系统背景负载。以下为 ns/op 中位数，越低越好。

| 平台 | 负载 | 原 Result | 轻量协议 | 耗时变化 |
| --- | --- | ---: | ---: | ---: |
| x64 | void 调用链 | 2.191 | 2.190 | -0.06%，基本持平 |
| x64 | 引用调用链 | 1.574 | 1.644 | +4.47% |
| Wasm | void 调用链 | 3.547 | 2.579 | -27.28% |
| Wasm | 引用调用链 | 2.974 | 2.002 | -32.69% |

Wasm 的新旧样本区间没有重叠；x64 引用样本区间有重叠，但其中位数确实升高，不能宣称原生平台获益。
原始样本、区间、制品/托管输入/实现源文件哈希、验证日志哈希和 LLVM IR 签名见
[data/aot-return-protocol.json](data/aot-return-protocol.json)。其中 parent_commit 标记工作区实现的父提交。

Release 和 Debug 的 Wasm LLVM IR 均确认：指针轻量结果直接返回 i32，void 轻量结果直接返回 i8；普通 Result 仍为 sret。
这证明消除了对应的隐藏返回地址，但不代表所有平台都会更快。

| Wasm 制品 | 原 Result | 轻量协议 | 差值 |
| --- | ---: | ---: | ---: |
| 整个测试程序字节 | 829553 | 829390 | -163 |
| code section 字节 | 664909 | 664742 | -167 |
| gzip 字节 | 264776 | 264771 | -5 |

x64 两个可执行文件均为 876544 字节，gzip 分别为 333586 和 333612 字节。
这些大小包含共享运行时，但不含外部 BCL、IL 和资源；体积差异很小，不能推断真实项目会明显缩小。

### 决策与限制

保持默认关闭，优先作为 Wasm 构建的显式试点。当前证据支持这两条 Wasm 调用链的耗时下降，
不支持全平台默认启用、全局替换运行时 Result，或承诺游戏整体加速/明显缩包。
32 位整数、宽值和结构体的协议选择留待后续单独审阅。

动态 void/引用调用桥增加了目标模块协议选择；本轮验证了其正确性，尚未专门量化这项分派成本相对旧提交的变化。
没有重新测量 IL2CPP、浏览器或真实游戏负载，也没有把错误编码/传播测试称为实际内存耗尽压力测试。
异常接管策略保持上一轮设计：优化返回协议不等于异常密集路径已加速。

## 复现

```powershell
# 默认协议；增加 --lightweight-returns 验证试点协议。
python scripts/test/aot-features.py --target all --emsdk D:/workspace/wasmclr/emsdk
python scripts/test/aot-features.py --target native --config Debug
python scripts/test/aot-features.py --target native --arch Win32

python scripts/benchmarks/run_aot.py --build-only --output out/performance/aot-returns/result --emsdk D:/workspace/wasmclr/emsdk
python scripts/benchmarks/run_aot.py --build-only --output out/performance/aot-returns/lightweight --lightweight-returns --emsdk D:/workspace/wasmclr/emsdk
# 等其他编译进程完成后采样；该命令也验证 Release/Debug 的 LLVM IR ABI。
python scripts/benchmarks/compare_aot_returns.py --emsdk D:/workspace/wasmclr/emsdk
```
