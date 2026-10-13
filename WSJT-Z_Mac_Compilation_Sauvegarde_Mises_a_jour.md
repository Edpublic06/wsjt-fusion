# WSJT-Z on Mac — building, backup, restoration and updates

Historical document prepared on 9 October 2026 from the conversation, then
supplemented by user confirmation. It records the original WSJT-Z build,
not the later Fusion integration. Existing filenames and archive paths are
retained so earlier procedures and links remain usable.

## 1. Scope and usage rules

- **Confirmed:** information/results evidenced in the conversation, including
  Terminal output supplied by the user.
- **Reviewed:** project script content examined during the conversation;
  describes behavior without establishing every installed tool version.
- **Proposed:** future procedure not executed or validated in that conversation.
- **Unknown:** information absent from available evidence.

Future procedures use new directories and must not delete, replace or modify
existing files without authorization. Proposed launches, tests, backups and
system changes were not performed when drafting this document.

The user later confirmed, in translation, “yes, this whole procedure works.”
That validates the build, application use and creation/opening of the DMG by
user report. CAT, PTT, decoding and automation outcomes were not separately
recorded. The checklist remains available for future versions. This does not
establish dependency versions or prove execution of every proposed backup or
restoration procedure.

## 2. Known machine and application state

| Item | Verifiable information | Status |
| --- | --- | --- |
| Computer | 13-inch MacBook Pro, Apple Silicon, 2020 | Screenshot confirmed |
| Memory | 16 GB | Screenshot confirmed |
| macOS | 15.8.1, build 24H212 | Screenshot confirmed |
| Previous application | User called it “wsjtx-z 2.6.A” | Confirmed; path and full identity unknown |
| Repository | `https://github.com/sq9fve/wsjt-z.git` | Confirmed |
| Downloaded version | Tag `v2.0.19` | Confirmed |
| About version | WSJT-Z `2.0.19 - b3a06e` | User screenshot confirmed |
| Source commit | `b3a06e89ae0174c51c8595d99e40ca179738eb0a` | Git confirmed |
| Declared base | WSJT-X 3.0.0 | Reviewed release notes |
| Source directory | `/Users/eddiefillia/Applications/WSJT-Z-build/wsjt-z` | Confirmed |
| Built application | `/Users/eddiefillia/Applications/WSJT-Z-build/wsjt-z/stage/wsjtx.app` | Confirmed |
| Working installed application | `/Applications/wsjtx.app` | User confirmed |
| Build date | 3 October 2026 | User confirmed; time unspecified |
| Application operation | User confirmed overall operation | Detailed test results not recorded |

The reviewed 2.0.19 release had two Windows installers and two source archives,
with no ready-to-install Mac app. Its Mac script supports arm64 and x86_64,
with macOS deployment target 11.0. These findings concern that release only.

## 3. Operation history

### 3.1 Commands actually reported

These reproduce steps confirmed by the user's Terminal output. The Git URL is
literal and directly usable in Terminal.

```bash
xcode-select --install
mkdir -p ~/Applications/WSJT-Z-build
cd ~/Applications/WSJT-Z-build
git clone --branch v2.0.19 --depth 1 https://github.com/sq9fve/wsjt-z.git
cd wsjt-z
```

Apple responded:

```text
Command line tools are already installed.
```

Git downloaded the sources and selected the commit in section 2. Detached HEAD
is expected when checking out a tag and is not a build error.

### 3.2 Build

The supplied command was:

```bash
bash doc/build-osx.sh
```

Its invocation is absent from the final excerpt, but its output establishes
completion of the build and bundle preparation:

```text
verified='1'
Verified 20 executable files in '/Users/eddiefillia/Applications/WSJT-Z-build/wsjt-z/stage/wsjtx.app'
fixup_bundle: done
codesign result: 0
Done. App bundle should be under: /Users/eddiefillia/Applications/WSJT-Z-build/wsjt-z/stage/wsjtx.app
```

“replacing existing signature”, printed below a line called “codesign error”,
does not establish failure: the reported exit result is 0. Apple notarization
was not documented and is not implied by this result.

