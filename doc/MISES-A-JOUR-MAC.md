# Maintaining WSJT-Z / Fusion on Mac

Historical maintenance procedure established for WSJT-Z 2.0.19. Current Fusion
component status is in UPSTREAM-SOURCES.md; later personal application changes
must be tracked separately from this original maintenance baseline.

## Retained references

- Personal repository: https://github.com/Edpublic06/wsjt-fusion
- origin: personal repository; upstream: https://github.com/sq9fve/wsjt-z.git
- macos-2.0.19: reference source and initial documentation.
- macos-build-maintenance: rebuild script and records.
- Historical release tag: v2.0.19-macos-m1 (publication now retained as a draft).
- Normal installed application: /Applications/wsjtx.app.
- Script: doc/build-macos-preserve.sh.
- Verified rebuild: doc/macos-build-records/macos.9Ielpt/.

The initial test confirms startup/settings recovery. Detailed CAT, PTT, audio
and automation tests still need records.

## 1. Check original WSJT-Z changes

From the local repository:

```bash
git status
git fetch upstream
git log --oneline v2.0.19..upstream/master
```

Do not start with unsaved local changes. Fetch retrieves references without
merging. On 13 October 2026 no upstream/master commit followed v2.0.19.

## 2. Prepare a separate candidate

Read release notes and select an exact tag/commit. Use a new worktree/test
branch. Replace REF_VERIFIEE below with the selected reference and adjust names:

```bash
git worktree add -b macos-candidat-01 ../WSJT-Z-candidat-01 REF_VERIFIEE
```

Do not reuse an existing folder/branch or merge directly into macos-2.0.19.

## 3. Compare source

In the candidate:

```bash
git log --oneline v2.0.19..HEAD
git diff --stat v2.0.19 HEAD
git diff v2.0.19 HEAD -- doc/build-osx.sh CMakeLists.txt Darwin
```

Review full changes as needed, including macOS, Qt, compilers, Hamlib and
other library requirements.

## 4. Reuse Mac adaptations

The original maintenance adaptations covered generated-file exclusions in
.gitignore, doc/build-macos-preserve.sh and build/test documentation. At that
point no local application correction was documented. A candidate created
from upstream may lack these adaptations: inspect/import/adapt them explicitly
without unauthorized replacement of existing files.

For each later application fix, keep it if still relevant, adapt if upstream
changed, or drop if upstream solved the problem. Verify behavior even when a
patch applies without conflict.

## 5. Build without replacing the normal application

After checking/adapting the candidate script:

```bash
bash doc/build-macos-preserve.sh
```

The script creates build/macos.XXXXXX and places results there. It runs neither
brew update nor brew install. Homebrew dependencies remain shared, not isolated
or pinned. Review/authorize dependency changes separately.
Retain Terminal logs, versions, commit, local diffs and exact script. A commit
alone does not describe untracked files/all changes present during the build.

## 6. Back up before launch

Close WSJT-Z, Fusion and WSJT-X. Create a private new backup without overwriting
an earlier one. Locations observed on this machine:

- ~/Library/Preferences/org.k1jt.wsjtx.plist
- ~/Library/Preferences/WSJT-X.ini
- ~/Library/Preferences/org.k1jt.wsjtx.WSJT-X.plist
- ~/Library/Application Support/WSJT-X
- ~/Library/Application Support/ft2
- /Applications/wsjtx.app

This does not guarantee completeness for other versions/profiles. Check actual
paths. Do not publish preferences, credentials or contact logs.

## 7. Test the candidate

Do not launch both apps simultaneously: different bundles may share data.
Start with Auto CQ, Auto Call and Enable Tx disabled. Check in order:

1. Version/revision in About.
2. Settings recovery/stability.
3. Receive audio/decoding.
4. CAT frequency/mode reading and control.
5. PTT switching/release.
6. Short controlled transmit audio test.
7. Manual QSO/logging.
8. Automation separately, with stop verified.
9. Actually used companion applications.
10. Settings persistence after restart.

Record hardware, settings/results. Monitor the radio during automatic TX tests.

## 8. Return to the previous version

Stop TX/automation, close the candidate and relaunch:

```bash
open /Applications/wsjtx.app
```

Check settings/operation. If shared data changed, restoration may be needed:
close apps, back up failed state, obtain authorization before replacement.
Do not automatically merge configurations or ADIF logs.

## 9. Publish after validation

Retain previous builds. Create a new tag/release and DMG from each validated
candidate; do not overwrite older DMGs. Document upstream source/exact commit,
local fixes, script/environment, tested machine/macOS, test results/limits and
rollback procedure.

## 10. Separate task: integrate a new WSJT-X version

A WSJT-X update is not automatically a WSJT-Z update. The inherited baseline
uses WSJT-X 3.0.0. The initial maintenance study did not verify the proposed
3.3.0-beta1 source; subsequent inspection/imports are documented separately in
PLAN-3.3.0-beta1.md and UPSTREAM-SOURCES.md.

Before direct integration:

1. Verify official source/exact reference.
2. Identify WSJT-Z's WSJT-X baseline.
3. Compare upstream changes with Z-specific functions.
4. Create a dedicated branch/folder.
5. Resolve conflicts while preserving automation/extensions.
6. Adapt dependencies/Mac build.
7. Repeat functional/radio tests.

No automatic WSJT-X merge command is validated here. Integration requires
source analysis; changing a version number is insufficient.
