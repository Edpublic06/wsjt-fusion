# Personal Auto Call extensions

`IncomingCallPolicy.hpp` is a personal WSJT-Z extension, dated 13 October
2026, licensed GPL-3.0-or-later. It is kept separate from upstream code.

The **Auto Call: prefer incoming calls** checkbox is in the WSJT-Z
operating settings, beside the PSK preference. It is off by default and saved
as `AutoCallPreferIncoming` in the existing per-configuration settings.

When enabled, an unanswered outgoing attempt can yield to a different station
that addresses our callsign directly in a standard live decode. Existing
filters apply. The current target's first directed response protects its QSO;
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
