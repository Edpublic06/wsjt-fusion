# Proposed native macOS ARM64 beta3 CI

Status: local proposal; no workflow has been published or run. Development
artifact only, not a release. Target branch: `beta3-remote`; required ancestor:
`3f9d41dcc761f820bf09a8974b94c795a972de9c`.

## Workflow

`.github/workflows/macos-arm64.yml` temporarily accepts only pushes to
`beta3-remote` that change at least one of these four exact paths:

- `.github/workflows/macos-arm64.yml`
- `tools/ci/macos-arm64.sh`
- `tools/ci/verify_macos_bundle.py`
- `doc/CI-MACOS-ARM64.md`

There are no wildcard paths or tags. Branch and path conditions both must
match. A push changing only application sources does not trigger this workflow.
The workflow runs on the `macos-15` standard Apple Silicon runner. It rejects
another repository/branch, an Intel host, or a checkout that lacks the reference
commit. No workflow_dispatch, pull-request, schedule, release or write permissions
are configured. Checkout uses the exact event SHA, including on a GitHub rerun,
so later pushes cannot change the sources of an earlier run.

The build runs outside the checkout in `$RUNNER_TEMP/fusion-macos`. It builds
every default target, runs every registered test (at least 25, including both
isolated-engine regressions), installs into a fresh stage, invokes macdeployqt,
checks the bundle, and ad-hoc signs it. No radio/audio-device smoke test runs.
The CMake tests use synthetic/offline signals and Qt's offscreen platform.
The disposable runner's SysV shared-memory limits are raised with sudo if
necessary (shmmax >=128 MiB, shmall >=32768 pages), following the existing local
build preflight. No persistent launch daemon is installed; inability to change
these limits is reported as an environment failure, not skipped tests.

Existing `doc/build-macos-preserve.sh` documents fresh local builds without
updating dependencies. `tools/build_beta2_decoder.py` builds the pinned upstream
engine. `bundle_fixup/CMakeLists.txt` supplies fresh-install dependency fixup.
The CI uses these CMake install rules, followed by macdeployqt and a strict
audit. It does not use `package_fusion_beta3.py` or `refresh_macos_test_app.py`,
which require a previous application bundle unavailable on a clean runner.

The artifact tarball preserves application executable permissions and framework
symlinks, and contains `wsjtx.app`, logs, dependency versions, the tested SHA,
JUnit results and warnings. Diagnostics upload even if a step fails; the app
uploads only if build, tests, deployment and signature verification succeed.
Retention: seven days. Job timeout: 120 minutes. No cache of compiled objects.
Artifact names include both run ID and attempt number, avoiding collisions when
rerunning a job. GitHub's Re-run jobs function can repeat the first execution
without a new source commit.

## Standard runner resource budget

GitHub currently documents the public macOS ARM64 standard runner as 3 M1 CPU
cores, 7 GB RAM and 14 GB SSD. These specifications are not a measurement of
free resources on the future VM. Before installation, after dependencies and
before packaging, the job records actual free disk, physical memory, df and
vm_stat. The logs also record final disk/memory state on success or failure.

Conservative preflight thresholds are 10 GiB free before dependency installation,
6 GiB afterward, and 2 GiB before packaging, plus at least 6 GiB physical RAM.
These are engineering margins for downloads, dependency extraction, both build
trees, staged frameworks and the artifact tarball; they are not measured peak
requirements. Insufficient resources stop the job, without deleting Xcode or
other runner software, enlarging the runner or altering application sources.

Homebrew must use native binary bottles (`--force-bottle` and
`HOMEBREW_NO_BOTTLE_SOURCE_FALLBACK=1`). Qt, Boost and GCC are not compiled from
source on this runner. If a bottle is unavailable or incompatible, installation
fails and requires review. A source build of Qt and its dependencies is not
assumed to fit this runner's RAM, disk or 120-minute time budget.

Application compilation is capped at two jobs. A generated toolchain file outside
the repository applies Ninja compile=2/link=1 pools to both CMake projects,
including the isolated engine script that otherwise invokes Ninja with -j6.
The engine is built first, avoiding simultaneous outer and nested compilations.
Tests run serially, still executing every registered test. No application build
script, radio algorithm or platform-specific source is changed by these limits.

## Dependencies and versions

| Component | Proposed selection | Evidence/limits |
|---|---|---|
| macOS | `macos-15`, native ARM64 | Explicit architecture check; no Rosetta. |
| C/C++ | Apple Clang from the runner's selected Xcode | Record exact Xcode/SDK/compiler; keep libc++. Local JTTY targets already use C++14/17. |
| Fortran/OpenMP | Homebrew `gcc@14`, `gfortran-14` | GNU runtime/libgomp required by current Apple CMake rules; existing source supports GNU Fortran. |
| CMake | 3.31.6, installed in an isolated Python venv | Same CMake release tested on Linux; no change to project minimum. |
| Qt | Homebrew `qt@5`, currently 5.15.19 | Core, Widgets, SerialPort, Multimedia, PrintSupport, Sql, LinguistTools, WebSockets, Test; qmake/macdeployqt required. |
| Boost | Homebrew `boost` | CMake requires >=1.62; engine also links log_setup/log/thread/filesystem/chrono/atomic. Preflight fails if these libraries disappear. |
| FFTW | Homebrew `fftw` | Single/double precision and thread variants for the isolated engine. |
| Hamlib | Homebrew `hamlib` | Headers, library and rigctl/rigctld; exact version logged. No reliable minimum declared by this project. |
| USB/audio | Homebrew `libusb`, `portaudio`; Qt Multimedia | All must be native ARM64; no new application dependency. |
| Other tools | Ninja, pkgconf, Python 3, Apple codesign/otool/lipo | Homebrew versions and formula metadata recorded. |

