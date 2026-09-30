# Scrubbles — Max for Live Prototype

## Purpose and current implementation

Perform a modulation contour as both a repeating phrase and a manually scrubbed path. The active prototype is an **unfrozen Max for Live MIDI Effect**, not a synthesizer or audio effect. Its internal `gen~` engine runs at signal rate and modulates a mapped Live parameter.

The earlier JUCE/VST3 instrument, its build configuration, verification sources, and build output are retained under `archive/`. They are not part of the current device build.

This prototype tests traversal, not contour authoring. Six read-only seed contours provide the material.

## Build and load

Requires Node.js to build, and Live with Max for Live / Max 9 to run.

```sh
node build-device.mjs
node validate_maxpat.mjs scrubbles.maxpat scrubbles-engine.gendsp
node --test scrubbles-midi.test.mjs
```

Output: `scrubbles.amxd` beside its source files. The editable AMPF wrapper follows `../remapperator/`; **do not freeze it during iteration**. Keep these files together:

- `scrubbles.amxd`
- `scrubbles.maxpat`
- `scrubbles-engine.gendsp`
- `scrubbles-control.js`
- `scrubbles-ui.js`
- `scrubbles-hold.js`
- `scrubbles-midi.js`

Replace an older audio-effect instance with this MIDI-effect build. Load it on a MIDI track, before an instrument if that track also makes sound. Click **Map**, then a destination parameter in Live; **Unmap** removes the assignment. Amount and Bipolar control modulation depth and polarity.

To receive played notes, route the controller to this MIDI track and use Monitor In, or arm the track with Monitor Auto. Live's global transport does not need to run for free-running contour traversal.

**C0–E0 (MIDI 24–28) are reserved controls on every MIDI channel.** Their note-on, note-off, and polyphonic pressure messages are consumed before MIDI output. Other notes, CCs, channel pressure, program changes, pitch bend, system-common, SysEx, and real-time messages pass through. The raw-byte filter emits explicit channel status bytes so filtering cannot corrupt downstream running status. There is no MIDI companion device or global MIDI bus.

## Transport sync

New instances start in **Sync** at **1 bar**. Click **Sync** above the Rate knob to switch to **Free**, where the Hz rate is independent of Live's transport. Saved instances retain their chosen mode. In Sync, the same knob selects the duration of one entire contour cycle:

`1/64`, `1/32`, `1/16`, `1/8`, `1/4`, `1/2`, `1 bar`, `2 bars`, `4 bars`.

The initial synced division is **1 bar**. Bar durations follow Live's time signature. Free rate and synced division are separate saved parameters; changing modes does not overwrite either. Double-clicking the knob resets the active rate/division.

Sync uses `phasor~ @lock 1` as a signal-rate transport phase source, not a UI timer or a BPM-to-Hz approximation. It follows Live's tempo and timeline while playing. Live Stop freezes the read position and shows **WAITING**; Play returns to the grid. The device's own Stop remains independent and is never cleared by Live Play.

**Hold and scrub release return to the grid in Sync**, provided both Live and the device are running and no other Hold/grab remains active. This intentionally permits a position/value jump. Position offsets the synced phase; Reverse mirrors that phase. Free retains the previous resume-from-here behavior.

Marker triggers temporarily displace that synced phase so traversal audibly continues from the selected location, rather than snapping back on the next sample. The displacement is cleared by Hold/grab release, device Run re-enable, Live Play, or entering Sync; a stopped/held read position still stays frozen until travel is permitted. Marker seeks never change the saved Position offset.

For a simple rhythmic check, choose **Staircase**, **Sync**, **1 bar** in 4/4: its four steps occupy quarter notes. Start Live, and use a restrained modulation range on one destination.

Replace existing device instances after rebuilding this version: new saved parameters and signal connections require reloading the patch, not just JavaScript autowatch.

## Freeze: direct note-on / note-off

**C0 (MIDI 24 in Live) is the fixed Freeze note.** The previous configurable Hold Note field has been removed. No Live MIDI mapping is required.

- Press C0: pause automatic travel at the current read position.
- Release it: resume from the current position in Free, or return to the grid in Sync, when the required transports are running and no other Hold or grab remains active.
- Position, playhead scrubbing, and marker seeking still work while held.
- The on-screen **Hold** button is momentary: mouse-down holds; release, including outside its bounds, releases.
- MIDI Hold and the automatable Hold parameter are independent sources. Releasing a note does not cancel a held button or automated Hold value.
- Stopped stays stopped. Releasing Hold never enables Run.
- Overlapping held notes are tracked by MIDI channel; releasing one does not release another. Velocity-zero Note On is a release. Sustain does not latch Hold.
- All Notes Off / All Sound Off clear captured notes for their MIDI channel.

Live's global note-to-parameter MIDI mapping can toggle rather than follow note release. Use the device's direct C0 input for momentary keyboard control.

## Playable seek markers

| Marker | Note in Live | MIDI number | Initial position |
| --- | --- | --- | --- |
| 1 | C#0 | 25 | 0% |
| 2 | D0 | 26 | 25% |
| 3 | D#0 | 27 | 50% |
| 4 | E0 | 28 | 75% |

- Click a note label to seek on release; drag it horizontally to edit its location without scrubbing. Small motion under three pixels remains a click.
- Each label has its own vertical lane so all four remain clickable even at coincident positions or endpoints.
- Marker locations are saved, automatable `marker1`–`marker4` parameters, also exposed in the Timing / Markers bank.
- A positive-velocity note-on seeks immediately. Note-offs, including velocity-zero Note On, do nothing to marker playback. Repeated triggers restart from the marker.
- Seek does not enable Run or release Freeze. Scanning continues from the marker only when the current transport state permits it; these are seek points, not bounded slices.
- A MIDI marker triggered during a ruler grab replaces the held coordinate without releasing mouse ownership. The next drag/release can replace it; releasing still follows the normal Sync/Free contract.
- The highlighted label is the last triggered marker, not an indication that its note must remain held.

