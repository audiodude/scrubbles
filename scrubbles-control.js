// Message-rate control only. Transport and contour evaluation live in gen~.
autowatch = 1;
inlets = 1;
outlets = 5;

// Outlets: surface/status, gen~ parameters, modulation attributes,
// live.map commands, live.modulate~ destination IDs.
var device = this.patcher;
var parameterNames = ["seed", "position", "rate", "amount", "running", "reverse", "bipolar", "hold", "marker1", "marker2", "marker3", "marker4", "sync", "division"];
var limits = {
    seed: [0, 5, true],
    position: [0, 1, false],
    rate: [0.02, 8, false],
    amount: [0, 1, false],
    running: [0, 1, true],
    reverse: [0, 1, true],
    bipolar: [0, 1, true],
    hold: [0, 1, true],
    marker1: [0, 1, false],
    marker2: [0, 1, false],
    marker3: [0, 1, false],
    marker4: [0, 1, false],
    sync: [0, 1, true],
    division: [0, 8, true]
};
var parameterObjects = {};
var divisionDurations = ["64n", "32n", "16n", "8n", "4n", "2n", "1.0.0", "2.0.0", "4.0.0"];
var syncClock = null;
var hostPlaying = 0;
var grabbing = false;
var seekGeneration = 0;
var gridGeneration = 0;
var markerPositions = [0, 0.25, 0.5, 0.75];
var deviceEnabled = true;
var destinationId = 0;
var destinationName = [];
var mappingActive = 0;
var runParameter = 1;
var holdParameter = 0;
var syncParameter = 1;
var midiHeld = false;
var heldNotes = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
var heldCount = 0;

function returnToGrid() {
    // Cancel a pending marker's displacement if a later grid-return event
    // arrives before the next DSP vector. A later seek sets markerseek again.
    outlet(1, "markerseek", 0);
    gridGeneration = (gridGeneration + 1) % 1000000;
    outlet(1, "gridgeneration", gridGeneration);
}
returnToGrid.local = 1;

function updateHold() {
    // Preserve the host's Run value; DSP receives permission to advance.
    if (midiHeld && heldCount === 0) returnToGrid();
    midiHeld = heldCount > 0;
    outlet(1, "running", runParameter && !holdParameter && heldCount === 0 ? 1 : 0);
    outlet(0, "state", "midiheld", heldCount > 0 ? 1 : 0);
    parameterObject("hold").message("midiheld", heldCount > 0 ? 1 : 0);
}
updateHold.local = 1;

function releaseNotes() {
    for (var i = 0; i < heldNotes.length; ++i) heldNotes[i] = 0;
    heldCount = 0;
    updateHold();
}
releaseNotes.local = 1;

function note(pitch, velocity, channel) {
    if (!deviceEnabled || channel < 1 || channel > 16) return;
    if (pitch >= 25 && pitch <= 28) {
        if (velocity > 0) seekmarker(pitch - 25);
        return;
    }
    if (pitch !== 24) return;
    var index = channel - 1;
    if (velocity > 0) {
        ++heldNotes[index];
        ++heldCount;
    } else if (heldNotes[index] > 0) {
        --heldNotes[index];
        --heldCount;
    }
    updateHold();
}

function cc(controller, value, channel) {
    // Freeze follows physical keys, not sustain pedal state.
    if ((controller !== 120 && controller !== 123) || channel < 1 || channel > 16) return;
    heldCount -= heldNotes[channel - 1];
    heldNotes[channel - 1] = 0;
    updateHold();
}

function parameterObject(name) {
    if (!parameterObjects[name])
        parameterObjects[name] = device.getnamed("p_" + name);
    return parameterObjects[name];
}
parameterObject.local = 1;

function bounded(name, value) {
    var range = limits[name];
    if (!range || typeof value !== "number" || !isFinite(value))
        return null;
    value = Math.max(range[0], Math.min(range[1], value));
    return range[2] ? Math.round(value) : value;
}
bounded.local = 1;

function param(name, value) {
    value = bounded(name, value);
    if (value === null)
        return;
    // The Live object owns storage/automation and echoes through changed().
    // Never echo a UI edit straight back as another param message.
    parameterObject(name).message(value);
}

function changed(name, value) {
    value = bounded(name, value);
    if (value === null)
        return;
    if (name === "running") {
        if (!runParameter && value) returnToGrid();
        runParameter = value;
        updateHold();
    } else if (name === "hold") {
        if (holdParameter && !value) returnToGrid();
        holdParameter = value;
        updateHold();
    } else if (name === "sync") {
        if (!syncParameter && value) returnToGrid();
        syncParameter = value;
        outlet(1, name, value);
    } else if (name.substring(0, 6) === "marker") {
        markerPositions[Number(name.charAt(6)) - 1] = value;
    } else if (name === "division") {
        if (!syncClock) syncClock = device.getnamed("syncclock");
        syncClock.message(divisionDurations[value]);
    } else if (name === "amount")
        outlet(2, "depth", value);
    else
        outlet(1, name, value);
    outlet(0, "state", name, value);
}

