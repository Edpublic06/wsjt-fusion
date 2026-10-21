#!/usr/bin/env python3
"""Inspect a CI macOS bundle without opening audio, CAT/PTT or user settings."""
import argparse
import os
from pathlib import Path
import re
import subprocess
import json
import hashlib
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
# before the Mach-O LC_MAIN entry point executes. It uses the bundle's QtCore, not a second Qt runtime.
TRANSLATION_PROBE = r"""
#include <QDir>
#include <QTranslator>
extern "C" int fusion_check_translations() {
    const QDir dir(":/Translations");
    const auto files = dir.entryList({"wsjtx_*.qm"}, QDir::Files);
    // Negative controls use Qt itself, before accepting the embedded catalogs.
    QTranslator rejected;
    const uchar corrupt[] = "not a Qt translation catalog";
    if (rejected.load(":/Translations/wsjtx_ci_missing.qm") ||
        rejected.load(corrupt, sizeof(corrupt))) return -2;
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


def executable_entry_address(executable):
    """Resolve LC_MAIN's file offset to an unslid __TEXT virtual address."""
    entries, text_segments = [], []
    for block in re.split(r'^Load command \d+\s*$',
                          output('otool', '-l', str(executable)), flags=re.M):
        def field(name):
            values = re.findall(r'^\s*' + name + r'\s+(\S+)\s*$', block, re.M)
            if len(values) != 1:
                raise RuntimeError(f'Invalid Mach-O field {name}: {executable}')
            return values[0]
        if re.search(r'^\s*cmd LC_MAIN\s*$', block, re.M):
            entries.append(int(field('entryoff'), 0))
        if (re.search(r'^\s*cmd LC_SEGMENT_64\s*$', block, re.M)
                and re.search(r'^\s*segname __TEXT\s*$', block, re.M)):
            # Restrict to segment fields; __TEXT's section records repeat names.
            block = block.split('Section', 1)[0]
            text_segments.append(tuple(int(field(name), 0)
                                       for name in ('vmaddr', 'fileoff', 'filesize')))
    if len(entries) != 1 or len(text_segments) != 1:
        raise RuntimeError(f'Expected one LC_MAIN and one __TEXT: {executable}')
    vmaddr, fileoff, filesize = text_segments[0]
    if not fileoff <= entries[0] < fileoff + filesize:
        raise RuntimeError(f'LC_MAIN entry is outside __TEXT: {executable}')
    return vmaddr + entries[0] - fileoff


