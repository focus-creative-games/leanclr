"""Repeat resource contracts; retain raw samples, medians, binary sizes and heap metrics."""
import argparse
import json
from pathlib import Path
import statistics
import subprocess


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--runner', type=Path, required=True)
    p.add_argument('--baseline', type=Path)
    p.add_argument('--runtime', required=True)
    p.add_argument('--node', action='store_true')
    p.add_argument('--samples', type=int, default=5)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    if a.samples < 3: p.error('Use at least three samples')
    result = {}
    for count in [1024, 4096, 16384]:
        data = {'current': [], 'baseline': []}
        for _ in range(a.samples):
            for label, runner in [('baseline', a.baseline), ('current', a.runner)]:
                if not runner: continue
                command = (['node'] if a.node else []) + [str(runner), str(count), '100000', a.runtime]
                output = subprocess.check_output(command, text=True)
                data[label].append(json.loads(next(line for line in output.splitlines() if line.startswith('{'))))
        result[str(count)] = {}
        for label, rows in data.items():
            if not rows: continue
            result[str(count)][label] = {'samples': rows, 'median': {
                key: statistics.median(row[key] for row in rows) for key in rows[0] if key != 'checksum'}}
    sizes = {a.runner.name: a.runner.stat().st_size}
    if a.node: sizes[a.runner.with_suffix('.wasm').name] = a.runner.with_suffix('.wasm').stat().st_size
    if a.baseline: sizes['baseline'] = a.baseline.stat().st_size
    a.output.parent.mkdir(parents=True, exist_ok=True)
    a.output.write_text(json.dumps({'binary_bytes': sizes, 'results': result}, indent=2) + '\n', encoding='utf-8')
    for count, rows in result.items():
        print(count, {label: row['median'] for label, row in rows.items()})


if __name__ == '__main__': main()
