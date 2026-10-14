> **JTTY development branch — 1.0.0-beta3-dev.** Published beta2 remains unchanged. This branch adds an experimental offline JTTY WAV receive panel under **Decode**; live JTTY reception/transmission are not yet integrated. See [development and rollback notes](doc/JTTY-DEVELOPMENT.md).

<h1 align="center">WSJT-Fusion</h1>

<p align="center">1.0.0-beta2 — independent fork maintained by F6FDY</p>

<p align="center">
  <a href="https://github.com/Edpublic06/wsjt-fusion/releases"><img src="https://img.shields.io/github/v/release/Edpublic06/wsjt-fusion?include_prereleases&sort=date&label=Fusion" alt="Latest release"></a>
  <a href="https://www.gnu.org/licenses/gpl-3.0.txt"><img src="https://img.shields.io/badge/license-GPL--3.0-blue" alt="License: GPL-3.0"></a>
  <a href="https://groups.io/g/WSJT-Z/topics"><img src="https://img.shields.io/badge/discuss-groups.io-orange" alt="groups.io"></a>
  <a href="https://github.com/Edpublic06/wsjt-fusion/releases"><img src="https://img.shields.io/github/downloads/Edpublic06/wsjt-fusion/total?label=Downloads" alt="Downloads"></a>
</p>

---

**WSJT-Fusion** is an independent fork maintained by **F6FDY**, derived from
**WSJT-Z 2.0.19 by SQ9FVE**, itself based on **WSJT-X** by Joe Taylor K1JT
and the WSJT Development Group. It incorporates FT2 contributions from
**WS/WSJT-X Improved** and additional automation developed for this fork.
It is not an official release of any upstream project.

The application has its own version series, starting at **1.0.0-beta1**.
The complete upstream engine migration and JTTY application
integration are **still in progress**; the imported JTTY core is not yet a
fully available operating mode. See [integration status](doc/PLAN-3.3.0-beta1.md)
and [source provenance](UPSTREAM-SOURCES.md).

WSJT-Fusion supports **FT8**, **FT4**, **FT2**, **JT4 / JT9 / JT65**, **Q65**, **MSK144**, **WSPR**, **Echo** and **FreqCal**.

> ⚠️ **Always monitor your transceiver while using WSJT-Fusion**, unless unattended/automated operation is explicitly permitted by the regulations in your country.

---

## Component versions

