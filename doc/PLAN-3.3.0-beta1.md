# WSJT-Fusion 1.0.0-beta1 — feasibility and integration plan

Initial study: 13 October 2026. This document retains findings about the original
WSJT-Z baseline. Current imports and validations are recorded in UPSTREAM-SOURCES.md.

## Examined sources

- Repository: https://github.com/Edpublic06/wsjt-fusion
- Starting branch: `macos-build-maintenance`, commit `e850c68`.
- Upstream reference: https://github.com/WSJTX/wsjtx/releases/tag/v3.3.0-beta1
- Upstream commit: `57ba9337fa020065dc0ceee2c4ef85e433ea32d4`.
- Target application identity: **WSJT-Fusion 1.0.0-beta1**.

## Conclusion

Integration appears technically feasible from the examined sources. It is a
substantial port including **JTTY**, not a simple replacement of `lib`. This
initial conclusion comes from source inspection, not build, performance or
radio validation.

At study time the Z baseline declared `VERSION_Z=2.0.19` and base `3.0.0` in
Versions.cmake. Target upstream declares `version=3.3.0`, `channel=BETA`,
`prerelease=1` in release-state.txt. Fusion now has its independent version;
component references remain separate.

After line-ending normalization, 216 common lib files and 37 common widgets
files differed. Differences include upstream evolution, not only Z changes;
attribute them through history before deciding what to port.

## WSJT-Z changes to preserve

- Auto CQ, Auto Call, Auto Call Next, Pounce, priority queues and band changes.
- Filters, alerts, worked-before state and highlighting.
- Z-specific QRZ, map, PSK Reporter, logging and UDP commands.
- Configurable multithread FT8 decoding and deduplication.
- Actual FT8 algorithm changes, inventoried separately from README claims.
  For example, an OSD modification is commented out in lib/ft8/ft8b.f90 after
  false decodes: do not reactivate it merely because README advertises it.
- FT2 and its adapters; Z's lib/ft2 sources are absent from this upstream reference.
- macOS adaptations and rebuild procedures preserving existing dependencies.

## Critical integration points

1. **Decoder engine:** upstream includes DecoderEngine.h (ABI 7),
   lib/decoder_engine.f90 and new per-mode contracts. Port C/C++ and Fortran
   interfaces, source and build configuration together.
2. **Decoder communication:** memory-layout and startup checks must remain
   consistent. Z fields in commons.h and lib/jt9com.f90, especially FT8
   options, require coordinated changes on both sides.
3. **Specific FT8 changes:** Z routing in lib/decoder.f90 is substantial.
   Compare upstream and Z multithreading; preserve Z features through explicit
   adapters rather than blindly combining competing systems.
4. **UI/audio:** widgets/mainwindow.*, Detector, Modulator and
   Decoder/decodedtext.* evolve together. Replacing the whole Z window would
   remove its extensions.
5. **Version/packages:** use Fusion's own identity/version rather than the
   historical combined X/Z suffix. Retain upstream licenses and attribution.

## JTTY in the target

Import lib/jtty, decoder/controllers, audio streams/queues, widgets/Jtty*,
mainwindow_jtty*.cpp, settings, menus, frequencies, documentation, tests and
actually required build dependencies.

JTTY receives/transmits continuously and can start transmission at any time;
it does not follow FT8/FT4 synchronized cycles. Retain Z automation for existing
modes and adapt it before enabling it for JTTY. Preserve transmit-session
ownership, message queue, stop and PTT behavior. Examine upstream macros,
contest features and N1MM/MMTTY integration with their platform restrictions.

## Required modular architecture

The goal is to simplify successive WSJT-X updates by isolating Z-specific
features as far as practical. This is a design target, not a claim of complete
implementation.

### Three layers

| Layer | Responsibility | Maintenance rule |
| --- | --- | --- |
| WSJT-X upstream | Decoders, codecs, audio transport, standard features including JTTY | Keep upstream paths/content where practical for comparison and commit imports. |
| Application adapters | Translate requests/results/capabilities/events between upstream and extensions | Concentrate upstream-interface dependencies in a few identified files. |
| Z/Fusion extensions | Automation, filters, priority queues, alerts, integrations, specific FT8/FT2 behavior | Dedicated modules; avoid direct dependency on internal Fortran structures or main-window implementation. |

Suggested directories, subject to actual source boundaries:

```text
wsjtz/
  core/           # internal models/contracts
  adapters/       # engine, audio, radio and upstream UI adapters
  automation/     # Auto CQ/Call, Pounce, queues and band changes
  filtering/      # selection, rules and highlighting
  integrations/   # log, UDP, QRZ, map and PSK Reporter extensions
  modes/          # specific capabilities/behavior including FT2
  ui/             # extension panels and controls
  decoding/       # specific decoder extensions
```

