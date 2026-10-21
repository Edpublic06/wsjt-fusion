# Fusion beta3 — notes for future GitHub documentation

Recorded 2026-10-15 at F6FDY's request. These notes describe implemented code, not a new public release. Consolidate them into English release notes and README updates when requested.

## Current distribution and permissions

- Protected public beta2 remains unchanged.
- Private tester repository: https://github.com/Edpublic06/wsjt-fusion-beta
- Its published test is still `v1.0.0-beta3-test.20261015.09a8a12`, built from application source `09a8a12`. It does NOT contain the subsequent changes below.
- Latest local test application: `build/jtty-integration/test-package-beta3-5e66bed/WSJT-Fusion.app`, source `5e66bed`.
- Compile only when F6FDY explicitly requests it. Upload/update GitHub release assets only on a separate explicit request. Do not automate either operation.
- Future upstream UI changes require F6FDY's decision. WSJT-Z imports require explicit agreement. Preserve Fusion ergonomics and beta2.

## Final feature behaviour to document

- Live expanded F1–F8 macro previews appear below editable templates, directly on the grey background. They update with selected correspondent, callsign, locator, report, serial, next call and template changes. Preview and send share the expansion function. Extra vertical separation between macro groups; Call next and Serial Number share a row.
- JTTY QSO logging exports mode `JTTY` through the shared log dialog, ADIF and UDP regardless of macro style, for all connected loggers.
- Entering JTTY from another mode clears the inherited target callsign, locator, Call Info and QSO reports. Re-selecting already active JTTY does not discard the current target.
- Configurable checked criterion `New Mode on Band [f/g unset]`, default light grey `#E0E0E0`. It applies when the callsign was worked on the same band in another mode but not the current mode. It is separate from New Call on Band. Existing palettes retain custom colours and order; the new item is inserted after New Call on Band. JTTY entities/calls are checked across modes so an already-worked country is not falsely classified as new. Earlier cyan/New Call on Band interim implementations are superseded.
- JTTY receive views use the common decoded-text font; controls follow the application font and subsequent changes. Macro previews keep the requested relative 80% size and inherit font style.
- Clicking a callsign in JTTY Band Activity selects the correspondent, moves Rx to that decoded line's audio frequency, and moves Tx only when Hold Tx Freq is unchecked. Rx Frequency is re-filtered around Rx using F Tol. Double-click triggers F1 in FT8 message style after tuning.
- Single-line RST S / RST R display above the JTTY Tx controls. RST S value is bold green `#008000`; RST R is bold purple `#800080`.
- RST S becomes available from the first selected-peer reception, then averages the first two and first three distinct received messages (rounded dB); later distinct messages do not enter the average. Existing progressive updates to those same decoded messages may refine their SNR. It feeds the display, macros and common sent-report logging field.
- RST R is taken from newly decoded messages explicitly addressed to our callsign by the selected correspondent, e.g. `F6FDY K2GAV +03 K` or `F6FDY K2GAV R+03 KN` -> `+03`. Optional DE is accepted. Other peers, other destinations and CQ/locator-only text do not populate the report. Received report is not averaged.
- Both report fields clear on target change and JTTY entry. Automatic QSO logging remains deferred; report extraction does not trigger logging.

## Verification and scope

Latest requested compilation completed successfully on 2026-10-15 (macOS Apple Silicon). Five focused suites passed: fusion_jtty_panel, fusion_jtty_sender, fusion_jtty_text_colors, fusion_jtty_sent_report, fusion_jtty_snr_average. Application packaged and ad-hoc signature verified by packaging tool. These are automated/offline checks, not proof of an on-air exchange or MacLoggerDX end-to-end validation. No GitHub assets updated for this build.

Implementation commits after the private test: `55cf52d`, `6c790c0`, `ce8b6e4`, `0d11f70`, `c29b8fc`, `6dd0e44`, `6f80096`, `8a5da4b`, `5e66bed`. Prior macro/logger implementation: `2064065`; initial first-three report work: `7ac2754`. Use final behaviour above rather than presenting superseded colour experiments as features.

Other beta3 integration and upstream provenance are maintained in JTTY-DEVELOPMENT.md, releases/1.0.0-beta3.md and UPSTREAM-SOURCES.md, plus import manifests. Decoder reference is WSJT-X 3.3.0-beta2, with FT2 sourced separately. Consult those records when preparing the complete beta3 announcement.

