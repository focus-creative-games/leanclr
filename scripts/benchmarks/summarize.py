"""Publish compact review tables from the measured P0/P1 artifacts."""
import hashlib
import json
import platform
import re
from pathlib import Path
from run_protocols import ROOT


def row(data, protocol, workload, period=0):
    return next(r for r in data['summary'] if r['protocol'] == protocol and r['workload'] == workload and r['error_period'] == period)


def main():
    inputs = ['wasm-O2', 'native-O2', 'wasm-O3-lto', 'wasm-Os', 'aot']
    datasets = {}
    destination = ROOT / 'docs/performance/data'
    destination.mkdir(parents=True, exist_ok=True)
    for label in inputs:
        directory = ROOT / 'out/performance' / label
        data = json.loads((directory / 'results.json').read_text())
        compact = {k: v for k, v in data.items() if k != 'samples'}
        family = 'aot-baseline' if label == 'aot' else 'result-abi'
        compact['source_hashes'] = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                                    for p in sorted((ROOT / 'src/benchmarks' / family).glob('*')) if p.is_file()}
        compact['cpu'] = platform.processor()
        compact['result_header_sha256'] = hashlib.sha256((ROOT / 'src/runtime/core/rt_result.h').read_bytes()).hexdigest()
        compact['recorded_samples'] = len(data['samples'])
        compact['raw_samples'] = f'out/performance/{label}/results.json'
        compact['compiler'] = []
        for path in directory.glob('build/CMakeFiles/*/CMakeCXXCompiler.cmake'):
            text = path.read_text()
            compact['compiler'].append(dict(re.findall(r'set\((CMAKE_CXX_COMPILER(?:_ID|_VERSION)?) "([^"]*)"\)', text)))
        (destination / f'{label}.json').write_text(json.dumps(compact, indent=2), encoding='utf-8')
        datasets[label] = data
    out = ['# P0/P1：返回协议与 LeanAOT 性能基线', '',
           '本轮只新增独立实验、基准及复现脚本，未改变生产运行时 ABI 或 LeanAOT 生成规则。', '',
           '## 测量条件', '',
           '- Windows x64；MSVC 19.44.35229；Wasm 使用 Emscripten/Clang 22.0.0，Node v22.12.0。',
           '- 每配置 7 轮，通常每项 5,000,000 次；连续失败项限制为 20,000 次。记录中位数、最小值与最大值。',
           '- 编译完成后串行测量，按固定随机种子交错协议/引擎顺序；每个进程先校验并预热。未锁定 CPU 频率或排除所有系统背景负载。',
           '- 表中时间单位 ns/op，越小越好；这是具体微基准，不代表游戏整体提速。',
           '- Wasm 是 Node 中的结果，尚无 IL2CPP/WebGL 端到端结果，也不代表浏览器或小游戏平台。', '',
           '## P1：非尾调用返回协议', '',
           '五种负载分别为 uint32、uint64、16 字节结构体、可空且对齐的指针、真实 ResultVoid。',
           '每条链有三层传播者和叶子；成功返回后继续计算，避免仅凭尾调用消除获得收益。',
           '独立叶子防止整个计算消失；校验完整 32 位取值边界、成功 null、全部失败及混合失败。', '',
           '| 协议 | u32 | u64 | struct16 | pointer | void | Wasm 字节 | gzip 字节 |',
           '| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |']
    data = datasets['wasm-O2']
    for name in ['current', 'trivial', 'scalar', 'status', 'context', 'eh', 'direct']:
        values = [f'{row(data,name,w)["median_ns"]:.3f}' for w in ['u32-chain','u64-chain','struct16-chain','pointer-chain','void-chain']]
        artifact = data['artifacts'][name]['wasm']
        out.append('| ' + ' | '.join([name] + values + [str(artifact['bytes']),str(artifact['gzip_bytes'])]) + ' |')
    out += ['', '- current：仓库当前 Result；trivial：独立平凡包装原型；scalar：u32 打包到 u64，指针使用 tagged word，宽值/结构体回退到平凡包装。',
            '- status：错误码 + 输出参数；context：原生返回值 + 每次调用独立的错误上下文；eh：C++/Wasm EH；direct：无失败协议的参考下界。',
            '- trivial 原型有确定初始化的 payload，布局也不同，因此它与 current 的时间差不能全部归因于平凡类型这一因素。',
            '- void 的 trivial/scalar/status 都使用标量 RtErr；其微小差异应按采样噪声看待。',
            '- 所有 Wasm 协议使用相同 EH 编译能力；仅 eh 路径实际抛异常。体积包括微基准与校验器，不能外推为完整运行时体积变化。',
            '- EH 原型只验证原生异常传播；未实现 CLR filter、finally、GC 根、堆栈追踪与解释器边界，不能直接用作产品异常后端。', '',
            '### 错误频率影响（Wasm O2 / u32）', '',
            '| 协议 | 成功 | 每 1024 次失败一次 | 连续失败 |', '| --- | ---: | ---: | ---: |']
    for name in ['current','trivial','scalar','status','context','eh']:
        values=[f'{row(data,name,"u32-chain",period)["median_ns"]:.3f}' for period in [0,1024,1]]
        out.append('| ' + ' | '.join([name]+values) + ' |')
    out += ['', '### 编译选项对照（u32 成功路径）', '',
            '| 配置 | current ns | scalar ns | status ns | context ns | EH ns | current Wasm 字节 | status Wasm 字节 |',
            '| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |']
    for label in ['wasm-O2','wasm-O3-lto','wasm-Os']:
        d=datasets[label]
        values=[f'{row(d,p,"u32-chain")["median_ns"]:.3f}' for p in ['current','scalar','status','context','eh']]
        values += [str(d['artifacts'][p]['wasm']['bytes']) for p in ['current','status']]
        out.append('| ' + ' | '.join([label]+values) + ' |')
    out += ['', 'O3 与 LTO 本轮作为组合配置测量，不能分别归因；所有类型和原始区间见 JSON 快照。', '',
            '### ABI 证据与建议', '',
            '- current_u32/current_pointer/current_void 在 LLVM IR 中使用隐藏 sret 返回地址。',
            '- trivial_pointer 返回 i32、scalar_void 返回 i8、packed_u32 返回 i64；trivial_u32 仍使用 sret。',
            '- 因此“缩小 Result”或“默认化移动构造”不等于对所有类型消除内存返回。',
            '- 建议优先试点 pointer/void 的平凡/标量协议；u32 可试点 packed scalar；宽值/结构体将 status+out 作为重点候选。',
            '- 不建议全局替换为平凡包装、context 或 EH。尤其 scalar 在宽类型上的回退可能变慢，需要按返回类型选择。',
            '- P1 是协议选择实验，尚未证明完整运行时或真实游戏获益；迁移前还要覆盖重入、托管异常、GC、OOM 与跨模块 ABI。', '',
            '## P0：同源托管基准', '',
            '同一 Benchmarks.cs 用于两种生成器，所有已测引擎的可观察校验值一致。',
            'IL2CPP 使用 Unity 2022.3.62f3 的 Unity BCL 和 UnityLinker；LeanAOT 使用仓库 Mono BCL。',
            'IL2CPP 开启 null、数组边界、除零和堆栈追踪；两套实际编译命令保存在构建清单。',
            '此对照反映各自当前部署配置，不能视为完全相同后端参数的纯代码生成实验。', '',
            '| 负载 | LeanAOT x64 | IL2CPP x64 | Lean/IL2CPP | LeanAOT Wasm | C++ Wasm 参考 | 执行路径/限制 |',
            '| --- | ---: | ---: | ---: | ---: | ---: | --- |']
    aot=datasets['aot']
    def get(engine,w):
        group=[r for r in aot['summary'] if r['engine']==engine and r['workload']==w]
        return group[0]['median_ns'] if group else None
    for w in ['arithmetic','calls','properties','array','interfaces','structs','generics','exceptions']:
        lean=get('native-leanaot',w);il2=get('native-il2cpp',w);wasm=get('wasm-leanaot',w);cpp=get('wasm-cpp',w)
        note={'calls':'NoInlining 处理不同，不作公平调用开销结论',
              'structs':'PairStep 的 NoInlining 处理不同',
              'generics':'外层 AOT；Identity<uint> 经桥接解释执行',
              'exceptions':'LeanCLR 解释执行；IL2CPP 编译执行'}.get(w,'AOT 入口已验证')
        out.append(f'| {w} | {lean:.3f} | {il2:.3f} | {lean/il2:.2f}x | {wasm:.3f} | {"—" if cpp is None else f"{cpp:.3f}"} | {note} |')
    out += ['', '### 优化顺序的调整', '',
            '1. **泛型实例 AOT 覆盖**：当前生成计划的 GenericMethodPlans 未覆盖本基准 Identity<uint>；先减少 AOT/解释器往返，再评估泛型本体的代码质量。',
            '2. **EH 方法编译覆盖**：Manifest 和规则候选层均跳过 HasExceptionHandlers；当前异常差距主要包含执行模式差异，不能只优化 Result 后期待解决。',
            '3. **接口分派**：是可以继续测量与优化的纯 AOT 热路径。',
            '4. **Result 热路径协议**：依据 P1 分类试点，保留普通运行时接口的错误语义。', '',
            '### 重要边界', '',
            '- 当前 LeanAOT 不将 MethodImplOptions.NoInlining 发射为对应 C++ 属性；IL2CPP 会发射。调用/结构体基准的“更快”不能据此认定优于 IL2CPP。',
            '- 手写 C++ 仅为计算参考，并非完整 CLR 语义等价物；内联和数组检查不同，所以也不是严格性能下界。',
            '- 本机 Unity 缺少独立 CLI 的 Libil2cppLackey 源码及默认 baselib 路径。复现脚本使用自有线程退出通知转发 DLL，并链接安装中真实 baselib；未改托管生成方法或 IL2CPP 运行时代码。它不是未经适配的官方 Player 构建。',
            '- 完整 IL2CPP/WebGL、浏览器/小游戏、真实项目负载、分配压力和独立冷启动时延仍未测量，不把这些标为已通过。',
            '- 进程 wall time 含预热、运行、校验与输出，不是启动时延；二进制大小不含外部 BCL/元数据/资源，不代表完整部署体积。', '',
            '## 复现', '', '从仓库根目录运行（现有工具链，无需下载）：', '', '```powershell',
            'python scripts/benchmarks/run_protocols.py --target native --samples 7 --iterations 5000000',
            'python scripts/benchmarks/run_protocols.py --target wasm --emsdk D:/workspace/wasmclr/emsdk --samples 7 --iterations 5000000',
            'python scripts/benchmarks/run_protocols.py --target wasm --emsdk D:/workspace/wasmclr/emsdk --opt 3 --lto --samples 7 --iterations 5000000',
            'python scripts/benchmarks/run_protocols.py --target wasm --emsdk D:/workspace/wasmclr/emsdk --opt s --samples 7 --iterations 5000000',
            'python scripts/benchmarks/run_aot.py --unity D:/UnityHubs/2022.3.62f3 --emsdk D:/workspace/wasmclr/emsdk --samples 7 --iterations 5000000',
            'python scripts/benchmarks/summarize.py', '```', '',
            '路径按本机调整。AOT 完整复现目前要求 Windows + .NET Framework CSC；协议实验可独立构建。',
            '原始样本、日志、LLVM IR 与命令位于 `out/performance/`；精简快照见 [data](data/)。',
            'CI 只校验协议语义，不对共享机器上的纳秒测量设阈值。']
    (ROOT / 'docs/performance/p0-p1.md').write_text('\n'.join(out)+'\n',encoding='utf-8')
    print(ROOT / 'docs/performance/p0-p1.md')


if __name__ == '__main__':
    main()