Historical inventory (13 October) lists Qt 5.15.19, Boost 1.92.0, FFTW 3.3.11,
Hamlib 4.7.2, libusb 1.0.30, PortAudio 19.7.0 and GCC 16.2.0. It is not an exact
lockfile for e2186c8 and does not prove availability/compatibility of each version
on Actions. This proposal records actual resolved versions rather than claiming
byte-for-byte reproduction. GCC 14 is a deliberate, versioned CI selection,
not a claim about the original compiler.

Qt 5's Homebrew formula currently has an ARM64 Sequoia bottle (5.15.19), is
deprecated since 2026-05-19, and announces disablement on 2027-05-19. If this or
another dependency becomes unavailable, stop and propose either a reviewed
versioned tap/bottle or a source-built Qt 5.15 toolchain. Do not silently migrate
to Qt 6 or alter application/decoder sources. The first run may also reveal
Boost/Hamlib API or plugin packaging changes. Stop and report those before
changing sources. Neither native runtime compatibility nor packaging has yet
been tested on this runner.

## Bundle checks and remaining limits

The audit requires wsjtx, jt9, fusion-jtty-rx, wsprd, the isolated decoder dylib,
ALLCALL7.TXT, cty.dat, USState.db, application translations, qt.conf, Cocoa,
SQLite and audio plugins, and license notices. It checks ARM64 in each Mach-O
and follows non-system dynamic dependencies, including dlopen-loaded libraries
and Qt plugins. It rejects missing files and dependencies left in Homebrew or
the runner/build directories. Full otool inspection covers OpenGL/system
framework references; no Linux Mesa paths or Windows libraries are added.

Compiler warnings remain visible. `memory-risk-warnings.txt` highlights
uninitialized variables, bounds/overflow, truncation, static storage/concurrency,
executable stack and ABI/alias concerns for review. This is an aid, not proof
that all risks have been detected. Logs retain full multiline diagnostics.

Signing is ad-hoc, without Apple Developer secrets or notarization. The nominal
deployment target remains macOS 11.0 as in CMake; modern Homebrew libraries may
require a newer macOS. A successful Actions run validates that runner's OS, not
all older Macs. Real audio, CAT/PTT, live QSOs and UI operation still require
operator validation. Linux and future Windows application paths are untouched.

## First launch, reruns and costs

The first push adding the four files to beta3-remote satisfies the branch and
path filters and triggers this single-job workflow from the pushed commit.
Unlike workflow_dispatch, this push trigger does not require placing the file
on the default branch. The default branch stays
`wsjtz-3.3.0-beta1-integration`; no repository setting is changed. Actual execution
still requires GitHub Actions and the referenced actions to be enabled/allowed
for the repository and a runner to be available. There is no promise of success
before a first native run. No other workflow file currently exists in this
checkout; this does not control external integrations or organization policies.

After the first launch, use GitHub Actions Re-run jobs (or Re-run all jobs) on
that run. It uses the same event/ref/SHA and fresh runner, with a distinct
artifact attempt suffix. Do not modify sources merely to repeat a test.

The repository is currently public. Standard GitHub-hosted runners are
generally free for public repositories.
For private repositories, included allowances, macOS billing and artifact
storage depend on the account plan and budgets; larger/xlarge runners have
separate charges and are not selected here. No billed estimate is guaranteed
without reading the account's current plan. The workflow has a 120-minute cap
and seven-day artifacts, but Homebrew installation/source builds can be slow
and a timeout is still a failed run. No job has been launched or cost incurred
by preparing these files.

References checked during preparation:

- https://github.com/actions/runner-images#available-images
- https://github.com/actions/runner-images/blob/main/images/macos/macos-15-arm64-Readme.md
- https://github.com/Homebrew/homebrew-core/blob/master/Formula/q/qt@5.rb
- https://docs.github.com/en/actions/reference/workflows-and-actions/events-that-trigger-workflows#push
- https://docs.github.com/en/actions/reference/runners/github-hosted-runners
- https://docs.github.com/en/billing/concepts/product-billing/github-actions

## Local review

Validate shell syntax with `bash -n tools/ci/macos-arm64.sh` and parse the
workflow YAML and Python AST without executing macOS commands. Actual macOS
build/test/package results remain pending publication and launch approval.
These syntax checks pass locally. Synthetic checks of the bundle auditor
accept a complete mocked ARM64 bundle and reject Intel images, external
dependencies and a missing Cocoa plugin. They do not replace a native run.