JTTY lower spectrum display gain is increased by a factor of 1.5 for readability, on top of the existing adjustable 2D gain. This affects display only, not waterfall intensity, receiver audio, SNR measurements or decoding. Other modes retain their scale. Pending compilation after the 5e66bed test build.

The full RST S / RST R line (labels and values) uses 125% of the configured application font, with dynamic sizing when font settings change. Value colours and bold formatting are preserved. Pending compilation.

The enlarged JTTY report line now starts with the selected callsign in bold DX blue (#007AFF), followed only by available RST S / RST R fields. It is entirely blank without a selected peer. In FT8 macro style, F2 is highlighted green until a report addressed to us from that peer is decoded, then F3 is highlighted green. This is a visual suggestion only; no automatic transmission. Selection changes clear old reports and update the suggestion. Other macro styles receive no F2/F3 suggestion.

Clarification: peer-dependent macro previews remain empty when no target callsign is selected; the placeholder "Select a DX Call" is removed (including its tooltip). The editable macro templates remain visible. No new compilation for this adjustment.

F6 is green by default; when the FT8-style F2/F3 suggestion applies, F6 loses its green background. Message style sits above Send message beside the message editor. The separate style row and bottom 40px spacer are removed, reducing the controls height so Rx Frequency can use the recovered area. Pending compilation and visual validation.

Double-clicking a JTTY callsign in either receive pane tunes Rx to the decoded frequency and Tx only if Hold Tx Freq is unchecked, highlights F1 green and triggers F1 in FT8 message style. The F1 highlight persists across unchanged context refreshes; a changed peer or received report restores normal F2/F3 guidance. Pending compilation.

JTTY hides the Auto (free-frequency selection) and Mode Now controls. Leaving JTTY restores Auto and restores Mode Now according to its existing automatic-mode-switch setting. Toggling that setting while in JTTY cannot reveal Mode Now. Pending compilation.

Reset WD is likewise hidden in JTTY and restored on leaving the mode. Pending compilation.

The Filtering toggle is hidden in JTTY because the structured-mode filters are not applied to its decoded stream, and restored when leaving JTTY. Its checked state is preserved. Pending compilation.

JTTY also hides Tx even/1st, Auto CQ and Auto Call. Leaving JTTY restores Auto CQ/Auto Call and restores Tx even/1st according to the normal destination-mode widget visibility policy. Pending compilation.

Correction: single-click selection must also highlight F1, without transmitting. Double-click reasserts F1 and triggers it. A new selected callsign defaults to F1 rather than F2; periodic report/SNR refreshes do not remove that highlight. A newly received report still restores F3 guidance. Pending compilation.

Selected callsign in the information line is enlarged to 175% of the application font (reports remain 125%). Send message has a white background when the editor is empty or whitespace-only and green when text is present; clearing the editor after queue acceptance automatically restores white. Pending compilation.

Expanded macro previews use DX Call blue (#007AFF), retaining their smaller relative font and transparent grey-panel background. Editable templates are unchanged. Pending compilation.

Priority adds New call (appended, preserving existing saved indices). Automatic candidates never worked in the loaded log, across all bands and modes, are preferred over worked calls independently of highlight-by-mode settings and PSK preference. Existing eligibility filters and CQ-target preference remain. The initial CQ reply queue choice can prefer new calls; subsequent waiting-list selection remains strict FIFO. Pending compilation.

New call priority never overrides enabled Min Signal filtering. Candidate SNR is checked before normal priority ranking and again before starting a queued caller, independently of the optional additional Auto CQ filters. Existing direct replies can continue an already active QSO below the threshold. Pending compilation.

With Signal strength priority, equally strong eligible candidates are separated by all-band/all-mode worked-before status: never-worked calls win the tie. Stronger signals retain precedence. Applies to automatic nomination and the initial CQ queue choice; subsequent queue order remains FIFO, with existing Min Signal and preference gates preserved. Pending compilation.

With Distance priority, equal-distance candidates (including multiple unknown distances) prefer never-worked calls across all bands/modes. An unknown distance no longer replaces a known-distance candidate indiscriminately. This uses computed locator distance, independent of whether its display is enabled. Existing Min Signal, eligibility/preferences and waiting-list FIFO remain. Pending compilation.

Tab 2 is now a read-only Fusion queue monitor outside Fox: Queue shows the actual FIFO waiting list, one callsign per line, and In Progress shows the active queued correspondent or selected DX target. It refreshes with the normal GUI cycle and uses the shared decoded font. Fox-specific controls are hidden without changing their values; Fox queue double-click actions are blocked outside Fox. Entering Fox restores those controls and repaints the native Fox queues; Fox queue/sequencer data remain separate. Pending compilation and live mode-switch validation.

The tab 2 queue heading switches dynamically between "Queue Fox Mode" and "Queue Fusion Mode". Pending compilation.

Tab 2 receives a green background while the Fusion FIFO has waiting calls, including when the tab is not selected. It returns to normal when the waiting list is empty (an active QSO alone does not keep it green). Fox mode does not activate this Fusion indicator. Pending compilation and visual validation.

Double-clicking a callsign anywhere in Queue Fusion Mode promotes it to the next FIFO output without interrupting the active QSO. The remaining callers retain their relative order. Explicit promotion overrides initial automatic ranking; updated decodes retain the promoted position. Eligibility checks, including enabled Min Signal, remain in force before calling. Fox double-click actions are unchanged. Pending compilation.

Deferred late RR73 completion (general rule, not specific callsigns): after an actual outgoing report exchange in normal FT8/FT4/FT2 operation, preserve the peer’s log metadata. If its RR73 arrives while another QSO is active, enqueue a signoff item in the Fusion FIFO, suppress immediate takeover, finish the active QSO and existing earlier FIFO items, then restore the deferred peer’s reports/grid/start time and send Tx5 (73) through the standard transmit/log path. Context changes clear saved exchanges; accepted logging removes the completed peer. Ordinary updated decodes cannot overwrite a deferred signoff item. A decoded late RR73 closes an existing exchange rather than starting a new Min Signal-filtered call. Fox/Hound and JTTY are excluded. Pending compilation and end-to-end validation.

Late RR73 collection is independent of Auto Call, Auto TX and Auto Seq selection for a known unfinished normal FT8/FT4/FT2 exchange, while live monitoring remains required. Deferred closures take priority over new-call FIFO items but never interrupt an active exchange. The standard 73/logging path is used even with Auto Call inactive. During closure, automatic mode selection is suspended and CQ/Call counters plus WD elapsed time are retained. After transmission and logging complete, Auto CQ resumes Tx6 with its prior counters/WD remainder if still enabled; otherwise the temporary transmission is disabled. Changing operating context clears deferred resume state. Pending compilation and end-to-end validation, especially WD timing and operator cancellation.

Fusion Queue and In Progress start empty each launch. In Progress no longer treats a saved DX Call focus as an active QSO: it displays the queue’s active caller or a target whose QSO progression has actually started. Saved DX settings themselves are retained. Pending compilation.

After a completed/logged normal FT8/FT4/FT2 QSO, when Hold Tx Freq is unchecked, execute the same free-frequency search as Auto. Defer the action until transmission has ended, recheck Hold and operating context, and perform it before queued continuation or CQ resumption. If no free slot or insufficient activity data exists, the Auto algorithm retains the current frequency. Fox/Hound and JTTY are unchanged. Pending compilation.

The same priority deferred-completion path now accepts a late acknowledged report (R+nn/R-nn) from a known unfinished peer: retain its received report without the R prefix and resume with RR73 (Tx4). A late RR73 instead resumes with 73 (Tx5). Repeated older R-reports cannot downgrade an already queued RR73 completion. Current QSO protection, closure priority, shared logging and Auto CQ/WD restoration remain in force. Bare callsigns, locator-only messages and unacknowledged reports do not trigger this extension. Pending compilation and live validation.

## Consolidation checkpoint — 16 October 2026

F6FDY explicitly requests retention of ALL beta3 changes for a future beta2/beta3 comparison. Use the final behaviour in these notes, the git history after protected beta2 `8345c155a878185f11736e9dec8df278ab7f1c49`, JTTY-DEVELOPMENT.md, UPSTREAM-SOURCES.md and releases/1.0.0-beta3.md. Do not limit that future summary to the last private-test delta. Separate inherited upstream features from Fusion additions, and implemented code from on-air validation.

Latest compiled local application: `build/jtty-integration/test-package-beta3-46b388d/WSJT-Fusion.app`. Latest private published test remains `v1.0.0-beta3-test.20261016.f993353`; subsequent features below have not been uploaded. Public beta2 remains unchanged. Compile and publish only on explicit respective requests.

Final queue/automation additions after f993353:
- Double-clicking a Fusion queue entry promotes it to the next ordinary FIFO output; other callers keep relative order, and the current QSO is not interrupted. Fox actions remain separate.
- Late RR73 from a known unfinished report exchange is retained with its original reports, grid, exchanges and start time; after the current QSO it receives 73 via standard transmit/logging.
- Late acknowledged report R+nn/R-nn uses the same deferred path but receives RR73 (Tx4); late RR73 receives 73 (Tx5). An older repeated R-report cannot downgrade a queued RR73.
- Deferred closure has priority over new ordinary callers/CQ, is collected even without Auto Call/Auto TX/Auto Seq enabled, and requires live monitoring and a known unfinished normal FT8/FT4/FT2 exchange. Fox/Hound/JTTY and composite decodes are excluded.
- Auto CQ resumes after deferred transmission and accepted logging, retaining its CQ/Call counters and watchdog elapsed time; automatic mode selection is suspended during the closure. Operating-context changes discard the saved resume state. These timing paths require on-air/end-to-end validation.
- Fusion Queue and In Progress start empty; a restored idle DX Call does not count as an active QSO.
- After an accepted QSO in normal FT8/FT4/FT2, when Hold Tx Freq is unchecked, defer the Auto free-frequency action until transmission ends, recheck Hold/context, then run the existing search. No free slot/insufficient data leaves the frequency unchanged.

Verification: latest application compiled and packaged successfully; incoming-call and CQ queue regression suites passed, including promotion ordering, closure priority, metadata retention and R-report/RR73 stage upgrade. These tests do not validate complete radio/PTT/log/CQ-WD resumption behaviour. Earlier QT declaration and stale-test-expectation failures were corrected; do not report them as outstanding bugs.

On-air FT4 evidence supplied by F6FDY: completed QSOs; M7FSE received message explicitly contained +30 and wsjtx.log recorded sent +06/received +30, matching the displayed exchange and RR73. This validates that particular report transfer, not universal SNR accuracy or all decoder modes. The duplicate screenshot of the same log is not a separate test.

Click selection in JTTY receive panes highlights the selected callsign with white lettering on red, replacing the previous black-on-yellow selection. This selection colour is distinct from the blue correspondent summary and ordinary log-based message colours. Pending compilation.

The white-on-red selected JTTY callsign explicitly has no underline. Pending compilation.

WITHDRAWN at F6FDY’s request: automatic Auto/free-frequency search after QSO completion has been removed. The manual Auto button and its pre-existing uses remain. Earlier entries describing this post-QSO addition are superseded and must not appear as a beta3 feature in the final summary. Pending compilation.

Final free-Tx policy (replaces withdrawn post-QSO action): in normal FT8/FT4/FT2, with Hold Tx Freq unchecked, invoke Auto immediately before each Auto CQ transmission and before the first transmission to each new Auto Call target. Store that target’s Tx frequency for retries and later QSO messages; automatic decode processing and legacy free-frequency scheduling must not retune the established exchange. Manual frequency actions remain available. Deferred closures keep their own existing frequency handling. No post-QSO search. Pending compilation and live testing.

Final Min Signal clarification (supersedes earlier blanket gating notes): a valid received standard message explicitly addressed to our full/base callsign is exempt from Min Signal, even if the sender is not yet the current partner. Apply the same exemption when taking it from the Fusion queue. CQ/third-party candidates remain filtered for Band Activity and automatic outbound nomination/dispatch; the New call/Signal/Distance preferences never bypass that threshold. Other enabled filters retain their existing rules. Example: an incoming direct report at -7 dB is eligible with Min Signal -4, whereas a CQ at -7 remains below the threshold. Pending compilation and validation.

## Auto Call: abandon after two unanswered CQs — 16 October 2026
In normal FT8/FT4/FT2 Auto Call, abandon an unanswered outgoing attempt when
the selected peer sends two consecutive CQ messages in distinct receive periods.
Repeated decodes of the same period count once; another peer does not count;
non-CQ traffic from the selected peer breaks the sequence. A direct reply
protects the exchange, including one decoded later in the same receive batch.
Stop the old transmission sequence without logging an incomplete QSO, and
resume normal candidate selection. The abandoned callsign is added without duplication to the normal Ignore list,
regardless of the Ignore after WD option. Existing Ignore filtering, manual clearing,
automatic reset and band-change rules apply. Fox, JTTY and established QSOs are excluded.
