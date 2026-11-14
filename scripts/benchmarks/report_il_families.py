"""Publish the Win64 same-IL measurements and their reproducibility evidence."""
import argparse
import csv
import json
import math
import shutil
from pathlib import Path
from run_protocols import ROOT
from run_il_families import NAMES

FAMILIES = {
    'Int32': '32 位整数 / 位运算：mul, add, xor, shr.un',
    'Int64': '64 位整数：mul, add, xor, shr.un, conv.u8',
    'DivRem': '可变无符号除数：div.un, rem.un',
    'Float32': 'float32：mul, add, conv.r4',
    'Float64': 'float64：mul, add, conv.r8',
    'Convert': '整数/浮点/截断转换：conv.i4/u4/u2/r8/r.un',
    'Checked': '无溢出正常路径：mul.ovf.un, add.ovf.un, conv.ovf.u4',
    'Branch': '数据相关条件分支：and, brtrue/brfalse, shr.un',
    'Switch': '8 路 switch 跳转表及整数计算',
    'ArrayLoad': '1K uint 数组依赖读：ldelem.u4, 边界检查',
    'ArrayStore': '1K uint 数组读改写：ldelem.u4, stelem.i4',
    'Field': '对象实例字段读改写：newobj, ldfld, stfld',
    'StaticField': '静态字段读改写：ldsfld, stsfld',
    'ByRef': 'ref 参数：ldarga.s, call, ldind.u4, stind.i4',
    'Calls': '禁止内联的静态 call / ret，携带整数结果',
    'Virtual': '两个接收者类型交替的虚调用 callvirt',
    'Interface': '两个实现类型交替的接口 callvirt',
    'Delegate': '缓存 Func<uint,uint> 的 Invoke / 间接调用',
    'Struct': '16 字节值类型按值传参及返回：initobj, ldfld, stfld, call',
    'Generic': 'Identity<uint> 闭合泛型调用，禁止内联',
    'Cast': '成功 castclass + isinst，两个类型交替',
    'Boxing': '每迭代 box + unbox.any，对象通过禁止内联函数逃逸',
    'Allocation': '每迭代 newobj，一个字段的对象逃逸',
    'NewArray': '每迭代 newarr uint[16] + ldlen + 元素读写，数组逃逸',
    'EhNormal': 'try/finally 正常路径：leave, endfinally，不抛异常',
    'EhThrow': 'try/catch/finally：每 1024 迭代 throw 一次（含 i=0）',
}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--input', type=Path, default=ROOT / 'out/performance/il-families')
    opts = ap.parse_args()
    src = opts.input.resolve()
    result = json.loads((src / 'results.json').read_text())
    data = ROOT / 'docs/performance/data/il-families-win64'
    data.mkdir(parents=True, exist_ok=True)
    for name in ['results.json', 'build-manifest.json', 'il-inventory.tsv']:
        shutil.copy2(src / name, data / name)
    shutil.copy2(src / 'generated/generic-aot-report.json', data / 'generic-aot-report.json')
    shutil.copy2(src / 'managed/AotBaseline.runtimeconfig.json', data / 'runtimeconfig.json')
    oracles = {}
    for path in src.glob('oracle-*.jsonl'):
        for line in path.read_text().splitlines():
            if line.startswith('{'):
                record = json.loads(line)
                key = (record['workload'], record['iterations'])
                oracles[key] = dict(workload=key[0], iterations=key[1], checksum=record['checksum'])
    assert all(r['checksum'] == oracles[r['workload'], r['iterations']]['checksum'] for r in result['samples'])
    (data / 'oracles.json').write_text(json.dumps(list(oracles.values()), indent=2), encoding='utf-8')
    inventory = {}
    for line in (src / 'il-inventory.tsv').read_text(encoding='utf-8-sig').splitlines():
        method, offset, op, target = line.split('\t')
        entry = inventory.setdefault(method, dict(opcodes={}, calls=[]))
        entry['opcodes'][op] = entry['opcodes'].get(op, 0) + 1
        if target and target not in entry['calls']:
            entry['calls'].append(target)
    (data / 'il-inventory.json').write_text(json.dumps(inventory, indent=2), encoding='utf-8')
    with (data / 'summary.csv').open('w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=list(result['summary'][0]))
        writer.writeheader()
        writer.writerows(result['summary'])
    rows = {(r['workload'], r['engine']): r for r in result['summary']}
    sample_count = len({r['sample'] for r in result['samples']})
    total_samples = 26 * 4 * sample_count
    assert len(result['samples']) == total_samples
    assert all(rows[n, 'interp']['entry'] == 'interpreter' for n in NAMES)
    assert all(rows[n, 'aot']['entry'] == 'aot' for n in NAMES)
    assert all(not rows[n, 'aot']['interpreted_ir'] for n in NAMES if n != 'EhThrow')
    ns = lambda n, e: rows[n, e]['median_ns']
    pure = [n for n in NAMES if n not in ('EhThrow', 'Delegate')]
    geo = lambda a, b, names: math.exp(sum(math.log(ns(n, a) / ns(n, b)) for n in names) / len(names))
    widest = max(result['summary'], key=lambda r: (r['max_ns'] - r['min_ns']) / r['median_ns'])
    outliers = '；'.join('%s/%s 为 %.3f～%.3fns' % (e, n, rows[n,e]['min_ns'], rows[n,e]['max_ns'])
                         for e, n in [('net10', 'Boxing'), ('aot', 'Switch'), ('il2cpp', 'EhThrow')])
    text = f'''# Win64 常见 IL 指令族性能实测

测量日期：2026-11-14。代码基线：`{result['commit']}`，另加本报告配套基准文件，未改运行时实现。

## 主要结论

- 26 个用例、4 个后端、每项 {sample_count} 次，共 {total_samples} 个正式样本；相同输入验证与每个实测迭代数的 .NET 10 参考结果校验全部通过。
- 排除委托桥接与抛异常混合路径，24 项等权几何平均：LeanAOT 比解释器快 **{geo('interp','aot',pure):.2f}×**；LeanAOT 耗时为 IL2CPP 的 **{geo('aot','il2cpp',pure):.2f}×**、.NET 10 JIT 的 **{geo('aot','net10',pure):.2f}×**。这是此用例集合的汇总，不能当作实际应用加速比。
- 整数、浮点、数组与普通字段循环中，LeanAOT 已与 IL2CPP/.NET JIT 接近；动态类型检查、委托、分配以及异常恢复是更明显的差距来源。
- `EhThrow` 首次迭代就抛异常；LeanAOT 在首次异常后由解释器完成本次方法的剩余执行，因此此项是 **AOT 入口 + 解释器续执行**，不是纯 AOT 异常性能。

## 环境和配置

| 项目 | 实测配置 |
|---|---|
| CPU | 13th Gen Intel Core i5-13600KF；所有采样子进程继承 affinity mask `{result['affinity_mask']}`（逻辑 CPU 2） |
| OS | {result['platform']}，Win64 |
| Native 工具链 | Visual Studio 2022，MSVC 19.44.35230.0，Windows SDK 10.0.26100.0 |
| LeanCLR | Release x64，MSVC `/O2 /Ob2 /DNDEBUG`；Mono 4.5 BCL；MARK_SWEEP；GC_DEBUG=OFF；PROFILE=OFF |
| LeanCLR 解释器 | 独立 host 未注册任何 AOT 模块；入口检查为 Interpreter；BCL 内建/native icall 仍按正常运行时路径执行 |
| LeanAOT | 默认 Result 返回协议，未开启轻量返回、接口缓存、PGO 或泛型引用共享；只 AOT 基准程序集，BCL 非全量 AOT |
| IL2CPP | Unity **2022.3.62f3** 自带 UnityLinker + il2cpp，WindowsDesktop x64 Release，Unity BCL/BDWGC；null/bounds/divide-zero 检查开启；stacktrace 开启 |
| .NET | SDK 10.0.204；运行时 **Microsoft.NETCore.App 10.0.8 x64**，runtimeconfig 禁止版本滚动；`DOTNET_TieredCompilation=0`、`DOTNET_ReadyToRun=0`，直接使用优化 JIT，无 tiering/动态 PGO |

IL2CPP 是独立 CLI 可执行程序，不是 Unity Player。安装包缺失 `Libil2cppLackey/DllMain.cpp`，复用了仓库已有的线程回调兼容 helper 并重放链接步骤；未修改 Unity 安装、生成的方法实现或优化选项。构建命令及 helper 标记见 [build-manifest.json](data/il-families-win64/build-manifest.json)。

## 测量方法

1. Framework CSC `/optimize+ /nostdlib+` 只编译一次，四后端使用同一个 `AotBaseline.dll` 输入；IL2CPP 随后经过其正常 UnityLinker 流程。输入 SHA-256：`{result['managed_sha256']}`。
2. 每次采样启动新进程、固定同一个逻辑 CPU，以 seed=19 预热 3 次（每次最多 16384 迭代）；类型初始化和首次方法翻译/JIT 位于计时外。
3. 每后端/用例先用 65536 迭代校准，目标约 100ms，迭代数限制为 16384～100000000。各后端可使用不同迭代数，测量的是稳态单位迭代成本；精确迭代数保存在 CSV/JSON。超快用例受上限限制，可能不足 100ms。
4. 先在 n=4096 对四后端做一致性检查；每个实际 n 再生成 .NET 10 参考校验值。{total_samples} 个正式样本均逐项验证，未只校验最后一次结果。
5. {sample_count} 轮采样，每轮随机打乱全部 104 个组合（固定随机种子 20261114），串行执行。报告使用中位数；CSV 保留 min/median/max，JSON 保留全部原始 ns/iteration、checksum 和执行后端标记。
6. LeanCLR host 用 `steady_clock` 计整个 kernel 调用，另两者用 `Stopwatch.GetTimestamp` 计 Dispatch/kernel。每次仅一次外层调用，含微小调用/返回成本，不含进程启动、输出和验证。数字越小越快。

这不是单条 IL 延迟测试：每迭代包含循环控制、多条指令和必要运行时检查；优化器可消除冗余操作、把字段提升到寄存器或化简类型检查。数组读为 4KiB 热数据、依赖索引；不是内存带宽测试。字段用例重复同一对象；不是随机对象图遍历。分支由确定性状态驱动，不代表所有分支预测分布。

## 全部结果

单位：**ns / 循环迭代**，{sample_count} 次中位数。`解释/AOT` 大于 1 表示 AOT 更快；`AOT/IL2CPP`、`AOT/JIT` 大于 1 表示 AOT 更慢。除 `EhThrow*` 外 AOT 入口均未创建解释器 IR；`Delegate†` 有运行时委托桥接。

| 用例 | 解释器 | LeanAOT | IL2CPP | .NET 10 JIT | 解释/AOT | AOT/IL2CPP | AOT/JIT |
|---|---:|---:|---:|---:|---:|---:|---:|
'''
    for name in NAMES:
        a, b, c, d = [ns(name, e) for e in ('interp', 'aot', 'il2cpp', 'net10')]
        label = name + ('*' if name == 'EhThrow' else '†' if name == 'Delegate' else '')
        text += f'| {label} | {a:.3f} | {b:.3f} | {c:.3f} | {d:.3f} | {a/b:.2f}× | {b/c:.2f}× | {b/d:.2f}× |\n'
    text += f'''
离散程度：全体组合中最大的 `(max-min)/median` 为 **{100*(widest['max_ns']-widest['min_ns'])/widest['median_ns']:.1f}%**，出现在 `{widest['engine']}/{widest['workload']}`。完整范围请看 [summary.csv](data/il-families-win64/summary.csv)，不要把小于噪声的细小差值解读为确定性优势。未做显著性检验或跨日期重复。

本轮有明显慢样本，全部保留，未删除离群值：{outliers}。中位数降低了孤立慢样本的影响，但不能排除后台调度、频率变化和 GC 的影响；接近 1× 的比值应理解为本轮相近，不作严格排名。

## 用例覆盖与设计

所有 kernel 的 n 与 seed 均为运行时参数，返回结果进入输出和跨后端校验。NoInlining 辅助方法隔离 call、byref、struct、generic 与逃逸分配成本。虚/接口调用交替两个实现，避免单一接收者完全去虚化。分配用例的对象传给禁止内联的 Keep 并写入静态引用；每次调用都进行可观察的逃逸，旧对象随后可回收。**后续计数器诊断确认：当前 LeanCLR host 的循环未自动触发 GC，因此 LeanCLR 分配项测的是持续分配和堆增长，不能视为 GC 稳态吞吐；原先“包含自然触发 GC”的表述已更正。**详见 [三项热点分析](il-hotspots-win64.md)。

| 用例 | 主要 IL / 操作 |
|---|---|
'''
    for name in NAMES:
        text += f'| {name} | {FAMILIES[name]} |\n'
    text += f'''
实际程序集已用 `ILInventory.cs` 解码，共 {len(inventory)} 个有方法体的方法、{len(set(op for m in inventory.values() for op in m['opcodes']))} 种 opcode（包含 harness 和辅助方法，并非全部都是热循环指令）。[完整 IL 清单](data/il-families-win64/il-inventory.tsv) 记录方法、IL 偏移、opcode 与方法调用目标；[指令统计](data/il-families-win64/il-inventory.json) 可核对 C# 实际生成的 IL，例如 Switch 确实含 `switch`、Checked 确实含 `mul.ovf.un/add.ovf.un`、BoxValue 含 `box`。

未覆盖冷启动、SIMD、线程/原子/volatile、unsafe/calli/cpblk、P/Invoke、反射、字符串算法、大对象堆/大工作集、完整 BCL 性能及真实 Unity 场景；不据此推导整体帧率、完整发布体积或内存占用。

## 结果解读与优化方向

- `Int32/Int64/Float32/Float64`：体现解释循环与本机优化计算的差距；三种本机后端的小差值需要结合样本范围看待。
- `Calls/Generic`：LeanAOT 的闭合 `Identity<uint>` 在 [生成报告](data/il-families-win64/generic-aot-report.json) 中确认有 AOT 方法体；此处没有把泛型解释器回退记为 AOT。标量调用相近，不能外推到复杂 ABI；16 字节 Struct 单列测量。
- `Cast`：LeanAOT 对成功类型检查的成本明显更高，是后续检查 castclass/isinst helper 与检查合并的候选；这只是由本用例定位的方向，尚无 profile/汇编证据证明具体根因。
- `Virtual/Interface/Delegate`：测量调用分派和辅助检查，而非只测乘加。Delegate 生成报告中的 `Func.Invoke unsupported-body` 表示走运行时桥接，不能据此直接称为解释执行。它不属于上述 24 项常规用例汇总。
- `Boxing/Allocation/NewArray`：.NET 10 在本轮分配用例中表现明显更好；各后端分配器、GC 触发方式、零初始化与对象布局不同，尤其 LeanCLR 循环中未自动 GC。应继续用分配吞吐/GC pause/内存数据拆分，不能把全部差距归因于 `newobj` 代码生成，也不能直接外推到长时间 GC 稳态。
- `EhNormal/EhThrow` 必须分开：前者在 AOT 中无解释 IR；后者有解释 IR。生成代码在错误出口调用 `Interpreter::resume_exception` 后直接返回其结果；由于 i=0 就抛出，剩余循环由解释器执行。因此抛异常项接近纯解释器，不能理解为每 1024 次只付出一次短暂桥接成本。

以上只评估默认 LeanAOT 配置；接口缓存、PGO、返回协议开关和引用泛型共享均可另做独立 A/B，不能在未运行时假定其加速。

## 复现

在仓库根目录、安装相同工具链的 Windows x64 环境中执行：

```powershell
python scripts/benchmarks/run_il_families.py --unity D:/UnityHubs/2022.3.62f3
python scripts/benchmarks/report_il_families.py
```

已有可执行文件仅重测：

```powershell
python scripts/benchmarks/run_il_families.py --skip-build --samples 7 --target-ms 100
python scripts/benchmarks/report_il_families.py
```

需安装 .NET 10.0.8（runtimeconfig 精确固定版本）；原始构建日志、生成 C++、每次运行 stdout 和 oracle 结果在 `out/performance/il-families/`。可移植的正式样本、构建命令、IL 清单和生成器回退报告已复制到 [data/il-families-win64](data/il-families-win64/results.json)。正式数据中的 source_hashes 可检查配套文件；重新运行前应核对提交与本地修改，保持相同电源/后台负载条件。

未自动提交 Git。运行时核心代码未改动；新增 C++ host 已使用仓库 clang-format 与 .clang-format 格式化，并完成 Release 构建及实测验证。
'''
    (ROOT / 'docs/performance/il-families-win64.md').write_text(text, encoding='utf-8')
    print(ROOT / 'docs/performance/il-families-win64.md')


if __name__ == '__main__':
    main()