### 3.3 Launch and user validation

Before the user's overall confirmation, suggested steps were to back up old
application data, close the old app, launch the new app, check About and
settings recovery, and adjust shared memory only if an error occurred.

Supplied launch command:

```bash
open ~/Applications/WSJT-Z-build/wsjt-z/stage/wsjtx.app
```

Execution of the conditional shared-memory commands and details of the initial
backup were not individually established. Do not present them as confirmed.
The user dates the build to **3 October 2026**. About shows **WSJT-Z — Version
2.0.19 - b3a06e**, matching the source commit prefix and identifying the app.

The user subsequently confirmed **/Applications/wsjtx.app** as the working
installed copy. Stage remains the build output; back up the installed copy
when preserving the app actually used. The copy method/date are undocumented.
For later use, launch the installed copy with:

```bash
open /Applications/wsjtx.app
```

## 4. Dependencies and versions

### 4.1 Dependencies in the reviewed script

The script uses Apple command-line tools and Homebrew:

```bash
brew install cmake ninja pkg-config gcc boost fftw hamlib libusb portaudio asciidoctor ccache qt@5
```

| Component | Build role | Known version at initial build |
| --- | --- | --- |
| Apple command-line tools | Development tools | Installed; version unknown |
| Homebrew | Dependency installation | Unknown |
| CMake | Configuration/installation | Unknown |
| Ninja | Build execution | Unknown |
| pkg-config | Library discovery | Unknown |
| GCC / gfortran | Fortran compiler | Unknown |
| Boost | C++ libraries | Unknown |
| FFTW | Fourier transforms | Unknown |
| Hamlib | Radio control | Unknown |
| libusb | USB access | Unknown |
| PortAudio | Audio interface | Unknown |
| Asciidoctor | Documentation tools | Unknown |
| ccache | Compilation cache | Unknown |
| Qt | Graphical UI, `qt@5` | Qt 5 family; exact version unknown |

The excerpts do not establish exact dependency versions. Successful completion
also does not prove every package was newly installed; some may already have
been present. See the separate later dependency inventory for observed versions.

### 4.2 Reviewed script parameters

The script detects architecture, finds Qt 5 and gfortran, then configures and
builds. This command requires variables prepared by the script and cannot be
used on its own:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="$CMAKE_ARCH" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET="$CMAKE_DEPLOY_TARGET" \
  -DCMAKE_Fortran_COMPILER="$GFORTRAN_BIN" \
  -DCMAKE_PREFIX_PATH="$QT5_PREFIX;$(brew --prefix)" \
  -DQt5_DIR="$QT5_PREFIX/lib/cmake/Qt5" \
  -DFFTW3_ROOT_DIR="$(brew --prefix fftw)" \
  -DHamlib_ROOT="$(brew --prefix hamlib)" \
  -DUsb_ROOT="$(brew --prefix libusb)" \
  -DWSJT_SKIP_MANPAGES=ON \
  -DWSJT_GENERATE_DOCS=OFF \
  -DCMAKE_INSTALL_PREFIX="$PWD/stage"
