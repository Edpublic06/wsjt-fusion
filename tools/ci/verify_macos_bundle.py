#!/usr/bin/env python3
"""Inspect a CI macOS bundle without opening audio, CAT/PTT or user settings."""
import argparse
from pathlib import Path
import re
import subprocess


def output(*args):
    return subprocess.check_output(args, text=True).strip()


def macho(path):
    with path.open('rb') as stream:
        return stream.read(4) in (
            b'\xcf\xfa\xed\xfe', b'\xfe\xed\xfa\xcf',
            b'\xca\xfe\xba\xbe', b'\xbe\xba\xfe\xca',
            b'\xca\xfe\xba\xbf', b'\xbf\xba\xfe\xca',
        )


def rpaths(path):
    lines = output('otool', '-l', str(path)).splitlines()
    result = []
    for index, line in enumerate(lines):
        if line.strip() == 'cmd LC_RPATH':
            for entry in lines[index + 1:index + 4]:
                match = re.match(r'\s*path (.+) \(offset \d+\)', entry)
                if match:
                    result.append(match.group(1))
    return result


def verify(app):
    app = app.resolve()
    mac = app / 'Contents/MacOS'
    resources = app / 'Contents/Resources'
    required = (
        mac / 'wsjtx', mac / 'jt9', mac / 'fusion-jtty-rx', mac / 'wsprd',
        mac / 'libfusion_decoder_beta2.dylib', mac / 'ALLCALL7.TXT',
        mac / 'rigctl-wsjtx', mac / 'rigctld-wsjtx', mac / 'rigctlcom-wsjtx',
        app / 'Contents/Info.plist', resources / 'qt.conf',
        resources / 'Fusion-Source-Notices/COPYING',
    )
    for path in required:
        if not path.is_file() or not path.stat().st_size:
            raise RuntimeError(f'Missing or empty bundle resource: {path}')
    for name in ('cty.dat', 'USState.db', 'JPLEPH'):
        if not any(p.is_file() for p in app.rglob(name)):
            raise RuntimeError(f'Missing application data: {name}')
    if not list(resources.rglob('wsjtx_*.qm')):
        raise RuntimeError('Missing application translations')
    conf = (resources / 'qt.conf').read_text()
    match = re.search(r'^Plugins\s*=\s*(.+)$', conf, re.M)
    if not match:
        raise RuntimeError('No plugin path in qt.conf')
    plugins = (app / 'Contents' / match.group(1).strip()).resolve()
    if not plugins.is_relative_to(app):
        raise RuntimeError('qt.conf points outside the application')
    for name in ('platforms/libqcocoa.dylib', 'sqldrivers/libqsqlite.dylib'):
        if not (plugins / name).is_file():
            raise RuntimeError(f'Missing Qt plugin: {name}')
    if not list((plugins / 'audio').glob('*.dylib')):
        raise RuntimeError('Missing Qt audio plugins')

    images = sorted({p.resolve() for p in app.rglob('*') if p.is_file() and macho(p)})
    if not images:
        raise RuntimeError('No Mach-O image in bundle')
    executable_paths = [p for p in mac.iterdir() if p.is_file() and macho(p)
                        and p.suffix != '.dylib']
    system = ('/System/Library/', '/usr/lib/')

    def expand(text, image, executable):
        return Path(text.replace('@loader_path', str(image.parent))
                    .replace('@executable_path', str(executable.parent)))

    visited = set()

    def walk(image, executable, inherited=()):
        key = image, executable
        if key in visited:
            return
        visited.add(key)
        search = [expand(s, image, executable) for s in rpaths(image)] + list(inherited)
        for line in output('otool', '-L', str(image)).splitlines()[1:]:
            dep = line.strip().split(' (', 1)[0]
            if dep.startswith(system):
                continue
            if dep.startswith('@rpath/'):
                candidates = [p / dep[len('@rpath/'):] for p in search]
            else:
                candidates = [expand(dep, image, executable)]
            resolved = next((p.resolve() for p in candidates if p.is_file()), None)
            if resolved is None or not resolved.is_relative_to(app):
                raise RuntimeError(f'Unresolved/external dependency: {image}: {dep}')
            if resolved != image:
                walk(resolved, executable, search)

    for image in images:
        arch = output('lipo', '-archs', str(image)).split()
        if 'arm64' not in arch:
            raise RuntimeError(f'Non-ARM64 Mach-O: {image}: {arch}')
        print(f'ARM64: {image.relative_to(app)}')
    for executable in executable_paths:
        walk(executable.resolve(), executable.resolve())
    # Plugins and the dlopen-loaded engine must also pass even if not in otool's
    # normal executable dependency graph. Qt plugins use the main app context.
    for image in images:
        walk(image, (mac / 'wsjtx').resolve(),
             [expand(s, mac / 'wsjtx', mac / 'wsjtx') for s in rpaths(mac / 'wsjtx')])
    print(f'PASS: {len(images)} ARM64 images; resources/plugins present; '
          'all non-system dynamic dependencies resolve inside the bundle.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('app', type=Path)
    verify(parser.parse_args().app)
