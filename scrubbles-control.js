// Message-rate control only. Transport and contour evaluation live in gen~.
autowatch = 1;
inlets = 1;
outlets = 5;

// Outlets: surface/status, gen~ parameters, modulation attributes,
// live.map commands, live.modulate~ destination IDs.
var device = this.patcher;
var parameterNames = ["seed", "position", "rate", "amount", "running", "reverse", "bipolar", "hold", "holdnote", "sync", "division"];
var limits = {
    seed: [0, 5, true],
    position: [0, 1, false],
    rate: [0.02, 8, false],
    amount: [0, 1, false],
    running: [0, 1, true],
    reverse: [0, 1, true],
    bipolar: [0, 1, true],
    hold: [0, 1, true],
    holdnote: [0, 127, true],
    sync: [0, 1, true],
    division: [0, 8, true]
};
var parameterObjects = {};
var divisionDurations = ["64n", "32n", "16n", "8n", "4n", "2n", "1.0.0", "2.0.0", "4.0.0"];
var syncClock = null;
var hostPlaying = 0;
var grabbing = false;
var seekGeneration = 0;
var deviceEnabled = true;
var destinationId = 0;
var destinationName = [];
var mappingActive = 0;
var runParameter = 1;
var holdParameter = 0;
var holdNote = 36;
var heldNotes = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
var heldCount = 0;

function updateHold() {
    // Preserve the host's Run value; DSP receives permission to advance.
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
    if (!deviceEnabled || pitch !== holdNote || channel < 1 || channel > 16) return;
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
    // Hold follows physical keys, not sustain pedal state.
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
        runParameter = value;
        updateHold();
    } else if (name === "hold") {
        holdParameter = value;
        updateHold();
    } else if (name === "holdnote") {
        if (holdNote !== value) releaseNotes();
        holdNote = value;
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

function grab(active, position) {
    if (typeof active !== "number" || !isFinite(active) ||
        typeof position !== "number" || !isFinite(position))
        return;
    active = active !== 0;
    if (!active && !grabbing)
        return;
    position = Math.max(0, Math.min(1, position));
    grabbing = active;
    outlet(1, "seekposition", position);
    outlet(1, "grabbed", active ? 1 : 0);
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
