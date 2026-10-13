# WSJT-Fusion — source and adaptation register

Recorded on 13 October 2026. This register distinguishes code actually present
in the baseline from downloaded references and integration work in progress.

<a id="tableau-des-versions-par-fonction"></a>

## Component versions by function

**Application version: WSJT-Fusion 1.0.0-beta2.** This is the fork's own version;
it does not describe every decoder's version.

| Function / component | Source project | Reference version | Revision / evidence | What is actually present in Fusion |
| --- | --- | --- | --- | --- |
| Application baseline, UI, filters, historical Auto CQ / Auto Call | WSJT-Z, SQ9FVE and contributors | **2.0.19** | [Original tag](https://github.com/sq9fve/wsjt-z/tree/v2.0.19), commit `b3a06e89ae0174c51c8595d99e40ca179738eb0a` | Baseline retained with adaptations and personal features; Fusion is not an official WSJT-Z release. |
| Inherited general engine | WSJT-X through WSJT-Z | **3.0.0** (declared base) | [`Versions.cmake`](Versions.cmake); baseline snapshot above | Still used. This does not mean every file matches WSJT-X 3.0.0: WSJT-Z has its own modifications. |
| FT8 decoding and multithreading | WSJT-X + WSJT-Z adaptations, JTDX contributions | WSJT-X base **3.0.0**; WSJT-Z **2.0.19**; JTDX version **not established** | [`lib/ft8`](lib/ft8), [`lib/decoder.f90`](lib/decoder.f90), baseline history | Inherited engine with specific improvements. **Full migration to WSJT-X 3.3.0-beta1 FT8 is unfinished.** Do not assign a precise JTDX version without evidence. |
| Standard FT4 | WSJT-X through WSJT-Z | Declared base **3.0.0**, Z baseline **2.0.19** | [`lib/ft4_decode.f90`](lib/ft4_decode.f90), baseline history | Standard FT4 retained; the recent private FT2 import does not replace this decoder. |
| FT2 updated receiver | WS / WSJT-X Improved and contributors | **WS 3.2.1, build 261010** | Archive `ws-3.2.1_261010.tgz`; [file/hash manifest](wsjtz/decoding/ft2/source-manifest.json); Fusion import [`095e907`](https://github.com/Edpublic06/wsjt-fusion/commit/095e907) | Receiver adapted from the FT4 sources used for FT2, in a private module. Build and synthetic tests passed; this does not certify a complete import of WS features. |
| FT2 inherited transmitter and application behavior | WSJT-Z, initial port declared from Improved | Z baseline **2.0.19**; initial Improved revision **not established** | Port `ec2a6037502bb177e93996d2224c97866b146a5b`; 3.75 s timing correction `c467f6653c5f79d0122fcac411b3caf27f223e2e` | Inherited transmission, mode selection, UI and operation with Fusion adapters. Historical experimental `lib/ft2` does not alone represent the active FT2 implementation. |
| JTTY codec / receive core | WSJT-X and contributors | **3.3.0-beta1** | [Upstream tag](https://github.com/WSJTX/wsjtx/tree/v3.3.0-beta1), commit `57ba9337fa020065dc0ceee2c4ef85e433ea32d4`; [manifest](wsjtz/jtty/source-manifest.json) | Core imported; six upstream tests passed. **Application UI, audio and transmission are not connected: the operating mode is not yet available.** |
| Target general engine update | WSJT-X | **3.3.0-beta1** | Same upstream tag/commit; [integration plan](doc/PLAN-3.3.0-beta1.md) | Working reference. **General integration remains in progress**, separate from the imported JTTY core. |
| Other inherited modes: JT4 / JT9 / JT65, Q65, MSK144, WSPR, Echo, FreqCal | WSJT-X through WSJT-Z | Declared base **3.0.0**, Z baseline **2.0.19** | Baseline history; precise file inventory still to complete | Inherited functions. No full 3.3.0-beta1 port of these modes is claimed. |
| Incoming-caller priority, deduplicated queue, WD Counter | WSJT-Fusion, F6FDY | **1.0.0-beta1** | [`wsjtz/automation`](wsjtz/automation), Fusion history | Personal extensions. First-caller protection [`8a28ecc`](https://github.com/Edpublic06/wsjt-fusion/commit/8a28ecc), incoming-filter preference [`b34f533`](https://github.com/Edpublic06/wsjt-fusion/commit/b34f533), receive-period handover resolution [`911456c`](https://github.com/Edpublic06/wsjt-fusion/commit/911456c). Automated tests passed; radio testing continues. |

“Declared base” identifies the baseline's provenance, not a complete file-by-file
comparison with an upstream tag. Unknown versions remain explicitly unknown.
Manifests provide finer traceability than the displayed application version.

## Immutable project references

| Source | Immutable reference | Status |
| --- | --- | --- |
| Original WSJT-Z | `sq9fve/wsjt-z`, `v2.0.19`, `b3a06e89ae0174c51c8595d99e40ca179738eb0a` | Application baseline present. |
| Edpublic06 macOS maintenance | Historical `Edpublic06/wsjt-z`, `macos-build-maintenance`, `e850c68` | Documentation and procedure present; no application change from tag Z at that point. |
| WSJT-X | `WSJTX/wsjtx`, `v3.3.0-beta1`, `57ba9337fa020065dc0ceee2c4ef85e433ea32d4` | JTTY core imported and tested; UI/audio connection and general engine migration unfinished. |
| WS / Improved | `ws-3.2.1_261010.tgz` | FT2 receiver integrated in a private module; macOS build and initial synthetic tests passed. |

Official WS archive:
https://sourceforge.net/projects/wsjt-x-improved/files/WS_v3.2.1/Source%20code/ws-3.2.1_261010.tgz/download

Downloaded archive SHA-256:
`b343f440baec95c6cf6eaf3766802c4c6533a2b3129fbecf23d9a096daf2a0d5`.
Application sources are inside nested archive `src/ws.tgz`.

## Baseline FT2

- Declared Improved introduction: `ec2a6037502bb177e93996d2224c97866b146a5b`.
- 3.75 s period correction: `c467f6653c5f79d0122fcac411b3caf27f223e2e`.
- Original active receiver: `lib/decoder.f90`, `nmode=52`, FT4 engine and
  `ft2_decoded` callback.
- Transmitter and controls: `widgets/mainwindow.*`, `Modulator`, modes,
  frequencies, display and logging.
- Historical `lib/ft2` also contains a different experimental implementation;
  its contents alone do not identify the receiver used by the application.
- Initial Improved revision and precise JTDX revision are not established.

## Actual imports and adaptations

First FT2 batch, modified 13 October 2026:
`wsjtz/decoding/ft2/upstream/` contains derived sources with attribution,
dates and private names. Original file hashes and the symbol mapping are in
`wsjtz/decoding/ft2/source-manifest.json`. Adapters changed:
`lib/decoder.f90`, `lib/jt9.f90`, `widgets/mainwindow.cpp`, `CMakeLists.txt`.
See `wsjtz/decoding/ft2/README.md` for adaptations and validation.

Personal Auto Call extension, 13 October 2026:
`wsjtz/automation/IncomingCallPolicy.hpp` and adapters in
`widgets/mainwindow.{cpp,h,ui}`. Disabled by default; an unanswered outgoing
attempt can yield to an eligible direct caller, the accepted QSO is protected,
and the normal Auto Call cycle resumes afterward. This branch owns the feature;
it is not attributed to an upstream project. macOS build and decision tests passed.

First JTTY batch, 13 October 2026: sources retained unchanged, paths/hashes
in `wsjtz/jtty/source-manifest.json`. CMake adapters are local to this branch.
Six upstream codec/receive tests passed; application UI, transmission and
audio remain unconnected. See `wsjtz/jtty/README.md`.

For each import, record files/symbols, source reference, modification date,
personal adaptations, integration commit, retained notices and validations.
Preserve third-party headers/licenses. A downloaded reference is not an
integrated import, and compilation alone is not functional validation.

CQ reply queue, 13 October 2026: personal extension in
`wsjtz/automation/CqReplyQueue.hpp`, with receive, QSO-completion, timeout
and manual-selection adapters in `widgets/mainwindow.*`. Always active in
supported automatic CQ operation. Upstream authors are not credited as
creators of this new feature. Build and queue tests passed; radio tests continue.

Queue extension to direct calls during an existing QSO, 13 October 2026:
attach the current partner to the queue session, wait for logging,
and retain deduplication. Optional handover remains limited to unanswered
outgoing attempts. Build and automation regressions passed.

Personal settings moved to **Settings / Fusion / Misc**: incoming priority
and per-queued-station unanswered transmission limit. **WD Counter** defaults
to numeric **0**, preserving the existing watchdog and the mode's Custom WD
Timer when enabled. Positive values use the per-station counter.

Private keys:
`Configuration/Edpublic06/PreferIncomingCalls` and
`Configuration/Edpublic06/WDCounter`. Temporary-file persistence tests preserve
existing WD keys and another MOD's values, adding exactly two keys. This is
not testing of every available MOD. No global settings conversion is introduced.
Accepted/Cancel behavior follows the existing configuration dialog.

The repository was renamed `Edpublic06/wsjt-fusion`. Historical references to
`Edpublic06/wsjt-z` identify its pre-rename history. Fusion's application version
is independent of upstream component versions.

## 1.0.0-beta2 — 14 October 2026

Includes receive-period handover resolution (911456c) and the Min Signal correction (19d1f57). An outgoing partner replying in the same receive period is protected before switching to another caller. Incoming-station filtering is respected when taking stations from the queue. Min Signal is checked before automatic-selection exemptions and again before starting an outgoing call. Direct replies from the current partner remain processable. macOS Apple Silicon build and existing FT2/automation regressions pass; live validation continues.