cmake --build build -j"$(sysctl -n hw.ncpu)"
cmake --install build
```

Expected Apple Silicon values are CMAKE_ARCH=arm64 and
CMAKE_DEPLOY_TARGET=11.0. The minimum target is not the host's macOS version.

### 4.3 Future inventory

**Proposed.** These commands read current state and write only to a new
folder. They establish state when run, not initial-build versions retroactively.

```bash
mkdir -p "$HOME/Applications/WSJT-Z-archives"
wsjtz_inventory=$(mktemp -d "$HOME/Applications/WSJT-Z-archives/inventaire.XXXXXX")
sw_vers > "$wsjtz_inventory/macos.txt"
uname -m > "$wsjtz_inventory/architecture.txt"
xcode-select -p > "$wsjtz_inventory/apple-tools-path.txt"
pkgutil --pkg-info com.apple.pkg.CLTools_Executables > "$wsjtz_inventory/apple-tools-version.txt" 2>&1
brew --version > "$wsjtz_inventory/homebrew-version.txt"
brew list --versions > "$wsjtz_inventory/homebrew-packages.txt"
brew info --json=v2 cmake ninja pkg-config gcc boost fftw hamlib libusb portaudio asciidoctor ccache qt@5 > "$wsjtz_inventory/dependances.json"
cmake --version > "$wsjtz_inventory/cmake-version.txt"
ninja --version > "$wsjtz_inventory/ninja-version.txt"
"$(brew --prefix qt@5)/bin/qmake" -query > "$wsjtz_inventory/qt.txt"
printf 'Inventory: %s\n' "$wsjtz_inventory"
```

If brew is missing, stop and check its path; the original script anticipates
/opt/homebrew/bin/brew or /usr/local/bin/brew. Empty/error files are not valid
version evidence.

## 5. Source changes

No local source correction was documented in the initial conversation.
Downloading the tag and building are established; no manual edit or patch was
reported. macOS changes in upstream release notes are not user corrections.
Building/signing the bundle is not a local source change.
Changes outside the conversation remain possible. Future backups record Git
status/diff; inspect untracked files too, since a simple Git patch omits them.

## 6. Problems and solutions

| Observation | Established diagnosis | Solution / status |
| --- | --- | --- |
| No Mac installer in reviewed release | Source and Windows installers available | Mac script build completed |
| Apple tools already installed | Informational message | No reinstall needed at that step |
| Detached HEAD | Tag selected | No correction needed |
| Replacing existing signature | Signature replaced; result 0 | No failure established |
| Low shared-memory limits | Script warning | Proposed adjustment only for launch errors |

Reported values:

```text
kern.sysv.shmmax=52428800
kern.sysv.shmall=25600
```

Script's recommended minimums:

```text
kern.sysv.shmmax>=134217728
kern.sysv.shmall>=32768
```

**Proposed, not executed; authorization required because system state changes.**
Only if launch fails with QSharedMemory or system-imposed size restrictions:

```bash
sudo sysctl -w kern.sysv.shmmax=134217728
sudo sysctl -w kern.sysv.shmall=32768
```

These settings last until restart. No persistent configuration mechanism was
established. Do not apply automatically or assume old values remain unchanged.

## 7. Back up current state

### 7.1 Identify what must be preserved

**Proposed.** Run section 10's tests before describing a copy as working.
Keep separately: the old 2.6.A app if still working; the installed 2.0.19 and
stage output; settings, contact logs and other data for each app; complete
source/local corrections; tool versions, build logs and test results.
The initial conversation did not identify configuration paths or the old app's
path. Identify them through the app and Finder rather than guessing. File-menu
data folders may not include all settings; verify completeness before migration.
Close apps before copying data to avoid partially written backups.

### 7.2 Archive application and source

**Proposed.** Each run creates a new folder without replacing earlier archives.
Stop on errors and verify each archive before continuing.

```bash
mkdir -p "$HOME/Applications/WSJT-Z-archives"
wsjtz_backup=$(mktemp -d "$HOME/Applications/WSJT-Z-archives/sauvegarde-2.0.19.XXXXXX")
wsjtz_source="$HOME/Applications/WSJT-Z-build/wsjt-z"
ditto -c -k --sequesterRsrc --keepParent "$wsjtz_source/stage/wsjtx.app" "$wsjtz_backup/wsjtx-2.0.19.app.zip"
ditto -c -k --sequesterRsrc --keepParent /Applications/wsjtx.app "$wsjtz_backup/wsjtx-2.0.19-installee.app.zip"
ditto -c -k --sequesterRsrc --keepParent "$wsjtz_source" "$wsjtz_backup/sources-et-construction.zip"
git -C "$wsjtz_source" rev-parse HEAD > "$wsjtz_backup/commit.txt"
git -C "$wsjtz_source" status --short > "$wsjtz_backup/git-status.txt"
git -C "$wsjtz_source" diff --binary HEAD > "$wsjtz_backup/modifications-suivies.patch"
git -C "$wsjtz_source" ls-files --others --exclude-standard > "$wsjtz_backup/fichiers-non-suivis.txt"
shasum -a 256 "$wsjtz_backup/wsjtx-2.0.19.app.zip" "$wsjtz_backup/wsjtx-2.0.19-installee.app.zip" "$wsjtz_backup/sources-et-construction.zip" > "$wsjtz_backup/SHA256.txt"
printf 'Backup: %s\n' "$wsjtz_backup"
```

The complete archive includes Git-ignored files and may be large. Build folders
may contain absolute paths and are not assumed reusable elsewhere. Rebuild in
a fresh directory.

### 7.3 Add data and the old application

**Proposed.** In the same Terminal session, enter the confirmed absolute data
folder path without adding quotes to the response:

```bash
printf 'Confirmed absolute data folder path: '
IFS= read -r wsjtz_data_path
if [ -d "$wsjtz_data_path" ]; then
  ditto -c -k --sequesterRsrc --keepParent "$wsjtz_data_path" "$wsjtz_backup/donnees.zip"
  printf '%s\n' "$wsjtz_data_path" > "$wsjtz_backup/emplacement-donnees.txt"
