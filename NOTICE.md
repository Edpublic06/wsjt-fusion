# Copyright, licenses and credits

This working branch develops **WSJT-Fusion 1.0.0-beta2**, a derivative version.
It is not an official release of WSJT-X, WS or WSJT-Z. Changes in this branch
do not transfer the rights of the authors of imported code. The licenses and
notices in individual files remain applicable; this document supplements them.

## License

WSJT-Z and the imported WSJT code are distributed under the GNU General
Public License, version 3 or later, according to the projects' notices.
The license text is retained in [COPYING](COPYING). Third-party components
have their own notices, which must accompany the relevant files and distributions.

## WSJT authors' notice

The following notice comes from WSJT-X `v3.3.0-beta1`,
`doc/common/license.adoc`, and also appears in the examined WS 3.2.1 source:

> The algorithms, source code, look-and-feel of WSJT-X and related
> programs, and protocol specifications for the modes FSK441, FST4,
> FST4W, FT4, FT8, JT4, JT6M, JT9, JT44, JT65, JTMS, Q65, QRA64, ISCAT,
> and MSK144 are Copyright (C) 2001-2026 by one or more of the following
> authors: Joseph Taylor, K1JT; Bill Somerville, G4WJS; Steven Franke,
> K9AN; Nico Palermo, IV3NWV; Uwe Risse, DG2YCB; Brian Moran, N9ADG;
> John Nelson, G4KLA; Charles Suckling, DL3WDG; Roger Rehr, W3SZ;
> Greg Beam, KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip
> Karn, KA9Q; and other members of the WSJT Development Group.

Specific attributions in source files, AUTHORS, documentation and the About
dialog must be retained, including for modes not listed in this general notice.

## Source lineages and contributions

- **WSJT-X / WSJT Development Group:** application base, protocols, engines
  and tools. Target reference for the general engine and JTTY:
  [WSJT-X v3.3.0-beta1](https://github.com/WSJTX/wsjtx/tree/v3.3.0-beta1),
  commit `57ba9337fa020065dc0ceee2c4ef85e433ea32d4`.
  JTTY is developed by Joe Taylor K1JT and the contributors identified in
  its source and history; retain their specific attributions.
- **WSJT-Z / SQ9FVE and contributors:** operating extensions, automation,
  filters and integrations. Baseline:
  [WSJT-Z v2.0.19](https://github.com/sq9fve/wsjt-z/tree/v2.0.19),
  commit `b3a06e89ae0174c51c8595d99e40ca179738eb0a`.
- **WS, formerly WSJT-X Improved / Uwe Risse DG2YCB and collaborators:**
  declared source of the active FT2 port in Z's history. Examined update
  source: WS 3.2.1, build `261010`,
  [official archive](https://sourceforge.net/projects/wsjt-x-improved/files/WS_v3.2.1/Source%20code/ws-3.2.1_261010.tgz/download).
  The exact version used for the initial Z port remains undetermined.
- **JTDX and contributors:** FT8 decoding improvements imported into the Z
  baseline. The initial snapshot declares the import; detailed attribution
  to a specific JTDX revision remains to be established. Retain existing
  headers and JTDX credits.
- **Edpublic06 personal branch:** macOS maintenance, documentation and
  integration adaptations identified in this branch's history. These
  adaptations must not be attributed to upstream projects.

Identifying WS/Improved as the source of Z's FT2 port does not attribute it
to Decodium/IU8LMC. Any future import from another implementation requires
separate provenance and credits.

## Integration status

Update references identify examined sources and integration targets, not a
claim that all their changes have already been integrated or validated.
[UPSTREAM-SOURCES.md](UPSTREAM-SOURCES.md) tracks actual imports and adaptations.

For distributions, retain notices, identify modified files and modification
dates, include applicable licenses and provide corresponding source code
according to those licenses. Published binaries and sources must match revisions.

## Fork identity

WSJT-Fusion is an independent fork maintained by F6FDY, derived from WSJT-Z
2.0.19 by SQ9FVE. Its version number is independent of component versions.
Historical copyrights and credits remain applicable; F6FDY does not claim
authorship of imported code.
