# Scrubbles — Brainstorming Notes

Working name: **Scrubbles**

Status: early concept exploration. No implementation yet. The core intent below comes from the discussion; proposed controls and interactions remain candidates unless explicitly settled.

## Core intent

Build a simple subtractive synthesizer, with the Juno-106 as a reference for simplicity rather than a commitment to exact circuit emulation. The differentiator is its modulation process, not a complicated sound engine.

Use the same broad architecture as `../../vibes/vape-station/`:

- C++17 and JUCE 8, built with CMake, for VST3 and standalone applications.
- A native JUCE editor, not an embedded web UI.
- Polyphonic voices, centralized parameter definitions, host automation, and saved patch state.
- Separation between the sound engine, modulation system, and UI. Modulation should not overwrite host-visible base parameter values.
- A separate JavaScript DSP implementation running in an AudioWorklet for the browser version.
- Offline MIDI/audio rendering through the real engine for verification.

This is an architectural reference, not a decision to copy VapeStation's modulation matrix, controls, or sound engine. Delivery order and platform scope have not been specified.

## The central idea: editable contours, movable time

Envelopes should be easy to edit expressively. Instead of an envelope being solely a function of elapsed time, control signals modulate its read position—the envelope's “time” index.

Conceptually:

`modulation(t) = contour(position(t))`

The contour defines what happens; the position signal defines how it is traversed.

Examples:

- A rising ramp reads the contour forward.
- A triangle scans back and forth.
- A stepped signal visits discrete locations.
- A stationary position holds one contour value.

The desired result is evolving, sequence-like patches without simply building a conventional step sequencer.

Two priorities accompany this model:

1. An opinionated, limited set of modulation destinations rather than making every parameter a target.
2. An editing mechanism somewhere between ADSR dials and a freehand pencil tool.

## First concrete example: a filter-frequency contour

The main example discussed is a contour controlling filter cutoff, with controls for:

- Position, something akin to phase.
- Speed.
- Direction.
- Potentially a few additional controls that make one contour musically versatile.

Both ways of playing are essential:

- A repeating phrase that can be reshaped while it runs.
- A timbral path that can be scrubbed manually, with automatic movement available.

These should coexist, rather than requiring separate sequencer and manual modes.

## Combining manual position and automatic motion

The proposed model is:

`read position = position offset + accumulated automatic travel`

This describes the combination, not a complete algorithm. Wrapping, endpoints, span, and traversal modes still need precise definitions.

### Proposed behavior

- With rate at zero, the Position control scrubs relative to the stopped transport position. Leaving it alone holds the sound there.
- With a nonzero rate, automatic travel advances through the contour; Position shifts the running phrase.
- Moving Position while running combines the manual gesture with automatic motion: push ahead, pull backward, or temporarily counteract the motion.
- An external control signal could modulate Position in the same way.
- Reverse changes the direction of automatic travel, not the meaning of a manual gesture.

An important proposed continuity rule: stopping should freeze the current read position, not jump back to the Position knob's nominal value. Accumulated travel is therefore retained when stopped.

Consequently, the Position knob is a phase/position offset, not always an absolute readout of where the contour is being read. The visible playhead shows the actual read position. A separate reset gesture could return automatic travel to its origin.

### Position offset versus seek

Two distinct interactions were discussed:

- **Phase offset:** shift the position being read while automatic travel continues. Returning the offset restores the underlying phase relationship, assuming other transport settings have not changed.
- **Seek:** move the playhead to a chosen location, then continue playback from there.

The proposed direction is to provide both through different gestures rather than overload one knob with incompatible meanings.

### Direct playhead grabbing

A proposed interaction on the contour itself:

1. Grab the playhead to take direct control of the read position.
2. Suspend automatic travel while it is grabbed.
3. Scrub or dwell anywhere in the contour.
4. On release, resume travel from that spot at the selected rate and direction.
5. If rate is zero, remain at the chosen position.

The resulting distinction:

- **Position knob:** shift or modulate the phrase without stopping its motion.
- **Grab playhead:** take over and perform it directly.

Example: a tempo-synced filter pattern runs through a held chord. Grab the playhead, linger on a bright peak, scrub through darker shapes, and release it back into motion.

This interaction was proposed during brainstorming; its exact state handling, automation behavior, and interaction with simultaneous position modulation are unresolved.

### Rhythmic alignment after manual intervention

Two release behaviors remain possible:

- Resume from the manually chosen position, preserving positional continuity.
- Rejoin the original clocked phrase, preserving its rhythmic alignment.

The suggested default was the first, with an explicit “return to sync” action for the second. This has not been finalized. Tempo-synced speed and clock-aligned phase are separate concerns.

## Candidate controls