function hostplaying(value) {
    if (typeof value !== "number" || !isFinite(value)) return;
    if (!hostPlaying && value !== 0) returnToGrid();
    hostPlaying = value !== 0 ? 1 : 0;
    outlet(1, "hostplaying", hostPlaying);
    outlet(0, "state", "hostplaying", hostPlaying);
}

function bang() {
    // Replay the actual restored parameter values, never initialization values.
    hostplaying(hostPlaying);
    for (var i = 0; i < parameterNames.length; ++i)
        parameterObject(parameterNames[i]).message("outputvalue");
    outlet(0, ["target"].concat(destinationName));
    outlet(0, "state", "midiheld", heldCount > 0 ? 1 : 0);
    outlet(0, "mapping", mappingActive);
}

function ready() {
    // live.thisdevice also supplies this readiness bang in standalone Max.
    // It fires again on preset restoration: do not reset gen~ or live.map here.
    bang();
}

function seekmarker(index) {
    if (typeof index !== "number" || !isFinite(index) ||
        index !== Math.floor(index) || index < 0 || index > 3)
        return;
    // A marker can replace the held ruler coordinate without ending its grab.
    // The next mouse drag/release still owns the gesture and can replace it.
    outlet(1, "seekposition", markerPositions[index]);
    outlet(1, "markerseek", 1);
    seekGeneration = (seekGeneration + 1) % 1000000;
    outlet(1, "seekgeneration", seekGeneration);
    outlet(0, "markeractive", index);
}

function grab(active, position) {
    if (typeof active !== "number" || !isFinite(active) ||
        typeof position !== "number" || !isFinite(position))
        return;
    active = active !== 0;
    if (!active && !grabbing)
        return;
    position = Math.max(0, Math.min(1, position));
    if (!active) returnToGrid();
    grabbing = active;
    outlet(1, "seekposition", position);
    outlet(1, "grabbed", active ? 1 : 0);
    outlet(1, "markerseek", 0);
    // A whole down/drag/up gesture may occur between two DSP vectors. Its
    // final seek must survive even when gen~ never observes grabbed == 1.
    seekGeneration = (seekGeneration + 1) % 1000000;
    outlet(1, "seekgeneration", seekGeneration);
}

function map(active) {
    if (typeof active !== "number" || !isFinite(active))
        return;
    if (!deviceEnabled && active !== 0) {
        outlet(0, "mapping", 0);
        outlet(0, "status", "Enable the device before mapping");
        return;
    }
    outlet(3, "mapping", active !== 0 ? 1 : 0);
}

function unmap() {
    // Detach immediately, including while disabled. live.map clears its own
    // persistent destination and reports the resulting name/mapping state.
    destinationId = 0;
    destinationName = [];
    mappingActive = 0;
    outlet(4, "id", 0);
    outlet(3, "cancel");
    outlet(3, "unmap");
    outlet(0, "target");
    outlet(0, "mapping", 0);
}

function mapped() {
    var args = arrayfromargs(arguments);
    if (args[0] === "id")
        args.shift();
    var id = Number(args[0]);
    if (!isFinite(id) || id < 0)
        return;
    destinationId = Math.floor(id);
    // IDs originate only from live.map; no target base value is ever written.
    outlet(4, "id", deviceEnabled ? destinationId : 0);
    if (destinationId === 0) {
        destinationName = [];
        outlet(0, "target");
    }
}

function targetname() {
    var args = arrayfromargs(arguments);
    if (args[0] === "name")
        args.shift();
    destinationName = !args.length || args[0] === "<none>" ? [] : args;
    outlet(0, ["target"].concat(destinationName));
}

function mappingstate() {
    var args = arrayfromargs(arguments);
    if (args[0] === "mapping")
        args.shift();
    mappingActive = Number(args[0]) !== 0 ? 1 : 0;
    outlet(0, "mapping", mappingActive);
}

function enabled(value) {
    deviceEnabled = value !== 0;
    outlet(0, "status");
    if (!deviceEnabled) {
        releaseNotes();
        outlet(3, "cancel");
        mappingActive = 0;
        outlet(0, "mapping", 0);
    }
    // Disabling must remove modulation, not send zero (which has a nonzero
    // meaning for Live's target-defined unipolar modulation). Keep the map.
    outlet(4, "id", deviceEnabled ? destinationId : 0);
}
