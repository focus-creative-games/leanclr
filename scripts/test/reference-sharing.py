"""Archive canonical reference sharing and specialized controls across return ABIs."""
import argparse
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--emsdk', type=Path, required=True)
    opts = ap.parse_args()
    output = ROOT / 'out/reference-sharing-matrix'
    output.mkdir(parents=True, exist_ok=True)
    cases = [
        ('shared-combined', ['--share-reference-generics', '--status-out-returns', '--packed-int32-returns', '--lightweight-returns', '--target', 'all', '--emsdk', str(opts.emsdk), '--output', 'out/reference-sharing-validation']),
        ('specialized', ['--target', 'all', '--emsdk', str(opts.emsdk)]),
        ('shared-debug', ['--share-reference-generics', '--config', 'Debug', '--output', 'out/reference-sharing-debug']),
        ('shared-win32', ['--share-reference-generics', '--arch', 'Win32', '--output', 'out/reference-sharing-win32']),
    ]
    rows = []
    for name, arguments in cases:
        print('Reference sharing:', name, flush=True)
        with (output / (name + '.log')).open('w') as stream:
            subprocess.run([sys.executable, 'scripts/test/global-generics.py'] + arguments, cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT, check=True)
        source = ROOT / (arguments[arguments.index('--output') + 1] if '--output' in arguments else 'out/global-generics')
        source /= 'lightweight' if '--lightweight-returns' in arguments else 'result'
        rows.append(dict(name=name, arguments=arguments,
                         reports={variant: json.loads((source / variant / 'generic-aot-report.json').read_text()) for variant in ['default', 'disabled']}))
    (output / 'validation.json').write_text(json.dumps(rows, indent=2) + '\n')


if __name__ == '__main__':
    main()