| Control | Intended musical role |
| --- | --- |
| Position | Scrub when stopped; shift the phrase while running. |
| Rate | Set traversal speed, with tempo-synced and free-running options under consideration. |
| Direction | Choose forward or reverse travel. |
| Span | Read the whole contour or a smaller region. |
| Depth | Scale the contour's effect around a base cutoff frequency. |

**Span** was the strongest additional-control candidate: a small region produces local motion, while widening it introduces larger gestures without redrawing the contour.

Example contour: a low plateau, a sharp peak, a descending staircase, and a long fall. Narrow the span around the staircase for a repeating figure; widen it to bring the peak and fall into the phrase.

The exact relationship between Position, Span, and the selected region is still open.

A proposed compact panel was:

**Position · Rate · Span · Depth**, with **Reverse** and **Ping-pong** as buttons.

### Other possibilities—not a feature commitment

- **Travel shape:** redistribute time within the contour, lingering in one part and rushing through another without changing the drawn values.
- **Glide:** smooth the resulting modulation output so steps become slides. Output smoothing is distinct from smoothing the playhead: moving the playhead between two locations would read intervening contour features.
- **Loop / ping-pong / one-shot:** alternative traversals of the same contour. Looping can introduce a discontinuity at the seam; ping-pong avoids the wrap jump but reverses the phrase.
- **Retrigger:** restart per note, restart on the first held note, or run globally. Global motion gives a chord a shared gesture; per-note motion produces overlapping phrases.

## Contour editing

The editing goal is expressive control with constrained complexity, between ADSR knobs and freehand drawing.

### Breakpoint-editor proposal

- Drag points horizontally to change their position and vertically to change their value.
- Bend a segment to change its curvature.
- Add and remove points deliberately; no freehand pencil drawing.
- Optionally snap positions for rhythmic structure.
- Support explicit hold/step segments for sequence-like movement.

ADSR-like shapes would be a simple case rather than the only available shape. Valleys, repeated peaks, plateaus, and staircases should also be natural to create.

### Gesture-oriented interpretation

Instead of emphasizing lots of tiny points, the editor could emphasize a few musical gestures: rise, fall, hold, and jump. Drag a junction to change position and height; drag within a segment to bend it.

These are related editing proposals, not yet separate modes or a finalized editor design. Point limits, snapping rules, segment types, and exact gestures remain open.

For filter cutoff, equal vertical distances should represent equal frequency ratios rather than equal increments in hertz, so the shape reads musically across the frequency range.

## Opinionated modulation destinations

Filter cutoff is the concrete starting example. Other destinations suggested during discussion were:

| Candidate | Musical purpose |
| --- | --- |
| Filter cutoff | Primary timbral movement. |
| Oscillator pulse width | Movement without changing the played note. |
| Pitch | Melodic patterns; requires deliberate quantization semantics. |
| Level | Rhythmic articulation. |

Resonance, noise/sub balance, and effects could remain ordinary controls initially. This is a scope proposal, not a settled parameter list.

A separate note-gated amplitude envelope was also proposed so that stopping or reversing a contour cannot prevent note-off from releasing a voice.

## Open design questions

- Do multiple destination contours share a playhead, have independent playheads, or support a deliberately limited combination?
- What control signals can drive Position, Rate, and other traversal parameters?
- How do direct position control and integrated speed control coexist at the modulation-routing level?
- How do Position and Span define the read region, especially near endpoints?
- What are the exact wrap, reverse, ping-pong, one-shot, and reset semantics?
- How should grabbing, releasing, stopping, and returning to sync affect transport state and host automation?
- How is “return to sync” expressed musically and visually?
- Which playback/retrigger behaviors are global versus per voice?
- How are base cutoff, depth, polarity, and frequency limits presented?
- Which editing constraints make contours quick to shape without making them restrictive?
- Which destinations earn a place in the first instrument?

## Direction so far

Keep the subtractive synth simple. Make the distinctive instrument out of the relationship between an expressive contour and its traversal controls. Automatic sequence-like motion and direct scrubbing are both central, not competing modes.

## Subsequent direction: note-assigned seek markers

The current prototype is a Max for Live modulation device rather than a synth. The scrub playhead is the strongest interaction so far.

Requested next direction:

- Place playhead/seek markers at chosen locations on the envelope/contour.
- Assign a MIDI note to each marker; triggering that note skips the read position to that location.
- Automatic scanning is orthogonal to these triggers, not a mutually exclusive partition-playback mode.
- Receive notes directly in Scrubbles: use a Max MIDI Effect with internal signal-rate modulation processing, not an Audio Effect plus a MIDI companion.

This revises the earlier idea of independently triggered partitions with individual durations. Marker count, note assignments, retrigger/overlap behavior, and whether a held trigger also pauses scanning remain undecided. Seek markers are a design note, not part of the current Hold change.
