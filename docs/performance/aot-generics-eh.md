# LeanAOT：封闭泛型与异常正常路径

本轮实现封闭泛型 AOT，以及“正常路径 AOT、发生异常后由解释器接管当前调用”。
生产 `Result<T>` 协议保持原样。此前 P0/P1 的数据文件保持原样，本轮数据单独记录。

## 实现范围

- 从已选 AOT 方法递归发现封闭泛型调用，按定义、声明类型实参和方法实参去重；支持跨程序集、泛型类构造/静态构造、泛型结构体、约束调用和委托。
- 生成独立特化及运行时注册项。泛型 AOT 函数携带实际 `RtMethodInfo` 上下文，接口调用根据实际实现选择 ABI，避免接口与实现的泛型性不同造成错误。
- 默认全局预算 2048 个实例、发现深度 32；`--leanaot-max-generic-instances 0` 可关闭。未注册的动态实例继续解释执行，不借用其他类型的特化。
- 对满足状态快照条件的 EH 方法生成正常路径及正常退出的 finally。catch 由解释器执行；出错时复制当时参数和局部变量，在对应 IL 位置进入异常分派，不重放故障指令或之前的副作用。本次调用随后保持解释执行。
- 正常路径不预先生成解释器 IR；状态打包发生在错误分支。EH 方法保留空引用及数组边界检查，保证这些错误能够进入接管流程。
- `generic-aot-report.json` 记录特化、回退原因与 EH 正常路径方法，便于核对实际覆盖率。

泛型发现只跟随已选根方法中的静态引用，不能穷举未来反射目标和所有虚方法覆盖。
泛型 EH、filter/fault、byref/指针/值类型快照、值类型 this、取地址、finally 内嵌套 EH、未初始化局部变量或超过快照预算的方法，仍整方法解释执行。
managed `calli` 方法也保留解释执行，避免签名式函数指针调用遗漏泛型上下文；委托通过运行时 invoker 调用并有覆盖测试。
原有解释器的 filter 语义保持原样，本轮没有新增跨帧 filter 搜索机制。

## 验证

本机通过以下 16 组配置，每组执行相应 fixture 的 CTest 检查：

| 平台/配置 | 泛型开启 | 泛型关闭 | EH 正常路径 | EH 循环解释对照 |
| --- | --- | --- | --- | --- |
| Windows x64 Release | 通过 | 通过 | 通过 | 通过 |
| Wasm32 / Node Release | 通过 | 通过 | 通过 | 通过 |
| Windows x64 Debug | 通过 | 通过 | 通过 | 通过 |
| Windows Win32 Release | 通过 | 通过 | 通过 | 通过 |

泛型用例检查实际后端，包括不同类型但相同原生签名、递归预算、跨程序集、两种方向的泛型接口实现、委托，以及反射创建已注册/未注册实例。
规划器检查通过，默认单程序集用例发现 45 个特化。
EH 用例检查副作用只执行一次、参数修改、标量/引用局部变量、空 throw、嵌套 finally、finally 替换异常、rethrow、多个 leave、浮点/64 位/void 返回、无异常时不创建解释器 IR、错误接管位置拒绝及帧栈清理。
现有运行时回归 41/41 通过。CI 增加相同的 AOT 功能配置；这里报告的是本机结果，尚未运行远端 CI。

## 性能测量

Windows x64，MSVC 19.44；Wasm 使用本机 Emscripten/Clang 22、Node v22.12.0。
每项 5,000,000 次，7 轮，编译完成后串行交错采样，记录中位数、最小值、最大值，并校验所有配置的 checksum。
未锁定 CPU 频率或排除全部系统背景负载。时间为 ns/op，越低越好。
原始记录、制品哈希及体积见 [data/aot-features.json](data/aot-features.json)。记录中的 commit 是实现前的父提交，源文件哈希和制品哈希标识本轮工作区样本。

| 负载 | 平台 | 同运行时解释对照 | 本轮 AOT |
| --- | --- | ---: | ---: |
| 泛型 Identity 循环 | x64 | 16.631 | 0.799 |
| 泛型 Identity 循环 | Wasm | 21.053 | 0.104 |
| 含 EH、实际不抛异常的循环 | x64 | 28.074 | 1.305 |
| 含 EH、实际不抛异常的循环 | Wasm | 26.360 | 0.818 |

泛型对照只把实例预算设为 0；EH 对照只通过规则排除同一个 `NormalLoop`。
两边使用同一运行时、同一托管方法。Identity 特化后可被 C++ 编译器内联并继续优化循环，极小数值不能当成普通泛型调用成本，更不能外推成游戏整体提速。
本轮未重新测量 IL2CPP，因此不能据此宣称已全面追平或超过 IL2CPP。

原有 `Exceptions` 基准在循环第 0 步就抛异常，随即切到解释器执行余下循环。
本轮该项中位数仍为 x64 32.829、Wasm 30.861 ns/op，说明异常密集路径的性能问题尚未解决。
正常路径加速不等于异常处理本身加速；首次异常还包含解释器翻译和接管成本，本轮没有单独量化这项延迟。

## 体积结果与限制

| 同运行时比较 | Wasm 字节增量 | code section 增量 | gzip 增量 |
| --- | ---: | ---: | ---: |
| 泛型开启 vs 关闭 | +120 | +75 | -33 |
| 单个 EH 正常循环 AOT vs 整方法解释 | +506 | +488 | +229 |

上表是整个测试程序的制品差值，含共享运行时，不含外部 BCL、IL 和资源。gzip 的微小变化不代表通用压缩收益。
此前这些方法整方法解释执行，所以新增 AOT 覆盖并不应预期比该基线更小。
本轮没有实现“完整原生 EH”对照，尚不能给出相对完整原生 EH 的代码缩减百分比。
当前结构避免生成原生 catch 和异常展开逻辑，同时仍需保留原始 IL、元数据及解释器代码。

## 接入与复现

运行时内部注册表新增泛型实例身份字段，特化函数增加上下文参数。
升级时必须一起重建运行时并重新生成所有 AOT 代码，旧的预编译注册表与新内部布局不具有二进制兼容性。
公共 C API 和生产 Result 协议未变化。部署时保留用于异常接管的原始 IL 和元数据。

```powershell
python scripts/test/aot-features.py --target all --emsdk D:/workspace/wasmclr/emsdk
python scripts/test/aot-features.py --target native --config Debug
python scripts/test/aot-features.py --target native --arch Win32

python scripts/benchmarks/run_aot.py --build-only --output out/performance/feature-aot --emsdk D:/workspace/wasmclr/emsdk
python scripts/benchmarks/run_aot.py --build-only --output out/performance/feature-control --max-generic-instances 0 --emsdk D:/workspace/wasmclr/emsdk
python scripts/benchmarks/compare_aot_features.py --baseline-build out/performance/feature-aot --control-build out/performance/feature-control --wasm
```

功能测试细节见 [泛型测试](../../src/tests/generic-aot/README.md) 和 [异常测试](../../src/tests/eh-aot/README.md)。
