#!/usr/bin/env python3
"""Read-only JTTY provenance verification and upstream comparison. Never imports files."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
MANIFESTS = ('wsjtz/jtty/source-manifest.json',
             'wsjtz/jtty/ui-audio-source-manifest.json',
             'wsjtz/jtty/live/source-manifest.json')

def git(repo, *args):
    return subprocess.check_output(['git', '-C', str(repo), *args], stderr=subprocess.PIPE)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--upstream', type=Path, help='Local official WSJT-X checkout')
    parser.add_argument('--ref', help='Candidate tag or commit already fetched into that checkout')
    args = parser.parse_args()
    if bool(args.upstream) != bool(args.ref):
        parser.error('--upstream and --ref must be supplied together')
    manifests = [json.loads((ROOT / name).read_text()) for name in MANIFESTS]
    failures = []
    tracked = {}
    for manifest in manifests:
        for entry in manifest['files']:
            source = entry['source']
            tracked[source] = entry.get('source_sha256', entry['sha256'])
            local = ROOT / entry['destination']
            if not local.is_file():
                failures.append({'file': entry['destination'], 'problem': 'missing'})
            elif hashlib.sha256(local.read_bytes()).hexdigest() != entry['sha256']:
                failures.append({'file': entry['destination'], 'problem': 'local hash mismatch'})
    result = {'baselines': [{'repository': m['repository'], 'tag': m['tag'],
                            'commit': m['commit']} for m in manifests],
              'individual_references': [{'source': e['source'], 'tag': e.get('source_tag'), 'commit': e['source_commit']} for m in manifests for e in m['files'] if 'source_commit' in e],
              'local_files_checked': sum(len(m['files']) for m in manifests),
              'local_failures': failures}
    if args.upstream:
        candidate = git(args.upstream, 'rev-parse', '--verify', '--end-of-options',
                        args.ref + '^{commit}').decode().strip()
        paths = set(git(args.upstream, 'ls-tree', '-r', '--name-only', candidate).decode().splitlines())
        changed, removed = [], []
        for source, expected in sorted(tracked.items()):
            if source not in paths:
                removed.append(source)
            elif hashlib.sha256(git(args.upstream, 'show', candidate + ':' + source)).hexdigest() != expected:
                changed.append(source)
        baseline_paths = set()
        for baseline in {m['commit'] for m in manifests}:
            baseline_paths.update(git(args.upstream, 'ls-tree', '-r', '--name-only', baseline).decode().splitlines())
        # Discover newly introduced paths, not existing but intentionally unvendored assets.
        additions = sorted(p for p in paths if p not in baseline_paths
                           and ('jtty' in p.lower() or 'AudioStream' in p))
        result.update(candidate_commit=candidate, changed=changed, removed=removed,
                      newly_discovered=additions,
                      reminder='Review shared engine/UI/audio dependencies and renamed files too; '
                               'filename discovery is not a complete dependency analysis. '
                               'Candidate comparison does not mean code has been integrated.')
    print(json.dumps(result, indent=2))
    return 1 if failures else 0

if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        # Do not dump command output or credentials from the environment.
        print('JTTY source check failed: ' + str(error), file=sys.stderr)
        sys.exit(2)
