"""Publish production AOT optimization measurements, correctness and memory evidence."""
import argparse
import csv
import hashlib
import json
import re
import shutil
import subprocess
from pathlib import Path
from run_protocols import ROOT
from run_il_families import NAMES


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--input', type=Path, default=ROOT / 'out/performance/il-families-optimized')
    opts = ap.parse_args()
    out = opts.input.resolve()
    result = json.loads((out / 'results.json').read_text())
    paired = json.loads((out / 'paired-results.json').read_text())
    assert len(paired['samples']) == 84
    for name in ['Cast', 'Delegate', 'NewArray']:
        assert len({r['checksum'] for r in paired['samples'] if r['workload'] == name}) == 1
        assert len({r['iterations'] for r in paired['samples'] if r['workload'] == name}) == 1
    rounds = len({r['sample'] for r in result['samples']})
    assert len(result['samples']) == 26 * 5 * rounds
    rows = {(r['workload'], r['engine']): r for r in result['summary']}
    native = [r for r in result['samples'] if r['engine'] in ('aot', 'interp')]
    assert all(r['gc_count_delta'] == 0 for r in native)
    assert all(r['used_bytes_delta'] == 96*r['iterations'] + 24 for r in native if r['workload'] == 'NewArray')
    assert all(rows[n, 'aot']['entry'] == 'aot' for n in NAMES)
    assert all(not rows[n, 'aot']['interpreted_ir'] for n in NAMES if n != 'EhThrow')
    oracles = {}
    for p in out.glob('oracle-*.jsonl'):
        r = next(json.loads(s) for s in p.read_text().splitlines() if s.startswith('{'))
        oracles[r['workload'], r['iterations']] = r['checksum']
    assert all(r['checksum'] == oracles[r['workload'], r['iterations']] for r in result['samples'])
    dest = ROOT / 'docs/performance/data/il-families-optimized-win64'
    dest.mkdir(parents=True, exist_ok=True)
    for name in ['results.json', 'paired-results.json', 'build-manifest.json', 'il-inventory.tsv']:
        shutil.copy2(out / name, dest / name)
    shutil.copy2(out / 'generated/generic-aot-report.json', dest / 'generic-aot-report.json')
    (dest / 'oracles.json').write_text(json.dumps([dict(workload=n, iterations=i, checksum=c) for (n,i),c in oracles.items()], indent=2))
    with (dest / 'summary.csv').open('w', newline='', encoding='utf8') as f:
        w = csv.DictWriter(f, fieldnames=list(result['summary'][0])); w.writeheader(); w.writerows(result['summary'])
    test_logs = ['out/aot-hotspots/default-test.log', 'out/aot-hotspots/protocols-test.log',
                 'out/aot-hotspots/shared-test.log', 'out/aot-hotspots-debug/default-test.log',
                 'out/regressions-hotspots/step-2.log']
    for name in test_logs:
        assert '100% tests passed' in (ROOT / name).read_text()
        target = name.replace('/', '-')
        shutil.copy2(ROOT / name, dest / target)
    changed = subprocess.check_output(['git', '-c', 'core.fsmonitor=false', 'diff', '--name-only'], cwd=ROOT, text=True).splitlines()
    sources = [ROOT / p for p in changed if p.endswith(('.cpp','.h','.cs'))]
    sources += list((ROOT / 'src/tests/aot-hotspots').glob('*'))
    hashes = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources if p.is_file()}
    generated = '\n'.join(p.read_text() for p in (out / 'generated').glob('*.cpp'))
    caches = len(re.findall(r'leanclr::vm::SzArrayAllocationCache\s+\w+\s*;', generated))
    bridge_count = len(re.findall(r'leanclr_generated_direct_call_bridge_[a-f0-9]+_delegate\([^;{}]*\)\s*noexcept\s*\{', generated))
    (dest / 'source-hashes.json').write_text(json.dumps(dict(source_hashes=hashes, array_cache_count=caches,
                                                          win64_cache_bytes=caches*16, delegate_bridge_count=bridge_count), indent=2))
    ns = lambda n,e: rows[n,e]['median_ns']
    paired_rows = {(r['workload'],r['engine']):r for r in paired['summary']}
    paired_ns = lambda n,e: paired_rows[n,e]['median_ns']
    manifest = json.loads((out / 'build-manifest.json').read_text())
    executables = [v for k,v in manifest['artifact_bytes'].items() if k.replace('\\','/').endswith('/il-aot.exe')]
    libraries = [v for k,v in manifest['artifact_bytes'].items() if k.replace('\\','/').endswith('/leanclr.lib')]
    text = f'''# Cast、Delegate、NewArray 正式优化与 Win64 复测

日期：2026-11-14。当前基础提交 `{result['commit']}` 加配套未提交优化。正式修改了 LeanAOT 生成器及 LeanCLR 辅助函数；没有使用诊断版 probes.h 替换生成代码。原 [基准](il-families-win64.md) 与 [原因分析](il-hotspots-win64.md) 保留为优化前记录。

## 收益

相同托管输入、相同机器和工具链，串行随机重测解释器、正式优化 AOT、IL2CPP、.NET 10，以及保留的旧 AOT，共 **{len(result['samples'])} 个全量样本**（26 项 × 5 后端 × {rounds} 轮）。另补 **84 个等迭代数对照样本**，验证分配总量不同不会误导收益判断。每个实测迭代数均与 .NET 参考 checksum 一致，未删除离群值。

| 用例 | 同轮旧 AOT | 正式优化 AOT | 加速 | IL2CPP | .NET 10 JIT |
|---|---:|---:|---:|---:|---:|
'''
    for n in ['Cast','Delegate','NewArray']:
        text += f'| {n} | {paired_ns(n,"before-aot"):.3f} | {paired_ns(n,"aot"):.3f} | {paired_ns(n,"before-aot")/paired_ns(n,"aot"):.2f}× | {paired_ns(n,"il2cpp"):.3f} | {paired_ns(n,"net10"):.3f} |\n'
    text += f'''
单位 ns/循环迭代，{rounds} 次中位数，越低越好。此表使用四后端等迭代数对照：Cast/Delegate 为 2000 万次，NewArray 为 200 万次；完整 [paired-results.json](data/il-families-optimized-win64/paired-results.json) 保留范围和逐次结果。正式实现保留了运行时 guard、各种返回 ABI 和慢路径，因此不能套用之前限定场景诊断原型的数字。

## 实现和正确性

- **Cast**：生成器仅对确定的非泛型普通引用类特化；sealed 类比较 klass，普通类复用 hierarchy_depth/super_types 的 O(1) 检查。未初始化父类表回退完整检查；接口、数组协变、泛型参数/实例、boxed 值和 Nullable 保留既有语义。不建立源/目标类型组合哈希缓存。
- **Delegate**：按 ABI 签名共享的专用 bridge，优化单播开放静态及封闭引用实例 AOT 目标；按实际目标选择 generic MethodInfo 和 Result/lightweight/packed32/status-out ABI。返回类型类别不一致（如 enum 与底层整数）时回退实际 invoker，避免把 packed/status-out 误当作 Result ABI。多播、开放实例、bound static、值类型 this、解释器等情况也保留通用 invoker。null、错误返回和异常路径不绕过。当前基准仅生成 {bridge_count} 个专用委托 bridge；委托对象不增加字段。
- **NewArray**：每个生成方法的元素 token 共享一个有界缓存，记录完整 SZArray 类和元素尺寸；缓存命中直接做 checked 布局计算及原 GC 分配器调用。保留负长度、溢出/OOM、数据全零与分配站点记录。缓存初始化成功才发布；元素变化会重新解析，覆盖泛型共享场景。所有长度均复用同一项缓存，没有按长度建表。
- 边界测试还揭示并修复了通用委托回退中的两个已有缺陷：开放实例虚委托按实际接收者分派；静态委托目标在执行前完成延迟类初始化，初始化失败也正确传播并保持失败状态。

## 内存及 GC

`SzArrayAllocationCache` 在 Win64 为 16 字节，受 native static_assert 限制不超过两个指针宽度。本基准生成 {caches} 项，即 **{caches*16} 字节静态缓存**。没有新增每对象字段，也没有引入额外的每次调用堆分配；缓存不随数组长度增长。原生代码量因专用 bridge/内联辅助函数略增。

本次基准可执行文件从 **{executables[0]} → {executables[1]} 字节（+{executables[1]-executables[0]} 字节）**；静态运行时库从 **{libraries[0]} → {libraries[1]} 字节**。这些是本机构建产物，不是完整发布体积或运行时 RSS；精确散列和尺寸见 [构建清单](data/il-families-optimized-win64/build-manifest.json)。

新 host 在计时外记录 GC 次数和对象计费字节。全部新解释器/AOT 样本在 kernel 内的 GC 次数为 0，保持原测量语义；NewArray 的单位计费对象大小仍是 96 字节（另有 host 每次调用的 24 字节结果装箱）。优化没有通过延迟回收、取消清零或改变对象布局获取收益。

边界测试另外在 host 安全边界显式 GC：含引用 struct 数组及其嵌套对象跨回收存活，释放静态根后重复 8 轮的存活对象计费字节稳定。此检查针对有界存活量；不代表 RSS/arena 保留量已做全面分析。

本轮未更改自动 GC 时机、size-class allocator、arena 大小或地址范围索引。因此 NewArray 仍可能落后于 IL2CPP；分配器和长期 GC 稳态是独立后续工作，不能把当前结果外推到完整应用。

## 测试

- 同一 Fixtures.cs 在 .NET 10 参考执行、纯解释器和 AOT 执行；正向入口检查实际 AOT invoker，明确保留一个解释器委托目标。
- 默认 ABI、全部返回协议开关同时开启、引用泛型共享三组配置，以及 GC_DEBUG=ON：共 **8 项 CTest 全部通过**。
- 仓库完整运行时回归：**42/42 通过**，包括类型系统、弱引用/finalizer、分配失败、GC accounting、非法布局、公共 invoker/ABI 和层依赖。
- 新增边界：null/失败 cast、深继承/接口/方差/数组协变、泛型及 Nullable；多种委托绑定、重入、多播顺序/异常、null、byref、enum/void/reference/64位/浮点/含引用结构体返回、静态初始化；零/负/变长/不同元素数组、真实数组类型、零初始化、OOM 注入、计费不增长和失败恢复；显式 GC 存活与回收后有界对象量。
- 所有修改过的 C++ 源码和头文件已使用仓库 tools/clang-format/clang-format.exe 与 .clang-format 格式化，再构建验证。

额外用运行时直接构造的绑定验证 enum/底层整数的 32/64 位双向返回别名，覆盖 packed/status-out 保护。Mono BCL 当前的反射 CreateDelegate 校验仍拒绝该别名（.NET 接受）；本次没有扩展该反射 API，不能将 native 绑定测试解读为该 API 的兼容性已修复。

测试原始日志和源码散列随 [数据目录](data/il-families-optimized-win64/results.json) 保存；基准脚本对 {len(result['samples'])} 个结果重新校验 oracle、入口和解释 IR 状态。

## 全部 26 项

| 用例 | 解释器 | 旧 AOT | 新 AOT | 旧/新 AOT | IL2CPP | .NET 10 JIT |
|---|---:|---:|---:|---:|---:|---:|
'''
    for n in NAMES:
        text += f'| {n} | {ns(n,"interp"):.3f} | {ns(n,"before-aot"):.3f} | {ns(n,"aot"):.3f} | {ns(n,"before-aot")/ns(n,"aot"):.2f}× | {ns(n,"il2cpp"):.3f} | {ns(n,"net10"):.3f} |\n'
    widest = max(result['summary'], key=lambda r: (r['max_ns']-r['min_ns'])/r['median_ns'])
    text += f'''
最大 `(max-min)/median` 为 {100*(widest['max_ns']-widest['min_ns'])/widest['median_ns']:.1f}%（{widest['engine']}/{widest['workload']}）。完整 min/median/max 在 [summary.csv](data/il-families-optimized-win64/summary.csv)，原始单样本在 JSON；接近 1× 的比值应按噪声范围解释，不作严格排名。EhThrow 仍是首次异常后解释器续执行。

## 口径与复现

保持 Win64 Release、逻辑 CPU 2、seed=19、3 次预热、每组目标约 100ms、每轮随机顺序。各后端自动校准自己的迭代数；结果按迭代归一化。输入 DLL SHA-256 仍为 `{result['managed_sha256']}`，IL2CPP 和 .NET 使用相同基准输入，IL2CPP 产物原样复用。IL2CPP 是 Unity 2022.3.62f3 的独立 CLI，.NET 10.0.8 JIT 关闭 tiering/ReadyToRun；详细边界沿用原报告。

旧 AOT 产物对应原始基准提交；当前基础提交之间另有字符串/元数据对齐读取修复，这些不在三项 kernel 的热循环内。构建清单记录两侧产物散列，避免把历史不同轮次的测量当作同轮控制。生产修改及测试源码散列另存 [source-hashes.json](data/il-families-optimized-win64/source-hashes.json)。

```powershell
python scripts/test/aot-hotspots.py --all-protocols
python scripts/test/aot-hotspots.py --gc-debug
python scripts/benchmarks/run_il_optimized.py
python scripts/benchmarks/report_il_optimized.py
```

比较驱动要求保留原 `out/performance/il-families/`，输出到独立目录 `out/performance/il-families-optimized/`；不覆盖原基准。原始生成代码、构建及逐次采样日志保留在 out，正式 JSON/CSV、构建清单、IL 清单和验证日志已复制到 docs/performance/data。
'''
    (ROOT / 'docs/performance/il-hotspots-optimized-win64.md').write_text(text, encoding='utf8')
    print(ROOT / 'docs/performance/il-hotspots-optimized-win64.md')


if __name__ == '__main__':
    main()
