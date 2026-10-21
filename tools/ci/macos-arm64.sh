#!/usr/bin/env bash
# CI orchestration only. No changes to the radio application or other platforms.
set -euo pipefail
: "${CI_ROOT:?Set CI_ROOT to an isolated build directory}"
source_root="$(cd "$(dirname "$0")/../.." && pwd)"
mkdir -p "$CI_ROOT/logs"

check_resources() {
  python3 - "$CI_ROOT" "$1" "$2" <<'PY'
from pathlib import Path
import json, shutil, subprocess, sys
root, phase, minimum_gib = Path(sys.argv[1]), sys.argv[2], int(sys.argv[3])
disk = shutil.disk_usage(root)
memory = int(subprocess.check_output(['sysctl', '-n', 'hw.memsize'], text=True))
record = dict(phase=phase, free_disk_bytes=disk.free, total_disk_bytes=disk.total,
              physical_memory_bytes=memory, minimum_free_disk_gib=minimum_gib)
(root / 'logs' / f'resources-{phase}.json').write_text(json.dumps(record, indent=2))
print(json.dumps(record, indent=2))
if memory < 6 * 1024**3:
    raise SystemExit('Less than 6 GiB physical RAM; stop before heavyweight builds.')
if disk.free < minimum_gib * 1024**3:
    raise SystemExit(f'Less than {minimum_gib} GiB free disk; stop without deleting runner software.')
PY
  vm_stat > "$CI_ROOT/logs/vm-stat-$1.txt"
  df -h "$CI_ROOT" > "$CI_ROOT/logs/disk-$1.txt"
}

case "${1:-}" in
  setup)
    test "$(uname -m)" = arm64
    test "$(brew --prefix)" = /opt/homebrew
    check_resources before-dependencies 10
    # Same runtime requirement documented by build-macos-preserve.sh. Change
    # only this disposable runner, never install a persistent launch daemon.
    if test "$(sysctl -n kern.sysv.shmmax)" -lt 134217728; then
      sudo sysctl -w kern.sysv.shmmax=134217728
    fi
    if test "$(sysctl -n kern.sysv.shmall)" -lt 32768; then
      sudo sysctl -w kern.sysv.shmall=32768
    fi
    sysctl kern.sysv.shmmax kern.sysv.shmall > "$CI_ROOT/logs/shared-memory.txt"
    # Require native binary bottles. Do not compile Qt/Boost/GCC on a 7 GB VM.
    brew install --force-bottle ninja pkgconf gcc@14 boost fftw hamlib libusb portaudio qt@5 \
      2>&1 | tee "$CI_ROOT/logs/dependencies-install.log"
    # Pin the CMake release already exercised on Linux; do not change app minima.
    python3 -m venv "$CI_ROOT/venv"
    "$CI_ROOT/venv/bin/python" -m pip install 'cmake==3.31.6' \
      2>&1 | tee "$CI_ROOT/logs/cmake-install.log"
    check_resources after-dependencies 6
    # Applies to both this project and the engine's nested CMake invocation.
    # Ninja pools also limit the nested script's explicit -j6 without editing it.
    cat > "$CI_ROOT/runner-limits.cmake" <<'CMAKE'
set(CMAKE_JOB_POOLS "ci_compile=2;ci_link=1" CACHE STRING "CI memory limits" FORCE)
set(CMAKE_JOB_POOL_COMPILE ci_compile CACHE STRING "CI compile pool" FORCE)
set(CMAKE_JOB_POOL_LINK ci_link CACHE STRING "CI link pool" FORCE)
CMAKE
    qt_prefix="$(brew --prefix qt@5)"
    fc="$(brew --prefix gcc@14)/bin/gfortran-14"
    test -x "$fc"
    test -x "$qt_prefix/bin/qmake"
    test -x "$qt_prefix/bin/macdeployqt"
    for tool in rigctl rigctld rigctlcom; do command -v "$tool"; done
    for plugin in platforms/libqcocoa.dylib sqldrivers/libqsqlite.dylib; do
      test -f "$qt_prefix/plugins/$plugin"
    done
    for directory in audio accessible imageformats styles; do
      test -d "$qt_prefix/plugins/$directory" || {
        echo "Qt plugin directory expected by CMake is absent: $directory. Review deployment before changing sources."
        exit 1
      }
    done
    {
      brew list --versions
      "$fc" --version
      xcrun clang --version
      "$qt_prefix/bin/qmake" -query
      "$CI_ROOT/venv/bin/cmake" --version
      brew info --json=v2 gcc@14 boost fftw hamlib libusb portaudio qt@5
    } > "$CI_ROOT/logs/toolchain.txt" 2>&1
    # Check the libraries explicitly named by the existing isolated-engine script.
    for library in boost_log_setup boost_log boost_thread boost_filesystem boost_chrono boost_atomic; do
      test -f "/opt/homebrew/lib/lib${library}.dylib" || {
        echo "Required library absent: $library. Review dependency compatibility before changing sources."
        exit 1
      }
    done
    {
      echo "FC=$fc"
      echo "QT_PREFIX=$qt_prefix"
      echo "CC=$(xcrun --find clang)"
      echo "CXX=$(xcrun --find clang++)"
      echo "CMAKE_TOOLCHAIN_FILE=$CI_ROOT/runner-limits.cmake"
    } >> "${GITHUB_ENV:?GITHUB_ENV is required for setup}"
    echo "$CI_ROOT/venv/bin" >> "${GITHUB_PATH:?}"
    ;;
  build)
    test ! -e "$CI_ROOT/build"
    cmake -S "$source_root" -B "$CI_ROOT/build" -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
      -DCMAKE_Fortran_COMPILER="$FC" \
      -DCMAKE_PREFIX_PATH="$QT_PREFIX;/opt/homebrew" \
      -DQt5_DIR="$QT_PREFIX/lib/cmake/Qt5" \
      -DFFTW3_ROOT_DIR="$(brew --prefix fftw)" \
      -DHamlib_ROOT="$(brew --prefix hamlib)" \
      -DUsb_ROOT="$(brew --prefix libusb)" \
      -DWSJT_SKIP_MANPAGES=ON -DWSJT_GENERATE_DOCS=OFF -DWSJT_BUILD_UTILS=ON \
      -DCMAKE_INSTALL_PREFIX="$CI_ROOT/stage" \
      2>&1 | tee "$CI_ROOT/logs/configure.log"
    # Build the isolated engine first so its nested Ninja never competes with
    # the application's C++ compilation for the standard runner's memory.
    cmake --build "$CI_ROOT/build" --target fusion_beta2_engine --parallel 2 \
      2>&1 | tee "$CI_ROOT/logs/engine-build.log"
    cmake --build "$CI_ROOT/build" --parallel 2 \
      2>&1 | tee "$CI_ROOT/logs/build.log"
    ;;
  test)
    ctest --test-dir "$CI_ROOT/build" --show-only=json-v1 > "$CI_ROOT/logs/test-inventory.json"
    # Ensure the platform-specific regression was actually registered.
    python3 - "$CI_ROOT/logs/test-inventory.json" <<'PY'