def verify_translations(app, qt_prefix):
    executable = (app / 'Contents/MacOS/wsjtx').resolve()
    entry_address = executable_entry_address(executable)
    print(f'Mach-O LC_MAIN entry address: {entry_address:#x}')
    framework = app / 'Contents/Frameworks'
    core = framework / 'QtCore.framework'
    if not core.is_dir():
        raise RuntimeError('Bundle QtCore framework is missing for translation probe')
    with tempfile.TemporaryDirectory(prefix='fusion-translations-') as tmp:
        root = Path(tmp)
        source, library = root / 'probe.cpp', root / 'probe.dylib'
        source.write_text(TRANSLATION_PROBE)
        # Compile separately: a bundle framework has no development headers and
        # must never shadow Homebrew's framework during QtCore/... includes.
        headers = qt_prefix / 'lib/QtCore.framework/Headers'
        installed_headers = Path(output(str(qt_prefix / 'bin/qmake'),
                                        '-query', 'QT_INSTALL_HEADERS'))
        print(f'Qt headers: {installed_headers}; QtCore headers: {headers}')
        for name in ('QDir', 'QTranslator', 'qstring.h', 'qglobal.h'):
            if not (headers / name).is_file():
                raise RuntimeError(f'Missing build Qt header: {headers / name}')
        obj = root / 'probe.o'
        subprocess.run(['xcrun', 'clang++', '-std=c++11', '-arch', 'arm64',
                        '-fPIC', '-c', str(source), '-I' + str(installed_headers),
                        '-I' + str(headers), '-F' + str(qt_prefix / 'lib'),
                        '-o', str(obj)], check=True)
        # No Homebrew search path at link time: use the final bundle framework.
        subprocess.run(['xcrun', 'clang++', '-arch', 'arm64', '-dynamiclib',
                        str(obj), '-F' + str(framework), '-framework', 'QtCore',
                        '-Wl,-rpath,' + str(framework), '-o', str(library)], check=True)

        def qtcore_loads(image):
            return [line.strip().split(' (', 1)[0]
                    for line in output('otool', '-L', str(image)).splitlines()[1:]
                    if '/QtCore.framework/' in line]

        # Match the application's QtCore load command before dlopen. The LLDB
        # check below also verifies the actual loaded image before and after.
        loads = qtcore_loads(library)
        if len(loads) != 1 or loads != qtcore_loads(app / 'Contents/MacOS/wsjtx'):
            raise RuntimeError(f'Translation probe QtCore differs from application: {loads}')
        print(f'Translation probe QtCore dependency: {loads[0]}')
        # Set one address breakpoint before launch, using LC_MAIN rather than
        # potentially ambiguous name/debug-info matches for 'main'.
        # Every debugger/load/expression error fails closed; no string scanning.
        driver = root / 'check.py'
        driver.write_text('''
import lldb
import os
from pathlib import Path

def check(debugger, command, result, internal_dict):
    process = None
    try:
        debugger.SetAsync(False)
        target = debugger.GetSelectedTarget()
        module = target.GetModuleAtIndex(0)
        def module_path(m):
            if not m.IsValid():
                return '<invalid>'
            spec = m.GetFileSpec()
            return Path(spec.GetDirectory(), spec.GetFilename()).resolve()
        if module_path(module) != Path(APP_EXECUTABLE):
            raise RuntimeError('Unexpected LLDB executable module')
        entry = module.ResolveFileAddress(ENTRY_ADDRESS)
        if not entry.IsValid() or entry.GetModule() != module:
            raise RuntimeError('Cannot resolve LC_MAIN in the application module')
        breakpoint = target.BreakpointCreateBySBAddress(entry)
        if not breakpoint.IsValid() or breakpoint.GetNumLocations() != 1:
            raise RuntimeError('LC_MAIN breakpoint must have exactly one location')
        # Keep the breakpoint alive: one-shot deletion can clear LLDB's stop
        # reason data before GetStopReasonDataCount/GetStopReasonDataAtIndex.
        breakpoint_id = breakpoint.GetID()
        location = breakpoint.GetLocationAtIndex(0)
        location_id = location.GetID()
        address = location.GetAddress()
        if address.GetModule() != module or address.GetFileAddress() != ENTRY_ADDRESS:
            raise RuntimeError('LC_MAIN breakpoint location has an unexpected address')
        print('Unique LC_MAIN breakpoint: id=%d location=%d file_address=%#x module=%s'
              % (breakpoint_id, location_id, ENTRY_ADDRESS, module_path(module)))
        launch = lldb.SBLaunchInfo([])
        # Environment belongs to this probe only; tests keep their offscreen QPA.
        launch.SetEnvironmentEntries([key + '=' + value
                                     for key, value in os.environ.items()], False)
        launch.SetWorkingDirectory(str(Path(APP_EXECUTABLE).parent))
        error = lldb.SBError()
        process = target.Launch(launch, error)
        if error.Fail():
            raise RuntimeError('Cannot launch translation probe: %s' % error)
        print('LLDB process state: %s' % process.GetState())
        if process.GetState() != lldb.eStateStopped:
            raise RuntimeError('Application did not stop at LC_MAIN')
        thread = process.GetSelectedThread()
        reason = thread.GetStopReason()
        stop_data = [thread.GetStopReasonDataAtIndex(i)
                     for i in range(thread.GetStopReasonDataCount())]
        frame = thread.GetFrameAtIndex(0)
        pc = frame.GetPCAddress()
        print('LLDB stop: expected_breakpoint=%d expected_location=%d '
              'reason=%s data=%s PC_file=%#x PC_load=%#x module=%s'
              % (breakpoint_id, location_id, reason, stop_data,
                 pc.GetFileAddress(), pc.GetLoadAddress(target),
                 module_path(pc.GetModule())))
        if (reason != lldb.eStopReasonBreakpoint
                or stop_data != [breakpoint_id, location_id]):
            raise RuntimeError('Application stopped outside the LC_MAIN breakpoint')
        if pc.GetModule() != module or pc.GetFileAddress() != ENTRY_ADDRESS:
            raise RuntimeError('Application did not stop at the exact LC_MAIN address')
        print('Stopped at LC_MAIN before executing the application entry point')
        options = lldb.SBExpressionOptions()
        options.SetLanguage(lldb.eLanguageTypeC_plus_plus)
        options.SetTimeoutInMicroSeconds(30000000)
        def evaluate(expression):
            value = frame.EvaluateExpression(expression, options)
            if value.GetError().Fail():
                raise RuntimeError(str(value.GetError()))
            return value.GetValueAsUnsigned()
        def check_qtcore():
            paths = {Path(m.GetFileSpec().GetDirectory(),
                          m.GetFileSpec().GetFilename()).resolve()
                     for m in debugger.GetSelectedTarget().modules
                     if m.GetFileSpec().GetFilename() == 'QtCore'}
            if paths != {Path(BUNDLE_QTCORE)}:
                raise RuntimeError('Expected only bundle QtCore, got: %s' % paths)
        check_qtcore()
        handle = evaluate(LOAD_EXPRESSION)
        if not handle:
            raise RuntimeError('Cannot load translation probe with bundle QtCore')
        check_qtcore()
        address = evaluate('(void*)dlsym((void*)%d, "fusion_check_translations")' % handle)
        if not address:
            raise RuntimeError('Translation probe symbol missing')
        count = evaluate('((int(*)())%d)()' % address)
        if count != 9:
            raise RuntimeError('Expected nine usable embedded catalogs; missing/corrupt catalog or negative-control failure')
        print('TRANSLATIONS_RUNTIME_PASS: %d nonempty catalogs loaded with QTranslator; missing/corrupt controls rejected; only bundle QtCore loaded' % count)
        # Real dyld check of the WebP plugin chain; no GUI or radio is started.
        webp = evaluate(WEBP_PLUGIN_EXPRESSION)
        if not webp:
            message = frame.EvaluateExpression('(const char*)dlerror()', options)
            raise RuntimeError('WebP plugin dlopen failed: %s' % message.GetSummary())
        check_qtcore()
        # Reject any on-disk image loaded from Homebrew or another external
        # location. The temporary diagnostic dylib is the sole explicit exception.
        for loaded in target.modules:
            path = module_path(loaded)
            if not isinstance(path, Path) or not path.is_file():
                continue  # LLDB JIT images have no on-disk Mach-O file.
            if str(path).startswith(('/System/Library/', '/usr/lib/')):
                continue
            if path == Path(PROBE_FILE):
                continue
            if not path.is_relative_to(Path(APP_EXECUTABLE).parent.parent.parent):
                raise RuntimeError('External image loaded during WebP probe: %s' % path)
        print('WEBP_DYLD_PASS: bundle WebP plugin loaded; only bundle QtCore loaded; no external on-disk images')
    finally:
        if process is not None and process.IsValid():
            process.Kill()
'''.replace('LOAD_EXPRESSION', repr(
            '(void*)dlopen(' + json.dumps(str(library)) + ', 2)'))
            .replace('WEBP_PLUGIN_EXPRESSION', repr(
                '(void*)dlopen(' + json.dumps(str(app / 'Contents/PlugIns/imageformats/libqwebp.dylib')) + ', 2)'))
            .replace('PROBE_FILE', repr(str(library.resolve())))
            .replace('ENTRY_ADDRESS', repr(entry_address))
            .replace('APP_EXECUTABLE', repr(str(executable)))
            .replace('BUNDLE_QTCORE', repr(str((core / 'Versions/5/QtCore').resolve())))
            + '\ndef __lldb_init_module(debugger, internal_dict):\n'
              '    debugger.HandleCommand("command script add -f check.check fusion-translations")\n')
        probe_env = os.environ.copy()
        if probe_env.get('QT_QPA_PLATFORM') == 'offscreen':
            del probe_env['QT_QPA_PLATFORM']
        run = subprocess.run(['xcrun', 'lldb', '--batch',
                              '-o', 'command script import ' + str(driver),
                              '-o', 'fusion-translations', '--',
                              str(app / 'Contents/MacOS/wsjtx')],
                             text=True, stdout=subprocess.PIPE, env=probe_env,
                             stderr=subprocess.STDOUT, timeout=120)
        print(run.stdout)
        if (run.returncode or 'TRANSLATIONS_RUNTIME_PASS:' not in run.stdout
                or 'WEBP_DYLD_PASS:' not in run.stdout):
            raise RuntimeError('Bundle translations/WebP runtime verification failed')



