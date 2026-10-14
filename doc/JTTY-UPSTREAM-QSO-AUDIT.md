# WSJT-X JTTY display, selection and QSO logging audit

Examined source: official WSJT-X **v3.3.0-beta1**, commit
`57ba9337fa020065dc0ceee2c4ef85e433ea32d4` (14 October 2026).
This describes upstream behavior, not a claim that all of it is integrated into Fusion.

| Data or action | Upstream implementation and limits |
| --- | --- |
| Receive text, UTC, audio frequency, measured SNR | Receive contexts and `JttyReceiveLine::Presentation` carry these separately. Progressive text rewrites its own line rather than adding a new decode every time. |
| All-activity versus QSO pane | `JttyReceiveResultController` routes results using the selected audio frequency/tolerance and operating context. It can report sender/grid/frequency/SNR/time to PSK Reporter. A spot is not a logged QSO. |
| Double-click on a callsign | The JTTY branch of `handleDecodeSelection` writes the selected word into DX Call and obtains the selected decode's SNR through `jttySnrForSelectedWord`. It returns before the structured FT8 `deCallAndGrid` selection path. |
| Grid and received exchange | That JTTY double-click branch does not automatically populate all the structured FT8 QSO fields from arbitrary free text. Finding a sender/grid for a spot does not itself populate the logging form. |
| Signal report macro | `%R` uses the selected peer's SNR in the JTTY macro context. This is distinct from automatically filling both sent/received log reports. |
| Log QSO button | Uses the shared `initLogQSO` form with call, available grid, `m_mode`, reports, times, exchanges and RX dial frequency plus TX audio offset. The form can be reviewed/edited. |
| Start/end time | Logging uses `m_dateTimeQSOOn` and current UTC for the end. Selection resets the QSO start; the common fallback still estimates a start using timed-mode TX counters/period. No dedicated JTTY free-text QSO-boundary recognition was found in this path. |
| Contest TU handling | `handleJttyContestSerial` recognizes transmitted text beginning with `TU `, schedules `logQSOTimer`, stores the serial as the sent exchange and increments the JTTY serial box. It is called on the first accepted transmit segment, so this is an enqueue-time trigger, not proof that RF playback or the remote QSO has finished. |
| Saved logs | Log form acceptance writes `wsjtx.log`; `acceptQSO` adds the ADIF record to the worked-before/log book (`wsjtx_log.adi`). |
| ADIF mode | `AdifQso::to_adif` emits `MODE=JTTY` through its generic mode branch. The MFSK/SUBMODE branch is used for FT4/FST4/Q65, not JTTY. This is the observed writer behavior, not a statement of external logger acceptance. |
| External logger notifications | `acceptQSO` sends UDP QSOLogged and LoggedADIF events, and supports configured N1MM ADIF forwarding. Windows also has a separate MMTTY/N1MM live-text interface. Those are different from merely displaying receive text. |

## Exact upstream locations

- [Selection and SNR](https://github.com/WSJTX/wsjtx/blob/v3.3.0-beta1/widgets/mainwindow.cpp#L9020)
- [Receive presentation and pane routing](https://github.com/WSJTX/wsjtx/blob/v3.3.0-beta1/widgets/mainwindow_jtty_receive.cpp#L90)
- [Common Log QSO entry point](https://github.com/WSJTX/wsjtx/blob/v3.3.0-beta1/widgets/mainwindow_slots.cpp#L486)
- [Contest TU/serial handling](https://github.com/WSJTX/wsjtx/blob/v3.3.0-beta1/widgets/mainwindow_jtty.cpp#L828)
- [Log form and text log writer](https://github.com/WSJTX/wsjtx/blob/v3.3.0-beta1/widgets/logqso.cpp#L215)
- [ADIF serialization](https://github.com/WSJTX/wsjtx/blob/v3.3.0-beta1/logbook/AdifQso.cpp#L6)
- [Accepted QSO and external notifications](https://github.com/WSJTX/wsjtx/blob/v3.3.0-beta1/widgets/mainwindow.cpp#L10366)

## Fusion integration boundary

The original JTTY transmit page was extracted from upstream `mainwindow.ui`
(`page_4`) into a standalone form, preserving its layout, widgets and labels.
The codec, local audio queue/stream and lifecycle are integrated with offline
checks. This phase does not claim upstream receive-line metadata, selected-peer
SNR mapping, all/QSO pane routing, click-to-call selection, contest TU logging or
MMTTY integration. For now enter DX Call and review Log QSO manually. Do not
infer a successful logged QSO from a received line or a queued transmission.
These remaining connections should be ported explicitly from the above sources.

### Received TU example

`1490 Hz -13 dB DK0BO TU CQ K3CLT CQ` is consistent with the F3 template
`%H TU CQ %M CQ`: K3CLT thanks DK0BO and resumes calling CQ. It does not
show a completed QSO involving F6FDY. A received TU is not the upstream
transmit-side `handleJttyContestSerial` event and must not trigger a local log.