Prefer separate CMake libraries/targets for extensions and adapters. The app
assembles components; avoid circular dependencies. Modularity does not require
dynamic plugins or moving all upstream files.

### Contracts and adapters

- Expose structured decode results: mode, time, frequency, SNR, text/session.
  Translate upstream outputs in the adapter, rather than spreading their text
  format or memory structures throughout the extensions.
- Provide explicit operating requests: station selection, TX message,
  frequency change, stop. Upstream radio/audio controllers own CAT, PTT and TX.
- Describe capabilities per mode: synchronized or continuous operation,
  messages, progressive transmission, compatible automation. JTTY must not
  inherit FT8 cycle assumptions.
- Separate extension settings/migration from upstream settings while retaining
  existing preferences.
- Gradually extract rules from mainwindow.*; the window displays/forwards events.
  Reduce .ui changes through separate extension panels when practical.
- Retain C/Fortran and communication checks. Do not add arbitrary Z fields to
  upstream structures: use explicit extension interfaces, or document and test
  necessary changed contracts on both sides.

### Changes inside the decoder

Display filters/automation can be isolated after decoding. Algorithmic FT8
changes may alter passes, thresholds, work data or engine strategy itself.
First seek upstream extension points; otherwise retain small identified patches
with rationale and decoding tests. Avoid a permanent complete engine copy that
doubles maintenance. Inventory necessary upstream changes by file, preserved
feature, adapter rationale and expected validation.

### Each WSJT-X update

1. Record upstream tag/commit, application version and dependency versions.
2. Compare with the previous reference; import upstream changes separately
   from extension adaptations.
3. Adapt interfaces and reapply only necessary patches; verify before dropping
   patches replaced by equivalent upstream functions.
4. Run upstream, contract and extension regressions, then compare decoding on
   a common WAV corpus.
5. Validate JTTY, retained modes and extension features before packaging.

The goal is to limit impact to adapters and inventoried patches. Architecture
cannot guarantee no regressions when protocols/interfaces change; contract
and regression tests are part of the separation.

## FT2 origin and independent maintenance

History explicitly identifies **WSJT-X Improved** as the declared source of
the active FT2 port:

- `ec2a6037502bb177e93996d2224c97866b146a5b`, 13 April 2026:
  “Port FT2 mode from wsjtx-improved”, adding receive routing in
  lib/decoder.f90 and mode support in widgets/mainwindow.{cpp,h,ui}.
- `c467f6653c5f79d0122fcac411b3caf27f223e2e`: corrects the period to **3.75 s**,
  with tone spacing doubled from FT4. It corrects the initial description
  incorrectly calling FT2 a slowed-down FT4.
- Later commits complete transmission, modulation, display, logging, timing
  and fixes. Preserve those Z adaptations separately.

In the examined baseline FT2 uses nmode=52: repeat samples twice, call
my_ft4%decode, then convert DT/frequency in ft2_decoded. Transmission uses
105 symbols of 288 samples each in mainwindow.cpp. Historical lib/ft2 instead
describes experimental LDPC (128,90), 144 symbols; CMakeLists.txt lists only
its gfsk_pulse.f90. Do not treat that folder as the active FT2 module.

WSJT-X Improved is now **WS — Digital Mode Suite**, by Uwe Risse DG2YCB and
collaborators. References:

- Official site: https://wsjt-x-improved.sourceforge.io/
- Releases/source archives: https://sourceforge.net/projects/wsjt-x-improved/files/
- 3.1.0 sources: https://sourceforge.net/projects/wsjt-x-improved/files/WSJT-X_v3.1.0/Source%20code/
- 3.1.0 260228 FT2 fixes: https://sourceforge.net/p/wsjt-x-improved/mailman/message/59302224/

The declared lineage is identified, but the initial Z port's exact source
version/archive was not recorded in its introduction commit. Establish precise
file attribution by comparing historical archives. Decodium/IU8LMC is a separate
implementation; the FT2 name alone does not attribute Z's port to it.

FT2 update organization:

1. Track a second upstream independently of WSJT-X: supplier, URL, dated
   version, archive and SHA-256 or commit.
2. Inventory imported files/sections, attribution, licenses and Z adaptations.
   Do not record an assumed origin version as verified.
3. Isolate FT2 parameters, time/frequency conversion and orchestration in its
   mode module/FT4 adapter; explicitly record the WSJT-X FT4 dependency.