## Six seed contours

Position and value are normalized to `0–1`.

| Seed | Shape | What it exercises |
| --- | --- | --- |
| **Ramp** | Linear rise from 0 to 1; abrupt loop reset. | Basic traversal, reverse, wrapping. |
| **Triangle** | Rise to 1 at the midpoint, then fall to 0. | Continuous looping and bidirectional scrubbing. |
| **Needle** | Low until 0.35; peak at 0.40; low again by 0.50. | Finding and dwelling on a narrow feature. |
| **Staircase** | Four equal-width levels: 0, ⅓, ⅔, 1. | Discrete changes during traversal. |
| **Gate** | Low for the first quarter, high for the middle half, low for the last quarter. | Plateaus and hard edges. |
| **Landscape** | Low plateau → sharp peak → descending staircase → long fall. | An asymmetric musical phrase with distinct regions. |

**Landscape is the default.** Its points are `(0,.12), (.16,.12), (.22,1), (.28,.80), (.36,.80), (.36,.58), (.46,.58), (.46,.36), (.58,.36), (1,.12)`. Duplicate positions represent downward steps.

Switching seeds preserves actual read position and running state. The output may change because the contour changed, not because the transport reset.

## Traversal contract

| Control | Behavior |
| --- | --- |
| Position | Phase offset, separate from actual read position. Offsets the Live grid in Sync; still scrubs relative to the frozen position while stopped or held. |
| Rate | Hz in Free; whole-contour musical duration in Sync. Modes retain separate values. |
| Run/Stop | Stop freezes position. Run resumes locally in Free or returns to the playing Live grid in Sync. |
| Reverse | Reverses travel continuously in Free; mirrors the transport phase in Sync, which can change position immediately. |
| Playhead grab | Suspends automatic travel and directly controls actual read position. |
| Grab release | Free resumes from the scrubbed position without a jump. Sync returns to the grid. Neither overrides Hold or stopped transport. |
| Seek marker | Jump to the stored position without changing Run, Hold, or Position. Running Sync continues with temporary phase displacement; Free continues locally. |
| Hold | Suspends automatic travel without disabling manual scrubbing or changing Run. |
| Contour ends | Wrap; endpoints can have different values. |

## Verified behavior

Earlier native Max smoke runs exercised raw MIDI through `midiparse`, the control bridge, and `gen~`: note press/release, unrelated notes, Position changes while held, grab/release precedence, stopped transport, combined button/MIDI Hold, overlapping channels, velocity-zero release, sustain independence, and All Notes Off.

Actual mouse interaction confirmed momentary Hold and release outside the button. The rebuilt MIDI AMXD was loaded in a separate Live Suite test set before an instrument: a computer-keyboard MIDI note held the playhead stationary, note release resumed traversal, and MIDI reached the instrument. These are runtime checks, not just patch-structure validation.

The initial separate DSP Hold gate failed in an already-loaded Live Beta instance despite the button reporting HELD. Run and both Hold sources now resolve to one DSP `running` gate, while the host Run parameter stays unchanged. The correction was checked in that existing instance: the playhead remained at 87.9% across a sustained mouse press and advanced after release.

Hold mouse handling uses explicit `onpointerdown` / `onpointerup`, not legacy click/double-click/drag callbacks. Pointer-up releases the mouse-owned Hold regardless of its button-state flag. Live Beta interaction checks covered stationary single presses, double-click-and-hold, dragging outside before release, and twelve rapid clicks; all released without latching.

Transport-sync smoke checks ran the real locked clock and `gen~`: one/two/four-bar durations, quarter notes, 3/4 meter, tempo changes, song-position jumps, held Position changes, grab ownership, grid return, reverse, and Free independence. In a separate Live Suite set, the actual AMXD followed Play/Stop, froze during Hold and scrubbing, returned to the grid on release, displayed WAITING when stopped, and selected a quarter-note cycle via the Rate knob.

The MIDI note and CC paths use `join 2 @triggers 0` to attach the channel before forwarding each event. Earlier `append 1` objects incorrectly declared a second inlet; Max deleted both channel patchcords when loading. A native Max smoke run of the corrected routing confirmed channel preservation for notes on channels 1, 2, and 16, note-off and velocity-zero release, plus channel-specific All Notes Off and All Sound Off messages, without extra events from channel updates.

The playable-marker native Max smoke exercised the actual controller, `gen~`, locked clock, parameter objects, and raw MIDI filter: all four notes, edited marker locations, marker release independence, Freeze across channels, button/MIDI ownership, channel CC resets, stopped/Free/Sync seeking, persistent synced displacement and retriggers, and grid return on Freeze/grab release, Run, and host Play. Scripted pointer events in the actual `v8ui` exercised click-to-seek, parameter-saving drags, repeated clicks at coincident endpoints, release outside the graph, ruler release, and knob double-click reset without desktop input. The production surface was loaded and captured in native Max.

`node --test scrubbles-midi.test.mjs` covers the reserved-note boundaries across all channels, running status across discarded messages, real-time interruption, and preservation of non-control channel/system messages. Patch-structure validation alone does not check these runtime semantics.

## Deferred design

- Contour authoring, point editing, and segment bending.
- Span, one-shot / ping-pong traversal, and retrigger policies.
- Multiple contours / destinations and transport-phase persistence.
