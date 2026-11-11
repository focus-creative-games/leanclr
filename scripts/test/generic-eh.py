"""Validate generic EH admission, handoff and return-protocol combinations."""
import argparse
import hashlib
import json
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--emsdk', type=Path, required=True)
    opts = parser.parse_args()
    archive = ROOT / 'out/generic-eh-validation'
    archive.mkdir(parents=True, exist_ok=True)
    rows = []
    for flags in range(4):
        protocol = (['--lightweight-returns'] if flags & 1 else []) + (['--packed-int32-returns'] if flags & 2 else [])
        for configuration in ['all', 'Debug', 'Win32']:
            build_args = ['--target', 'all', '--emsdk', str(opts.emsdk)] if configuration == 'all' else ['--config', 'Debug'] if configuration == 'Debug' else ['--arch', 'Win32']
            destination = archive / f'flags-{flags}' / configuration
            destination.mkdir(parents=True, exist_ok=True)
            for suite in ['aot-features', 'global-generics']:
                print(f'Generic EH validation: flags={flags} {configuration} {suite}', flush=True)
                command = [sys.executable, str(ROOT / f'scripts/test/{suite}.py')] + protocol + build_args
                if suite == 'global-generics':
                    command += ['--interface-call-cache', '--interface-cache-entries', '2']
                process = subprocess.run(command, cwd=ROOT, text=True, encoding='utf-8', errors='replace', stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                (destination / f'{suite}.log').write_text(process.stdout, encoding='utf-8')
                if process.returncode:
                    raise RuntimeError(process.stdout[-4500:])
                cfg, arch = ('Debug' if configuration == 'Debug' else 'Release'), ('Win32' if configuration == 'Win32' else 'x64')
                targets = ['native', 'wasm'] if configuration == 'all' else ['native']
                root = ROOT / ('out/aot-features-lightweight' if flags & 1 else 'out/aot-features') if suite == 'aot-features' else ROOT / 'out/global-generics' / ('lightweight' if flags & 1 else 'result')
                for family in (['generic', 'eh'] if suite == 'aot-features' else ['formal']):
                    source = root / family if suite == 'aot-features' else root
                    for variant in (['default', 'interpreted-loop'] if family == 'eh' else ['default', 'disabled']):
                        report = json.loads((source / variant / 'generic-aot-report.json').read_text())
                        assert report['return_protocol_flags'] == flags
                        if family != 'eh':
                            assert bool(report['generic_normal_path_methods']) == (variant == 'default')
                        shutil.copy2(source / variant / 'generic-aot-report.json', destination / f'{family}-{variant}-report.json')
                        for target in targets:
                            path = source / f'{variant}-{target}-{cfg}-{arch}/Testing/Temporary/LastTest.log'
                            text = path.read_text(errors='replace')
                            assert 'Test Failed.' not in text and 'Test Passed.' in text, path
                            if family == 'formal':
                                for case, count in [('TC_GenericAot', 24), ('TC_InterfaceDispatch', 14), ('TC_Int32Returns', 7), ('TC_GenericEh', 13)]:
                                    assert text.count(f'Passed: AotTests.{case}.') == count, path
                            name = destination / f'{family}-{variant}-{target}.log'
                            shutil.copy2(path, name)
                            rows.append(dict(flags=flags, configuration=configuration, family=family, variant=variant, target=target,
                                             path=name.relative_to(ROOT).as_posix(), sha256=hashlib.sha256(name.read_bytes()).hexdigest()))
    (archive / 'matrix.json').write_text(json.dumps(rows, indent=2) + '\n', encoding='utf-8')
    print(f'Generic EH validation passed: {len(rows)} configurations', flush=True)


if __name__ == '__main__':
    main()
