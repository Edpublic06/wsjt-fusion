#!/usr/bin/env python3
"""Inspect a CI macOS bundle without opening audio, CAT/PTT or user settings."""
import argparse
import os
from pathlib import Path
import re
import subprocess
import json
import tempfile


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



# Loaded only by LLDB after the application's static resource initializers and
# before main executes. It uses the bundle's QtCore, not a second Qt runtime.
TRANSLATION_PROBE = r"""
#include <QDir>
#include <QTranslator>
extern "C" int fusion_check_translations() {
    const QDir dir(":/Translations");
    const auto files = dir.entryList({"wsjtx_*.qm"}, QDir::Files);
    int usable = 0;
    for (const auto &name : files) {
        QTranslator translator;
        if (!translator.load(dir.filePath(name))) return -1;
        // The deliberately empty English catalog is a locale fallback guard.
        if (!translator.isEmpty()) ++usable;
    }
    return usable;
}
"""


def verify_translations(app, qt_prefix):
    framework = app / 'Contents/Frameworks'
    core = framework / 'QtCore.framework'
    if not core.is_dir():
        raise RuntimeError('Bundle QtCore framework is missing for translation probe')
    with tempfile.TemporaryDirectory(prefix='fusion-translations-') as tmp:
        root = Path(tmp)
        source, library = root / 'probe.cpp', root / 'probe.dylib'
        source.write_text(TRANSLATION_PROBE)
        # Headers only come from the build Qt; linkage is exclusively bundle Qt.
        headers = qt_prefix / 'lib/QtCore.framework/Headers'
        subprocess.run(['xcrun', 'clang++', '-std=c++11', '-arch', 'arm64',
                        '-dynamiclib', str(source), '-I' + str(headers),
                        '-F' + str(framework), '-iframework', str(qt_prefix / 'lib'),
                        '-framework', 'QtCore',
                        '-Wl,-rpath,' + str(framework), '-o', str(library)], check=True)
        # No app arguments or Qt GUI startup: stop before main's first instruction.
        # Every debugger/load/expression error fails closed; no string scanning.
        driver = root / 'check.py'
        driver.write_text('''
import lldb

def check(debugger, command, result, internal_dict):
    process = debugger.GetSelectedTarget().GetProcess()
    try:
        if process.GetState() != lldb.eStateStopped:
            raise RuntimeError('Application did not stop before main')
        thread = process.GetSelectedThread()
        if thread.GetStopReason() != lldb.eStopReasonBreakpoint:
            raise RuntimeError('Application stopped unexpectedly')
        frame = thread.GetFrameAtIndex(0)
        if frame.GetFunctionName() != 'main':
            raise RuntimeError('Application did not stop at main')
        options = lldb.SBExpressionOptions()
        options.SetLanguage(lldb.eLanguageTypeC_plus_plus)
        options.SetTimeoutInMicroSeconds(30000000)
        def evaluate(expression):
            value = frame.EvaluateExpression(expression, options)
            if value.GetError().Fail():
                raise RuntimeError(str(value.GetError()))
            return value.GetValueAsUnsigned()
        handle = evaluate(LOAD_EXPRESSION)
        if not handle:
            raise RuntimeError('Cannot load translation probe with bundle QtCore')
        address = evaluate('(void*)dlsym((void*)%d, "fusion_check_translations")' % handle)
        if not address:
            raise RuntimeError('Translation probe symbol missing')
        count = evaluate('((int(*)())%d)()' % address)
        if not 0 < count < 1000:
            raise RuntimeError('No usable embedded translations, or a catalog failed to load')
        print('TRANSLATIONS_RUNTIME_PASS: %d nonempty catalogs loaded with QTranslator' % count)
    finally:
        process.Kill()
'''.replace('LOAD_EXPRESSION', repr(
            '(void*)dlopen(' + json.dumps(str(library)) + ', 2)'))
            + '\ndef __lldb_init_module(debugger, internal_dict):\n'
              '    debugger.HandleCommand("command script add -f check.check fusion-translations")\n')
        run = subprocess.run(['xcrun', 'lldb', '--batch',
                              '-o', 'breakpoint set --name main --skip-prologue false', '-o', 'run',
                              '-o', 'command script import ' + str(driver),
                              '-o', 'fusion-translations', '--',
                              str(app / 'Contents/MacOS/wsjtx')],
                             text=True, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT, timeout=120)
        print(run.stdout)
        if run.returncode or 'TRANSLATIONS_RUNTIME_PASS:' not in run.stdout:
            raise RuntimeError('Embedded translations runtime verification failed')


def verify(app, qt_prefix):
    app = app.resolve()
    mac = app / 'Contents/MacOS'
    resources = app / 'Contents/Resources'
    for name in ('rigctl-wsjtx', 'rigctld-wsjtx', 'rigctlcom-wsjtx'):
        helper = mac / name
        if helper.is_symlink():
            raise RuntimeError(f'Hamlib utility must be a copied executable, '
                               f'not a symlink: {helper} -> {os.readlink(helper)}')
        if not helper.is_file() or not helper.stat().st_size:
            raise RuntimeError(f'Missing or empty Hamlib utility: {helper}')
        if not os.access(helper, os.X_OK):
            raise RuntimeError(f'Hamlib utility is not executable: {helper}')
        if not macho(helper):
            raise RuntimeError(f'Hamlib utility is not Mach-O: {helper}')
        print(f'Hamlib utility: {name}, size={helper.stat().st_size}, '
              f'mode={oct(helper.stat().st_mode & 0o777)}')
        print(output('otool', '-L', str(helper)))
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
    verify_translations(app, qt_prefix)
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
    parser.add_argument('--qt-prefix', required=True, type=Path,
                        help='Build Qt headers for the runtime-only translation probe')
    args = parser.parse_args()
    verify(args.app, args.qt_prefix)