def snapshot_dependencies(app, destination):
    """Preserve the actual WebP graph and Frameworks symlinks for comparison."""
    app = app.resolve()
    frameworks = app / 'Contents/Frameworks'
    inventory = []
    for path in sorted(frameworks.rglob('*')):
        if path.is_symlink() or path.is_file():
            inventory.append(dict(path=str(path.relative_to(app)),
                                  link=os.readlink(path) if path.is_symlink() else None,
                                  exists=path.exists(), resolved=str(path.resolve())))
    selected = {app / 'Contents/MacOS/wsjtx',
                app / 'Contents/PlugIns/imageformats/libqwebp.dylib'}
    for pattern in ('libwebp*.dylib', 'libsharpyuv*.dylib'):
        selected.update(frameworks.glob(pattern))
    images = []
    for path in sorted(selected):
        resolved = path.resolve()
        item = dict(path=str(path.relative_to(app)), resolved=str(resolved),
                    link=os.readlink(path) if path.is_symlink() else None,
                    exists=path.exists())
        if resolved.is_relative_to(app) and path.is_file() and macho(path):
            item.update(sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                        otool_L=output('otool', '-L', str(path)),
                        otool_l=output('otool', '-l', str(path)),
                        architectures=output('lipo', '-archs', str(path)))
        images.append(item)
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(json.dumps(dict(frameworks=inventory, images=images), indent=2))
    print(f'Dependency snapshot: {destination}')


