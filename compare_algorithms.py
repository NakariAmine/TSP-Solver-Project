#!/usr/bin/env python3
"""Run the four algorithms and write readable tables to algorithm_comparison.md.
"""
import argparse
from datetime import datetime
from zoneinfo import ZoneInfo
import json
import math
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent
ALGORITHMS = [
    ('trivial', 'Trivial'),
    ('held-karp', 'Held–Karp'),
    ('aco-only', 'Ant colony only'),
    ('aco-optimized', 'Ant colony + optimization'),
]

# Build the C++ program that calls and times the solvers.
def build_worker():
    build_folder = ROOT / 'build'
    subprocess.run(['cmake', '-S', str(ROOT), '-B', str(build_folder),
                    '-DCMAKE_BUILD_TYPE=Release'], check=True)
    subprocess.run(['cmake', '--build', str(build_folder),
                    '--target', 'compare_algorithms'], check=True)
    return build_folder / 'compare_algorithms'


def read_header(path):
    info = {'edge_weight_format': 'Not specified'}
    for line in path.read_text().splitlines():
        line = line.strip()
        if line in ['NODE_COORD_SECTION', 'EDGE_WEIGHT_SECTION']:
            info['format'] = 'Coordinates' if line == 'NODE_COORD_SECTION' else 'Matrix'
            break
        if ':' in line:
            key, value = line.split(':', 1)
            info[key.strip().lower()] = value.strip()
    info['cities'] = int(info['dimension'])
    info.setdefault('edge_weight_type', 'EUC_2D')
    return info


def save_table(output, instances, results, timeout):
    text = [
        '# TSP algorithm comparison',
        '',
        'Generated: ' + datetime.now(ZoneInfo('Europe/Paris')).strftime('%Y-%m-%d %H:%M %Z'),
        '',
        '**One run per algorithm per file.** Execution time measures the solver only; '
        'file parsing, distance calculation, output, and tour validation are excluded. '
        'Tour length includes returning to the starting city; smaller is better.',
        '',
        'Both ant-colony variants use seed **7**, **30 ants**, and at most **200 iterations**. '
        '“Ant colony + optimization” is the adaptive bandit hybrid: it applies 2-opt, '
        'Or-opt, or their combination to the best two tours per iteration. '
        '“Ant colony only” uses no local search. All other settings retain their defaults.',
        'Local search uses one pass per operator without restarting after improvements. '
        'The combined operator performs one 2-opt pass followed by one Or-opt pass.',
        '',
        f'**Held–Karp keeps its current 20-city limit. Each process has a {timeout:g}-second timeout**, '
        'including startup and parsing. A timeout has no completed solver time or tour length. '
        '“Did not run” is not a measured failure. Times come from a Release build on this machine.',
        '',
    ]
    for path, info in instances:
        text += [
            f"## {path.stem} — {info['cities']} cities", '',
            f"File: `{path.relative_to(ROOT) if path.is_relative_to(ROOT) else path}`  ",
            f"Input: **{info['format']}** · Distance: **{info['edge_weight_type']}** · "
            f"Edge-weight format: **{info['edge_weight_format']}**", '',
            '| Algorithm | Execution time | Tour length | Result |',
            '|---|---:|---:|---|',
        ]
        for algorithm, label in ALGORITHMS:
            row = results.get((str(path), algorithm), {})
            elapsed = '—'
            length = '—'
            if 'time_ms' in row:
                if row['time_ms'] < 0.001:
                    elapsed = '<0.001 ms'
                else:
                    elapsed = f"{row['time_ms']:.3f} ms"
            if 'length' in row:
                length = f"{row['length']:,.0f}"
            status = row.get('status', 'Not run yet').replace('|', '/')
            text.append(f'| {label} | {elapsed} | {length} | {status} |')
        text.append('')
    output.write_text('\n'.join(text)+'\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT/'algorithm_comparison.md')
    parser.add_argument('--instances-dir', type=Path, default=ROOT/'TSP-instances')
    parser.add_argument('--timeout', type=float, default=120)
    args = parser.parse_args()
    if not math.isfinite(args.timeout) or args.timeout <= 0:
        parser.error('Timeout must be positive')
    output = args.output.resolve()
    if output.exists():
        parser.error('Output already exists; choose another --output filename')
    if not output.parent.is_dir():
        parser.error('Output folder does not exist')
    instances = []
    for path in args.instances_dir.resolve().rglob('*.tsp'):
        info = read_header(path)
        instances.append((path, info))
    instances.sort(key=lambda item: (item[1]['cities'], str(item[0])))
    if not instances:
        parser.error('No .tsp files found')
    binary = build_worker()
    results = {}
    save_table(output, instances, results, args.timeout)
    current = None
    try:
        for path, info in instances:
            for algorithm, label in ALGORITHMS:
                current = (str(path), algorithm)
                print(f"{path.stem}: {label}", flush=True)
                if algorithm == 'held-karp' and info['cities'] > 20:
                    results[current] = {'status': 'Did not run — exceeds 20-city limit'}
                else:
                    try:
                        process = subprocess.run([str(binary), str(path), algorithm],
                                                 capture_output=True, text=True, timeout=args.timeout)
                        if process.returncode:
                            raise ValueError(process.stderr.strip() or 'Solver failed')
                        result = None
                        for line in process.stdout.splitlines():
                            if line.startswith('RESULT '):
                                result = json.loads(line[7:])
                                break
                        if result is None:
                            raise ValueError('Solver did not return a result')
                        if sorted(result['tour']) != list(range(info['cities'])):
                            raise ValueError('Returned tour is invalid')
                        results[current] = {'status': 'Completed — valid tour',
                                            'time_ms': result['time_ms'], 'length': result['length']}
                    except subprocess.TimeoutExpired:
                        results[current] = {'status': f'Stopped — exceeded {args.timeout:g} seconds'}
                    except ValueError as error:
                        results[current] = {'status': 'Failed — ' + str(error).replace('\n', ' ')}
                save_table(output, instances, results, args.timeout)
                print('  '+results[current]['status'], flush=True)
    except KeyboardInterrupt:
        if current and current not in results:
            results[current] = {'status': 'Stopped by user'}
        for path, _ in instances:
            for algorithm, _ in ALGORITHMS:
                results.setdefault((str(path), algorithm), {'status': 'Did not run — stopped by user'})
        save_table(output, instances, results, args.timeout)
        print('\nStopped. Completed results were preserved.', flush=True)
        return 130
    print(f'\nReadable tables saved to {output}', flush=True)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