import json, sys
tests = {t['name'] for t in json.load(open(sys.argv[1]))['tests']}
assert {'fusion_beta2_gui_decoders', 'fusion_beta2_bridge'} <= tests
assert len(tests) >= 25, tests
PY
    ctest --test-dir "$CI_ROOT/build" --output-on-failure --parallel 1 \
      --output-junit "$CI_ROOT/logs/tests.xml" \
      2>&1 | tee "$CI_ROOT/logs/tests.log"
    ;;
  package)
    check_resources before-package 2
    # Start with a fresh installation, never an old developer/test bundle.
    cmake --install "$CI_ROOT/build" 2>&1 | tee "$CI_ROOT/logs/install.log"
    app="$CI_ROOT/stage/wsjtx.app"
    "$QT_PREFIX/bin/macdeployqt" "$app" -always-overwrite -verbose=2 \
      -executable="$app/Contents/MacOS/jt9" \
      -executable="$app/Contents/MacOS/fusion-jtty-rx" \
      -executable="$app/Contents/MacOS/wsprd" \
      -executable="$app/Contents/MacOS/libfusion_decoder_beta2.dylib" \
      2>&1 | tee "$CI_ROOT/logs/macdeployqt.log"
    # License notices are deliberately included beside the application resources.
    mkdir -p "$app/Contents/Resources/Fusion-Source-Notices"
    cp "$source_root/"{COPYING,NOTICE.md,UPSTREAM-SOURCES.md} \
      "$app/Contents/Resources/Fusion-Source-Notices/"
    python3 "$source_root/tools/ci/verify_macos_bundle.py" "$app" \
      2>&1 | tee "$CI_ROOT/logs/bundle-verification.log"
    codesign --force --deep --sign - --timestamp=none \
      --entitlements "$source_root/Darwin/entitlements.plist" "$app" \
      2>&1 | tee "$CI_ROOT/logs/codesign.log"
    codesign --verify --deep --strict --verbose=2 "$app" \
      2>&1 | tee -a "$CI_ROOT/logs/codesign.log"
    ;;
  report)
    df -h "$CI_ROOT" > "$CI_ROOT/logs/disk-final.txt"
    vm_stat > "$CI_ROOT/logs/vm-stat-final.txt"
    if test -d "$CI_ROOT/build/Testing"; then
      cp -R "$CI_ROOT/build/Testing" "$CI_ROOT/logs/Testing"
    fi
    if test -f "$CI_ROOT/build/CMakeCache.txt"; then
      cp "$CI_ROOT/build/CMakeCache.txt" "$CI_ROOT/logs/"
    fi
    python3 - "$CI_ROOT/logs" <<'PY'
from pathlib import Path
import os, re, sys
p = Path(sys.argv[1])
warnings = []
for name in ('configure.log', 'engine-build.log', 'build.log', 'install.log', 'macdeployqt.log'):
    for line in (p / name).read_text(errors='replace').splitlines() if (p / name).exists() else []:
        if re.search(r'warning[:\s]|Warning:', line, re.I):
            warnings.append(f'{name}: {line}')
(p / 'compiler-warnings.txt').write_text('\n'.join(warnings) + '\n')
risk = [s for s in warnings if re.search(
    r'uninitial|overflow|bounds|truncat|static storage|concurren|executable stack|INTENT\(OUT\)|interoperab', s, re.I)]
(p / 'memory-risk-warnings.txt').write_text('\n'.join(risk) + '\n')
summary = (f'## Native macOS ARM64 beta3\n\nCommit: `{os.environ.get("GITHUB_SHA", "unknown")}`\n\n'
           f'{len(warnings)} warning lines; {len(risk)} require memory/ABI/concurrency review. '
           'Warnings are not suppressed. See the diagnostic artifact.\n\n'
           'A successful run is a development build, not a release or radio validation. '
           'Ad-hoc signing only; no notarization. Homebrew versions are recorded, not fully locked. '
           'Native bottles required; compilation pools limited to two jobs, links to one; tests serial.\n')
(p / 'report.md').write_text(summary)
if os.environ.get('GITHUB_STEP_SUMMARY'):
    with open(os.environ['GITHUB_STEP_SUMMARY'], 'a') as out:
        out.write(summary)
PY
    ;;
  *) echo 'Usage: macos-arm64.sh setup|build|test|package|report' >&2; exit 2 ;;
esac