4. Compare new WS/Improved releases to the last imported FT2 reference and
   select useful changes, rather than importing every feature of that app.
5. Test updated FT2 and upstream FT4 together: cross TX/RX, 3.75 s period,
   symbols, frequency, DT, messages, false decodes, logs and Z automation.
   FT4 engine evolution can affect FT2 without changes in the FT2 module.

These requirements define source tracking; they create neither scheduled
monitoring nor automatic code updates.

## Track original WSJT-Z and select future features

Track three lineages: WSJT-X for the base/JTTY, WS/Improved for FT2, and original
WSJT-Z for its own features/fixes. Fusion's independent version does not erase
the original Z baseline 2.0.19.

### Z reference verified on 13 October 2026

- Upstream: https://github.com/sq9fve/wsjt-z
- Tag: v2.0.19; commit: b3a06e89ae0174c51c8595d99e40ca179738eb0a.
- Official remote/local tags matched; official master also matched at inspection.
  Tag/commit are durable references, unlike master.
- Personal macos-build-maintenance starts there and reached e850c68 for the
  study. Inventory personal commits separately from official Z source.
- v2.0.19..e850c68 changes only nine documentation/macOS procedure/Git exclusion
  files, not application engine/UI. Starting app source is official Z 2.0.19.

The tag identifies the complete Z snapshot exactly. Attribution of each part is
more limited: the initial snapshot already includes integrated modifications.
A Z comment or last git-blame author alone does not establish algorithm origin.
Distinguish verified, declared and unresolved provenance.

### Evolution organization

Build a provenance/preservation matrix during modularization:

| Recorded information | Purpose |
| --- | --- |
| Feature and personal module | Locate where to port changes after moves. |
| Files/symbols in Z 2.0.19 | Compare future original versions. |
| Lineage/source reference | Distinguish X, WS/Improved, Z and personal adaptations. |
| Personal changes/rationale | Preserve this fork's choices. |
| Dependencies/tests | Assess fix or feature scope. |
| Import decision/integrated reference | Record adopted, adapted, deferred or rejected changes. |

Retain original Git references and separate attributed import commits. Avoid
rewriting/flattening reference history. Separate module moves from behavior
changes so comparison remains tractable.

### Each new official Z release

1. Compare its tag with the last reviewed Z tag, initially v2.0.19; review
   changelog, commits and diffs.
2. Identify retained-feature fixes, new features, dependencies and already
   imported X/WS changes; avoid duplicate imports.
3. Present new features with value, impact/dependencies for user selection;
   do not automatically enable all additions.
4. Import/adapt selected changes into corresponding modules, retaining source
   Z commit/PR references. Cherry-pick may suit isolated fixes; explicit
   adaptation often becomes necessary after modularization.
5. Test affected features/adapters and update the matrix. Record the **last
   reviewed version** separately from **actually imported changes**; partial
   selection does not mean a complete Z release was adopted.

Document X base, FT2 reference, original Z baseline, subsequent Z imports and
personal changes together. This study creates no periodic monitoring.

## Recommended implementation order

1. Establish a baseline Z build and record macOS dependency versions.
2. Attribute differences, define the preservation matrix/contracts and
   progressively extract extensions into dedicated modules.
3. Port engine/communication contracts, then specific FT8/FT2 adapters.
4. Integrate JTTY end to end: RX/TX, UI and radio control.
5. Apply consistent Fusion version/package metadata independently of upstream.
6. Validate and produce a distinct beta package for testing.

## Validation criteria

- macOS build with preserved dependencies, startup/settings recovery.
- Upstream C/Fortran, communication and engine tests.
- Same-WAV comparison against baseline: messages, duplicates, false decodes,
  time. Do not promise sensitivity gains without measurement.
- Existing modes, especially FT8, FT4, FT2.
- JTTY upstream RX/loopback tests followed by controlled audio testing.
- Z automation, filters, logging and UDP.
- CAT/PTT and stopping TX; real-radio tests require hardware and the operator.

## Study and integration status

Work continues on wsjtz-3.3.0-beta1-integration. WS 3.2.1 (261010) FT2 reception
is integrated separately; optional incoming-caller priority, a deduplicated queue
and WD Counter are implemented. JTTY core is imported unchanged and tested.
Initial macOS build and eight targeted tests passed; later automation changes
have their own regression records.

General engine migration, JTTY application UI/audio integration and full radio
validation remain unfinished. Fusion's identity is now 1.0.0-beta1, with an
Apple Silicon test package published. That package does not claim completed
JTTY or general-engine integration. Exact imports/adaptations/validation are
in UPSTREAM-SOURCES.md and module README files.
