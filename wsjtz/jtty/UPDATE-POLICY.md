# Tracking JTTY across WSJT-X releases

This policy applies to **v3.3.0-beta1 and later releases**, including subsequent
betas, release candidates and stable versions. It defines a repeatable update
procedure, not a scheduled monitor or automatic importer.

## Current immutable source reference

Official repository: https://github.com/WSJTX/wsjtx

Tag: **v3.3.0-beta1**

Commit: **57ba9337fa020065dc0ceee2c4ef85e433ea32d4**

| Group | Inventory | Integration status | Update rule |
| --- | --- | --- | --- |
| Codec, FEC, waveform and receive core | `source-manifest.json` | Imported, compiled and tested | Compare pristine source hashes first; port changes into the compiled paths deliberately. |
| UI, continuous audio, transmit lifecycle and integration context | `ui-audio-source-manifest.json` | Unchanged reference snapshot; not compiled as a second application | Compare the reference tree, then adapt selected changes into Fusion. Never replace Fusion's entire main window or configuration. |
| Stateful audio FIR | `live/source-manifest.json` | Extracted private routine from upstream `lib/fil4.f90` | Compare original source hash and preserve private state/symbol adaptation. |
| Personal application adapter | `ui/WavReceiveDialog.*` | Offline WAV panel | Fusion-owned code; adapt to upstream interfaces without attributing it to upstream. |
| Personal build adapter | `CMakeLists.cmake`, application CMake/menu wiring | Fusion integration | Review whenever source lists, dependencies or upstream APIs change. |

Every upstream entry records its original path, destination, SHA-256 and
adaptation status. All inventories must be checked, including the adapted FIR source in `live/source-manifest.json` (original-source hash and adapted-file hash are separate). Preserve source notices,
COPYING, AUTHORS and NOTICE.md. The two groups can advance independently;
record each group's actual integrated revision rather than claiming the whole
application uses the latest upstream release.

## Read-only check

From the Fusion worktree:

```bash
python3 tools/check_jtty_upstream.py
```

To compare a candidate already fetched into the official local checkout:

```bash
python3 tools/check_jtty_upstream.py --upstream /path/to/wsjtx --ref VERIFIED_TAG_OR_COMMIT
```

The tool verifies the retained local files, resolves the candidate to an exact
commit and reports modified, deleted and newly discovered JTTY/audio-stream
files. It changes neither source files nor Git references and performs no network
fetch. It is an inventory aid, not a complete dependency or compatibility analysis.

## Procedure for each future upstream version

1. Keep the last working Fusion tag and binary; create a separate update branch.
2. Fetch the official release, read its notes and resolve its exact commit.
3. Run the read-only check and retain its report with the update record.
4. Review changed/deleted files, new tests and possible renames. Inspect shared
   engine contracts, main window, Detector/Modulator, configuration and CMake;
   their evolution may affect JTTY even without a JTTY-named file change.
5. Update the pristine reference snapshot in an isolated commit. Update its
   manifest with tag, commit, date and hashes. Do not mark reference-only code
   as integrated.
6. Port the selected changes into compiled code and adapters in separate commits.
   Record local patches, rationale, source commits, dependency changes and licenses.
   Preserve Fusion automation, FT2 and settings compatibility.
7. Run upstream codec/RX/TX tests as applicable, unchanged-WAV comparisons,
   continuous-audio lifecycle checks, stop/PTT checks, and FT8/FT4/FT2/automation
   regressions. Validate live audio/radio with the operator.
8. Record the last **examined** upstream version separately from the last
   **integrated** revision for each group. Publish a new Fusion prerelease only
   when its matching binary/source and test limitations are documented.

## Update record template

| Field | Value to record |
| --- | --- |
| Candidate upstream tag / exact commit | |
| Previously examined / integrated reference | |
| Scope: core, UI/audio/TX, shared dependencies | |
| Changed, added, removed or renamed paths | |
| Selected / deferred / rejected changes and rationale | |
| Pristine snapshot commit and manifest | |
| Fusion adapter commits / local patches | |
| Tests, hardware, results and remaining limitations | |
| Published Fusion version / corresponding source commit | |

## Protected rollback baseline

Fusion **v1.0.0-beta2** remains at
`8345c155a878185f11736e9dec8df278ab7f1c49`, also referenced by `backup/beta2`.
Never move that tag or overwrite its release assets for a JTTY update.
