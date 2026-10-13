# Personal Auto Call extensions

`IncomingCallPolicy.hpp` is a personal WSJT-Z extension, dated 13 October
2026, licensed GPL-3.0-or-later. It is kept separate from upstream code.

The **Auto Call: prioritize stations calling us** checkbox is in
**Settings / Fusion / Misc**. It is off by default and saved as
`Configuration/Edpublic06/PreferIncomingCalls`. The prior main-window preference is
used as the migration fallback when the new key is absent.

When enabled, an unanswered outgoing attempt can yield to a different station
that addresses our callsign directly in a standard live decode. Incoming-caller filters apply when **Apply filtering to stations calling us** is enabled. The current target's first directed response protects its QSO;
a newly accepted caller is also protected against later callers. CQ, free text,
signoff messages, file playback and messages received during transmission do
not trigger a handover. Fox/Hound and Q65 pileup keep their specialized logic.

The handover clears the abandoned target, report state and pending priority
selection, resets its watchdog and feeds the incoming message to the existing
QSO sequencer. It keeps Auto Call enabled. The existing logging and cycle code
then returns to normal Auto Call operation; it does not restore the abandoned
target as a pending call. That station may still qualify as a fresh candidate
later under the normal selection/filter rules.

Validation: macOS arm64 application build and
`wsjtz_incoming_call_regression` passed. The regression covers disabled mode,
operational gates, filters, current-target replies, caller protection and reset
for a new outgoing cycle. It does not substitute for a live radio QSO test.

## CQ reply queue

Always active during automatic CQ answering in FT8, FT4, FT2 and Q65.
Eligible standard direct replies are collected for the decode batch before
selection; FT8 early passes wait for the configured final decode stage.
Filters apply both at collection and before starting each queued QSO.

The queue uses existing distance/strongest-signal and PSK preferences, and
existing Best S&P highlight priorities where applicable. Equal priorities
keep arrival order. Repeated decodes update the same entry without moving
its original arrival position. The active or already attempted caller is
not queued again during that round.

The next pending station is processed after logging the active QSO, using
the existing QSO sequencer and watchdog. A watchdog timeout advances to the
next entry. An operator stop pauses automatic processing. Selecting a
different station, by manual edit or decode selection, clears the whole
queue. A mode, band or dial-frequency change also invalidates its context.
The queue is transient and is not restored after an application restart.
Fox/Hound and Q65 pileup retain their specialized operation.

The pending calls are shown in the main controls as `Call queue`. Validation:
macOS arm64 build and `wsjtz_cq_reply_queue_regression` passed (priorities,
FIFO ties, deduplication, exhaustion and manual/context cancellation).
Live CQ/QSO, radio and operator-flow tests are still outstanding.

The same queue now accepts direct callers during an existing automatic QSO,
including an outgoing/manual-selected QSO. It attaches the current partner as
the active exchange, so waiting callers cannot start until it is logged (or
the existing watchdog expires). Repeated calls update one waiting entry.
The optional Auto Call handover still applies to unanswered outgoing attempts;
after a response, new callers wait. Operator selection of another station
continues to cancel the entire queue. Manual TX without automatic sequencing
is not made automatic by receipt of a call.

## Queued-station call limit

Settings / Fusion / Misc also contains **WD Counter**.
The numeric default `0` uses the existing Custom WD Timer for the current
mode when enabled; otherwise the existing standard watchdog configuration applies. A positive value replaces that elapsed-time watchdog
for a dequeued station with a cap on transmissions without a reply, including
the first call. Replies reset the counter for the next exchange step. The
current QSO attached to a queue is not retroactively given this budget.

Transmissions are counted at the PTT start transition, not during message
regeneration. The cap blocks any additional PTT start and advances the queue
after the receive/decode window, without interrupting a running transmission.
A manual station selection still cancels the queue. Values are committed by
OK in the existing Configuration dialog; Cancel retains the accepted settings.
The call-count regression covers zero/WD fallback, limits 1/2, counter reset
on a reply, unrelated stations and a fresh queued target.

The two settings use `Configuration/Edpublic06/PreferIncomingCalls` and
`Configuration/Edpublic06/WDCounter`. Existing WSJT-X/WSJT-Z keys, types,
configuration grouping and WD Timer settings are unchanged. Only the two
new namespaced values are written by this extension; previous experimental
keys are read as migration fallbacks. No settings-schema/version conversion
or removal of other keys is introduced. Other MODs can ignore these private
keys; compatibility with every MOD has not been experimentally verified.

## Minimum received signal for outgoing selection

With Filtering enabled, Min Signal is checked before the Pounce and
current/last-partner exemptions, using the local received SNR (not the report
inside the transmitted text). A newer below-threshold decode cancels an
older nomination of that station. The threshold is checked again before an
automatic outgoing call starts. Direct replies from the selected QSO partner
remain processable when its signal drops; this setting must not break an
ongoing exchange. The existing -30 setting retains its disabled-threshold
meaning. Live validation of this correction is still required.