def verify_dependency_graph(app, images, executable_paths):
    """Check each real load chain, retaining its dyld run-path stack."""
    main = (app / 'Contents/MacOS/wsjtx').resolve()
    system = ('/System/Library/', '/usr/lib/')
    visited, covered = set(), set()

    def expand(text, image, executable):
        return Path(text.replace('@loader_path', str(image.parent))
                    .replace('@executable_path', str(executable.parent)))

    def walk(image, executable, inherited=()):
        search = tuple(dict.fromkeys(
            [expand(s, image, executable) for s in rpaths(image)] + list(inherited)))
        key = image, executable, search
        if key in visited:
            return
        visited.add(key)
        covered.add(image)
        for line in output('otool', '-L', str(image)).splitlines()[1:]:
            dep = line.strip().split(' (', 1)[0]
            if dep.startswith(system):
                continue
            candidates = ([p / dep[len('@rpath/'):] for p in search]
                          if dep.startswith('@rpath/') else [expand(dep, image, executable)])
            resolved = next((p.resolve() for p in candidates if p.is_file()), None)
            if resolved is None or not resolved.is_relative_to(app):
                raise RuntimeError(f'Unresolved/external dependency: {image}: {dep}; '
                                   f'executable={executable}; rpaths={list(search)}; '
                                   f'candidates={candidates}')
            if resolved != image:
                walk(resolved, executable, search)

    for executable in executable_paths:
        walk(executable.resolve(), executable.resolve())
    main_search = [expand(s, main, main) for s in rpaths(main)]
    # Qt plugins and the isolated decoder are actual dlopen roots. Libraries
    # reached through them inherit that root's paths, rather than losing them
    # through a second, artificial standalone load from the main executable.
    roots = [p for p in images if (app / 'Contents/PlugIns') in p.parents]
    engine = (app / 'Contents/MacOS/libfusion_decoder_beta2.dylib').resolve()
    if engine in images:
        roots.append(engine)
    for root in roots:
        walk(root, main, main_search)
    # Retain strict coverage for unused/duplicate images as standalone roots.
    # Nothing is exempted simply because the executable graph did not reach it.
    for image in images:
        if image not in covered:
            walk(image, main, main_search)


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
    for image in images:
        arch = output('lipo', '-archs', str(image)).split()
        if 'arm64' not in arch:
            raise RuntimeError(f'Non-ARM64 Mach-O: {image}: {arch}')
        print(f'ARM64: {image.relative_to(app)}')
    verify_dependency_graph(app, images, executable_paths)
    print(f'PASS: {len(images)} ARM64 images; resources/plugins present; '
          'all non-system dynamic dependencies resolve inside the bundle.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('app', type=Path)
    parser.add_argument('--qt-prefix', type=Path,
                        help='Build Qt headers for the runtime-only translation probe')
    parser.add_argument('--snapshot', type=Path,
                        help='Record WebP dependencies, rpaths, hashes and Frameworks links')
    args = parser.parse_args()
    if args.snapshot:
        snapshot_dependencies(args.app, args.snapshot)
    elif args.qt_prefix is None:
        parser.error('--qt-prefix is required for verification')
    else:
        verify(args.app, args.qt_prefix)