else
  printf 'Folder not found: no data backup was made.\n'
fi
```

Repeat in a fresh folder for other confirmed configuration locations. Copy the
old application in Finder or make a separate ditto archive after confirming its
path; never substitute the 2.0.19 path for 2.6.A.
Add this document, section 4.3's inventory, available Terminal logs, settings
screenshots and test records. Record audio devices, radio model, CAT/PTT
settings, ports and companion apps; these were unknown in this record.
Copy to a second medium in a new folder and verify SHA-256 after transfer.
Protect configuration archives, which may contain credentials.

## 8. Reproduce the current version

### 8.1 Reproducibility limits

Tag/commit are known; initial tool versions are not. The original script runs
brew update and installs currently available formulas, so later builds may use
different versions and produce different results. The bundle archive preserves
the built app. Source, dependency inventories and logs help rebuilding but do
not guarantee identical bytes or future availability of old dependencies.

### 8.2 Rebuild in a fresh location

**Proposed.** Download sources without modifying the current repository:

```bash
mkdir -p "$HOME/Applications/WSJT-Z-reconstructions"
wsjtz_rebuild=$(mktemp -d "$HOME/Applications/WSJT-Z-reconstructions/version-2.0.19.XXXXXX")
git clone --branch v2.0.19 --depth 1 https://github.com/sq9fve/wsjt-z.git "$wsjtz_rebuild/source"
git -C "$wsjtz_rebuild/source" rev-parse HEAD
```

The commit must be exactly:

```text
b3a06e89ae0174c51c8595d99e40ca179738eb0a
```

Read the script before executing:

```bash
cat "$wsjtz_rebuild/source/doc/build-osx.sh"
```

**Authorization required:** this script may install Homebrew, refresh metadata
and install/change existing dependencies. A separate source folder does not
isolate Homebrew. After authorization:

```bash
cd "$wsjtz_rebuild/source"
bash doc/build-osx.sh > "$wsjtz_rebuild/compilation.log" 2>&1
wsjtz_build_status=$?
printf 'Exit status: %s\n' "$wsjtz_build_status"
tail -n 40 "$wsjtz_rebuild/compilation.log"
```

Output goes to a log rather than the screen. Interactive prompts may require
session inspection; do not leave this unattended. If exit status is not 0,
inspect the log before launching. Review any backed-up local corrections and
apply them only to the new copy with authorization.

## 9. Prepare a future update

### 9.1 Backup and download

**Proposed.** Validate/archive the current app first. Read releases and record
the exact new tag; never assume a future number.

```bash
printf 'Exact new version tag: '
IFS= read -r wsjtz_new_tag
mkdir -p "$HOME/Applications/WSJT-Z-candidats"
wsjtz_candidate=$(mktemp -d "$HOME/Applications/WSJT-Z-candidats/candidat.XXXXXX")
git clone --branch "$wsjtz_new_tag" https://github.com/sq9fve/wsjt-z.git "$wsjtz_candidate/source"
git -C "$wsjtz_candidate/source" rev-parse HEAD > "$wsjtz_candidate/commit-candidat.txt"
git -C "$wsjtz_candidate/source" show-ref --tags > "$wsjtz_candidate/tags.txt"
```

This clone is not shallow so history remains available for comparison. Stop if
cloning fails; do not continue with an incomplete folder.

### 9.2 Compare with the current version

```bash
git -C "$wsjtz_candidate/source" log --oneline v2.0.19..HEAD > "$wsjtz_candidate/commits-depuis-2.0.19.txt"
git -C "$wsjtz_candidate/source" diff --stat v2.0.19 HEAD > "$wsjtz_candidate/resume-changements.txt"
git -C "$wsjtz_candidate/source" diff v2.0.19 HEAD -- doc/build-osx.sh CMakeLists.txt Darwin > "$wsjtz_candidate/changements-compilation-macos.diff"
git -C "$wsjtz_candidate/source" diff v2.0.19 HEAD > "$wsjtz_candidate/changements-complets.diff"
cat "$wsjtz_candidate/source/doc/build-osx.sh"
```

These compare upstream source. Also review release notes, changelog and the
saved local patch. If v2.0.19 is unavailable, stop and obtain a reference copy
in a new folder; do not interpret failed comparison as an empty diff.

### 9.3 Decide what to do with earlier corrections

No local correction was established in the initial record. For later corrections:

| Correction | Problem/evidence | Files | Upstream status | Decision | Validation test |
| --- | --- | --- | --- | --- | --- |
| None documented in initial record | — | — | — | Apply nothing automatically | — |

Keep a correction if the problem and fix remain relevant; adapt it if code or
dependencies changed; drop it if upstream fixed the problem or made it obsolete.
A conflict-free patch is not necessarily correct. Test the intended behavior.
Changes require authorization and must remain limited to the candidate copy.

### 9.4 Dependencies and build

Compare dependency lists, Qt major version, compilers, macOS target, CMake
options, signing and output paths; do not blindly reuse section 4.
Before dependency installation/update, request authorization and retain the
old inventory. Homebrew changes may affect the previous application too;
archive its bundle and verify independent launch.
After review/authorization, run the new version's script in its own directory,
logging as in section 8.2. If renamed/missing, find that version's instructions;
no replacement script is established here. Keep candidate and working bundles
at separate, clearly identified paths.

## 10. Test without compromising the working version

### 10.1 Isolation limits

Different bundles can share settings/logs. **A fresh build folder does not
isolate user data.** No separate-profile option was established in the initial
conversation. Back up all data and authorize possible changes before launch.
A separate macOS test account can offer stronger isolation; its creation
requires authorization and was not verified here. Never run both versions
simultaneously against the same radio.

### 10.2 Suggested validation order

| Step | Action | Success criterion |
| --- | --- | --- |
| Identity | Check About and running bundle path | Correct version/candidate |
| Startup | Observe messages, windows, stability | No crash/memory error |
| Settings | Compare reference screenshots | Callsign, modes, ports/settings consistent |
| Receive audio | Usual input and known activity | Visible level, consistent decodes |
| CAT | CAT test, then frequency/mode check | Correct reading/control |
| PTT | Brief test on a suitable setup | Correct TX/RX switching/release |
| Transmit audio | Short controlled transmission | Correct output/radio behavior |
| Manual QSO | Controlled contact | Correct sequence, stop, logging |
| Automation | Auto CQ/Call and used functions separately | Correct selection, chaining, stop/watchdog |
| Integrations | Actually used companion apps | Correct communication |
| Restart | Relaunch/check retained settings | Correct persistence |

Initially disable Auto CQ, Auto Call and Enable Tx. Verify stopping transmission
before automation tests and monitor the radio. Exact menu labels, hardware,
ports and settings were unknown here; record them from the actual version.
Record version, commit, date, hardware, settings and symptoms for each result.
Declare the update working only after validating functions needed in practice.

## 11. Rollback and restoration

### 11.1 Immediate rollback

**Proposed.** Stop automation/transmission, close the candidate, relaunch the
previous bundle, and check settings, CAT, reception and PTT release. If shared
data changed, close the old app and restore below with authorization.
Reliable rollback requires retaining the old bundle and compatible or isolated
data. Without identified/backed-up configuration, no guarantee can be made.

### 11.2 Extract archives without overwriting existing files

**Proposed.** Enter the real backup directory:

```bash
printf 'Absolute backup folder path: '
IFS= read -r wsjtz_archive_path
shasum -a 256 -c "$wsjtz_archive_path/SHA256.txt"
```

Continue only if hashes match. Section 7's manifest uses absolute paths; if
moved, compute hashes for the three ZIPs at their new location and compare
recorded values without editing the original manifest.

```bash
mkdir -p "$HOME/Applications/WSJT-Z-restaurations"
wsjtz_restore=$(mktemp -d "$HOME/Applications/WSJT-Z-restaurations/restauration.XXXXXX")
ditto -x -k "$wsjtz_archive_path/wsjtx-2.0.19-installee.app.zip" "$wsjtz_restore/application"
printf 'Extracted application: %s/application/wsjtx.app\n' "$wsjtz_restore"
```

Extract data to a fresh directory too:

```bash
ditto -x -k "$wsjtz_archive_path/donnees.zip" "$wsjtz_restore/donnees"
```

Extraction does not yet restore active data. Inspect results and the saved
emplacement-donnees.txt path. **Replacing active configuration requires
authorization.** Close the app and archive failed state in a new folder first.
Do not automatically merge configurations or ADIF logs.
For offline rebuilding, extract sources-et-construction.zip to another new
folder. Dependencies/downloaded resources may still be missing, so retaining
the working bundle is essential.

## 12. Information still to record after validation

- Exact dependency and Apple tool versions.
- Preserve About screenshot confirming 2.0.19 - b3a06e; user dates build to 3 October 2026.
- Old 2.6.A bundle path if retained; installed 2.0.19 path is /Applications/wsjtx.app.
- Complete settings/data locations.
- Radio, interface, CAT/PTT ports, audio devices/settings.
- Manual/automatic test results.
- Local corrections with patches/rationale.
- Secondary backup location and restoration verification.

## 13. References already reviewed in the conversation

These retrieve source references; no additional research was used to prepare
the original record.

- [WSJT-Z releases](https://github.com/sq9fve/wsjt-z/releases)
- [2.0.19 assets](https://github.com/sq9fve/wsjt-z/releases/expanded_assets/v2.0.19)
- [Project repository](https://github.com/sq9fve/wsjt-z)
- [v2.0.19 build script](https://github.com/sq9fve/wsjt-z/blob/v2.0.19/doc/build-osx.sh)

Review proposed procedures for each new version. Keep confirmed historical
results separate from future results.

## 14. Create a DMG of the compiled application

**User-validated procedure:** the user confirmed the procedure works after it
was added. No detailed creation log was supplied; validation rests on that
report. The image preserves/transfers the app, not configuration, contact logs,
source or dependencies.

### 14.1 Creation

Close the new app. If WSJT-Z-2.0.19-macOS-Apple-Silicon.dmg already exists on the
Desktop, choose a new name; do not delete/replace it. Execute:

```bash
hdiutil create -volname "WSJT-Z 2.0.19" -srcfolder "$HOME/Applications/WSJT-Z-build/wsjt-z/stage/wsjtx.app" -format UDZO "$HOME/Desktop/WSJT-Z-2.0.19-macOS-Apple-Silicon.dmg"
```

-volname sets the mounted volume name; -srcfolder identifies the compiled
bundle; -format UDZO requests compression; the last path names the new image.

### 14.2 Verification and retention

Open the DMG and verify the app is present. The user confirmed this procedure.
For future images, retain Terminal output and verification results too.
Keep the DMG with section 7's backups and on the second medium. This does not
validate radio operation or add Apple notarization.
For future versions, update version/volume name, validated candidate bundle
path and output filename. Always create a new image and retain the previous one.
