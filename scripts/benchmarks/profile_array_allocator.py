"""Count scaling and separate Win64 process-memory probes for NewArray.

The memory probes are not included in latency summaries: they poll the child
process and report the kernel's peak working set and observed private bytes.
"""
import argparse
import ctypes
import json
import random
import statistics
import subprocess
import time
from pathlib import Path
from run_protocols import ROOT, invoke


class ProcessMemoryCounters(ctypes.Structure):
    _fields_ = [('cb', ctypes.c_ulong), ('PageFaultCount', ctypes.c_ulong)] + [
        (name, ctypes.c_size_t) for name in ['PeakWorkingSetSize', 'WorkingSetSize',
            'QuotaPeakPagedPoolUsage', 'QuotaPagedPoolUsage', 'QuotaPeakNonPagedPoolUsage',
            'QuotaNonPagedPoolUsage', 'PagefileUsage', 'PeakPagefileUsage', 'PrivateUsage']]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--baseline', type=Path, default=ROOT / 'out/performance/il-families-optimized')
    ap.add_argument('--output', type=Path, default=ROOT / 'out/performance/il-array-allocator')
    opts = ap.parse_args()
    before, out = opts.baseline.resolve(), opts.output.resolve()
    k32 = ctypes.windll.kernel32
    k32.GetCurrentProcess.restype = ctypes.c_void_p
    k32.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
    k32.K32GetProcessMemoryInfo.argtypes = [ctypes.c_void_p, ctypes.POINTER(ProcessMemoryCounters), ctypes.c_ulong]
    if not k32.SetProcessAffinityMask(k32.GetCurrentProcess(), 4):
        raise ctypes.WinError()

    def command(engine, n):
        if engine == 'il2cpp':
            return [out / 'il2cpp/AotBaseline.exe', 23, n]
        base = before if engine == 'before-aot' else out
        return [base / 'native/Release/il-aot.exe', out / 'managed', 'NewArray', n, 19]

    rng = random.Random(20261114)
    counts = [65536, 262144, 1048576, 4194304]
    jobs = [(sample,e,n) for sample in range(3) for e in ['before-aot','aot','il2cpp'] for n in counts]
    rng.shuffle(jobs)
    scaling = []
    for sample, engine, n in jobs:
        text, _ = invoke(command(engine,n), out / ('scaling-%d-%s-%d.jsonl' % (sample,engine,n)))
        row = next(json.loads(s) for s in text.splitlines() if s.startswith('{'))
        scaling.append(dict(row, engine=engine, sample=sample))
    for n in counts:
        assert len({r['checksum'] for r in scaling if r['iterations']==n}) == 1
    (out / 'allocator-scaling.json').write_text(json.dumps(scaling, indent=2))
    print('NewArray scaling passed: 36 samples', flush=True)

    memory = []
    jobs = [(sample,e) for sample in range(3) for e in ['before-aot','aot']]
    rng.shuffle(jobs)
    for sample, engine in jobs:
        n = 2000000
        proc = subprocess.Popen([str(x) for x in command(engine,n)], cwd=ROOT, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, text=True, creationflags=subprocess.CREATE_NO_WINDOW)
        handle = ctypes.c_void_p(int(proc._handle))
        if not k32.SetProcessAffinityMask(handle, 4):
            raise ctypes.WinError()
        peak_working_set = peak_private = peak_commit = polls = 0
        while True:
            counters = ProcessMemoryCounters()
            counters.cb = ctypes.sizeof(counters)
            if k32.K32GetProcessMemoryInfo(handle, ctypes.byref(counters), counters.cb):
                polls += 1
                peak_working_set = max(peak_working_set, counters.PeakWorkingSetSize)
                peak_private = max(peak_private, counters.PrivateUsage)
                peak_commit = max(peak_commit, counters.PeakPagefileUsage)
            if proc.poll() is not None:
                break
            time.sleep(.001)
        stdout, _ = proc.communicate()
        (out / ('memory-%d-%s.jsonl' % (sample,engine))).write_text(stdout)
        assert proc.returncode == 0 and polls > 0 and peak_working_set > 0
        record = next(json.loads(s) for s in stdout.splitlines() if s.startswith('{'))
        assert record['gc_count_delta'] == 0 and record['used_bytes_delta'] == 96*n+24
        memory.append(dict(engine=engine, sample=sample, iterations=n, checksum=record['checksum'],
                           gc_count_delta=record['gc_count_delta'], used_bytes_delta=record['used_bytes_delta'],
                           peak_working_set_bytes=peak_working_set, observed_peak_private_bytes=peak_private,
                           peak_pagefile_bytes=peak_commit, successful_polls=polls))
    assert len({r['checksum'] for r in memory}) == 1
    result = dict(samples=memory, logical_cpu=2, poll_seconds=.001,
                  notes='Separate memory-only runs; polling latency is excluded from timing datasets.')
    (out / 'allocator-memory.json').write_text(json.dumps(result, indent=2))
    for engine in ['before-aot','aot']:
        print(engine, {field: statistics.median(r[field] for r in memory if r['engine']==engine)
                       for field in ['peak_working_set_bytes','observed_peak_private_bytes']})


if __name__ == '__main__':
    main()