**[View the source table by function: FT8, FT4, FT2, JTTY, WSJT-Z and Fusion features](UPSTREAM-SOURCES.md#component-versions-by-function).**

The table lists versions, known exact revisions and actual integration status. Fusion’s version is independent of its component versions.

## Discussion & support

- **Upstream WSJT-Z community (not Fusion support):** https://groups.io/g/WSJT-Z/topics
- **Issue tracker:** https://github.com/Edpublic06/wsjt-fusion/issues
- **Changelog:** see [`changelog.txt`](changelog.txt) and the GitHub [Releases](https://github.com/Edpublic06/wsjt-fusion/releases) page

## Download

Current development and testing focus on **macOS Apple Silicon (arm64)**. Windows builds have not yet been validated for Fusion.

**[Download WSJT-Fusion 1.0.0-beta2 for macOS Apple Silicon](https://github.com/Edpublic06/wsjt-fusion/releases/tag/v1.0.0-beta2).**

Published builds and release notes are listed on this fork’s **Releases** page:

  → **https://github.com/Edpublic06/wsjt-fusion/releases**

## What is new compared with WSJT-Z 2.0.19?

**WSJT-Fusion 1.0.0-beta1** introduces the following changes to the inherited WSJT-Z application. These features are under active development and live radio testing.

| Fusion feature | Behavior |
| --- | --- |
| **Optional incoming-caller priority in Auto Call** | An unanswered outgoing call can yield to a station calling you directly. Once a QSO is established, further callers cannot interrupt it. After the QSO is logged, the normal Auto Call cycle resumes. |
| **Protection of the first accepted caller** | A station accepted after your CQ is protected immediately, including before its next report is decoded. Another caller waits rather than taking over the QSO. |
| **Automatic caller queue** | Multiple replies to your CQ, and direct calls received during an ongoing automatic QSO, are queued and handled one at a time after the current QSO finishes. The queue is always active in supported automatic operation, with no separate enable switch. |
| **Priority-aware selection and duplicate prevention** | Selection uses the applicable existing priority criteria; equal-priority callers retain their arrival order. Repeated calls from the same station do not create duplicate entries. |
| **Queue cancellation on manual station change** | Choosing another station manually clears the remaining queue. A mode, band or dial-frequency change also clears it. |
| **WD Counter for queued stations** | A configurable limit controls transmissions without a reply when calling each queued station. The default **0** follows the existing watchdog behavior, including the mode’s Custom WD Timer when enabled. A positive value limits transmissions, including the first call; a reply resets the counter. |
| **Incoming-caller filtering preference** | Incoming-call handover respects **Apply filtering to stations calling us**, rather than always enforcing the outgoing-call filters. |
| **Updated FT2 receiver** | Receiver code from **WS 3.2.1 build 261010** is adapted in a separate module, preserving the standard FT4 decoder. This is a receiver update, not a claim that all WS features have been imported. |
| **Independent identity and source tracking** | Fusion has its own version series, explicit upstream credits and a per-function source register to support future updates. |

The incoming-caller priority checkbox and **WD Counter** are located in **Settings → Fusion → Misc**. The main window displays the pending call queue. Automatic queue operation currently covers **FT8, FT4, FT2 and Q65**; Fox, Hound and Q65 pileup retain their specialized behavior.

### Work in progress

- **JTTY:** the upstream codec and receive core have been imported and tested. Application UI, audio and transmission integration remain unfinished; JTTY is **not yet available as an operating mode**.
- **General decoder engine:** migration to the newer upstream engine is ongoing. The current FT8 engine still derives from the inherited WSJT-Z baseline and its existing enhancements.
- **Validation:** current builds and tests focus on **macOS Apple Silicon**. Automated tests pass for FT2 and the new automation policies; live radio testing continues. Windows builds are not yet validated for Fusion.

See the [component version and integration table](UPSTREAM-SOURCES.md#component-versions-by-function) for exact known references and the [automation notes](wsjtz/automation/README.md) for technical details.

## Features

### Operating workflow
- **Auto CQ** — call CQ unattended with configurable repeat count and band-rotation rules
- **Auto Call** — automatically respond to filtered stations until a daily/QSO limit is reached
- **Pounce mode** — lock onto a specific call until the QSO completes
- **Priority call queue** — promote callsigns of interest to the front of the call list
- **PSK-priority Auto Call** — optionally prefer stations already heard by PSK Reporter receivers on your band
- **Auto Call Next** — chain QSOs without operator intervention
- **Rapid RR73 short QSO** — double-click TX2 to skip to an RR73 reply for a no-report exchange
- **Band-hopper** — schedule automatic band changes by time-of-day or activity

### Filtering & highlighting
- **Ignored stations list** — silence specific calls or prefixes, with one-click save of the current list into the permanent list
- **Prefix / state / continent / CQ-target filters** — include or exclude by call prefix, US state, continent, or directed-CQ target
- **DXCC / Continent / CQ Zone / ITU Zone** worked-before alerts (per-band variants supported)
- **New on band / new in mode / new for the year** highlighting
- **LoTW user filter** — restrict to stations active on Logbook of the World
- **Signal-strength threshold** filter

### Decoder pipeline
- **Multi-threaded FT8 decoder** (selectable: Auto, 1–12 threads) — substantial speed-ups on multi-core systems
- **JTDX-derived FT8 enhancements** — additional decoding passes, OSD on `ndepth=2`, lowered sync thresholds for weak signals
- **Early-decode dedup** — eliminates duplicate decodes within a single FT8 cycle in multi-threaded mode
- **Stacked-call DXpedition support** — parses composite `RR73;` messages and replies to the tertiary caller
- **Performance work in the C++ hot path** — cached filter lists, reduced regex compilation, gated debug logging, tuned Fortran release flags

### Integration & UX
- **QRZ.com lookup panel** — name, address, grid, biography
- **DX Station Map** — world-map window plotting stations calling you and your logged QSOs
- **Log rotation** — archive `wsjtx_log.adi` with a timestamped name from the File menu, or remotely via a UDP `RotateLog` message
- **Audio alerts** — configurable per-event sounds
- **PSK Reporter integration** — spotting plus band-activity feedback highlighting
- **UDP control server** — optional inbound UDP `Configure` requests (mode/frequency/DX call, Auto-CQ/Auto-Call) for companion apps. Disabled by default; enabling it (**Accept UDP requests**) opens an unauthenticated control surface on all interfaces that can key your transmitter, so only enable it on a trusted network
- **Larger Band Activity window** with improved column layout
- **Custom alert rules**
- **NA_VHF / EU_VHF contest flows** with associated UI tweaks
- **Modernized highlighting model** with per-band overrides

## Tools

The `tools/` directory contains helper scripts for project maintenance:

- `tools/rebuild_usstate_db.py` — rebuilds `USState.db` (callsign → US state mapping used for state filtering) from the weekly FCC ULS amateur licensee dump. Stdlib-only Python; run with `--help` for options.

## License

For this integration branch, see [copyright and credits](NOTICE.md) and the
[upstream source register](UPSTREAM-SOURCES.md). The branch prepares
WSJT-Fusion 1.0.0-beta2; it is not an official upstream release. Those documents
distinguish the existing WSJT-Z baseline from updates still being integrated.

Licensed under the **GNU General Public License v3** — see [`COPYING`](COPYING) and https://www.gnu.org/licenses/gpl-3.0.txt.

The vast majority of the code is created and copyrighted by the WSJT Development Group, led by Joe Taylor K1JT, and licensed under the same terms. WSJT-Z extensions follow the same license.

## Credits

- **SQ9FVE and WSJT-Z contributors** — original WSJT-Z application and automation, baseline 2.0.19
- **WS/WSJT-X Improved authors and contributors** — imported FT2 receiver evolution
- **F6FDY** — maintenance of this independent fork and its additional automation

- **WSJT Development Group** — Joe Taylor K1JT and contributors — for WSJT-X, the upstream project this fork is built on
- **JTDX team** — for the multi-threaded FT8 decoder enhancements that informed the WSJT-Z decoder pipeline
- **JTSDK** — for the Windows build environment
- **WSJT-Z contributors** — see the commit log
