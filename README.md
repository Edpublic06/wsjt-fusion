<h1 align="center">WSJT-Fusion</h1>

<p align="center">1.0.0-beta1 — independent fork maintained by F6FDY</p>

<p align="center">
  <a href="https://github.com/Edpublic06/wsjt-fusion/releases"><img src="https://img.shields.io/github/v/release/Edpublic06/wsjt-fusion?include_prereleases&label=release" alt="Latest release"></a>
  <a href="https://www.gnu.org/licenses/gpl-3.0.txt"><img src="https://img.shields.io/badge/license-GPL--3.0-blue" alt="License: GPL-3.0"></a>
  <a href="https://groups.io/g/WSJT-Z/topics"><img src="https://img.shields.io/badge/discuss-groups.io-orange" alt="groups.io"></a>
  <a href="https://github.com/Edpublic06/wsjt-fusion/releases"><img src="https://img.shields.io/github/downloads/Edpublic06/wsjt-fusion/total?label=Téléchargements" alt="Téléchargements"></a>
</p>

---

**WSJT-Fusion** is an independent fork maintained by **F6FDY**, derived from
**WSJT-Z 2.0.19 by SQ9FVE**, itself based on **WSJT-X** by Joe Taylor K1JT
and the WSJT Development Group. It incorporates FT2 contributions from
**WS/WSJT-X Improved** and additional automation developed for this fork.
It is not an official release of any upstream project.

The application has its own version series, starting at **1.0.0-beta1**.
The complete WSJT-X 3.3.0-beta1 engine migration and JTTY application
integration are **still in progress**; the imported JTTY core is not yet a
fully available operating mode. See [integration status](doc/PLAN-3.3.0-beta1.md)
and [source provenance](UPSTREAM-SOURCES.md).

WSJT-Fusion supports **FT8**, **FT4**, **FT2**, **JT4 / JT9 / JT65**, **Q65**, **MSK144**, **WSPR**, **Echo** and **FreqCal**.

> ⚠️ **Always monitor your transceiver while using WSJT-Fusion**, unless unattended/automated operation is explicitly permitted by the regulations in your country.

---

## Versions du code intégré / Component versions

**[Voir le tableau des sources par fonction : FT8, FT4, FT2, JTTY, WSJT-Z et fonctions Fusion](UPSTREAM-SOURCES.md#tableau-des-versions-par-fonction).**

Le tableau indique les versions, les révisions exactes connues et l’état réel de chaque intégration. La version de Fusion est indépendante de celles de ses composants.

## Discussion & support

- **Upstream WSJT-Z community (not Fusion support):** https://groups.io/g/WSJT-Z/topics
- **Issue tracker:** https://github.com/Edpublic06/wsjt-fusion/issues
- **Changelog:** see [`changelog.txt`](changelog.txt) and the GitHub [Releases](https://github.com/Edpublic06/wsjt-fusion/releases) page

## Download

Published builds and release notes, when available, are listed on this fork’s **Releases** page:

  → **https://github.com/Edpublic06/wsjt-fusion/releases**

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
WSJT-Fusion 1.0.0-beta1; it is not an official upstream release. Those documents
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
