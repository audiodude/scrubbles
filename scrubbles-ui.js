// Scrubbles: native Max v8ui surface. Transport is exclusively DSP-owned.
autowatch = 1;
inlets = 1;
outlets = 1;
mgraphics.init();
mgraphics.relative_coords = 0;
mgraphics.autofill = 0;

const W = 760;
const H = 169;
const TAU = Math.PI * 2;
const RATE_MIN = 0.02;
const RATE_MAX = 8;
const RATE_LOG = Math.log(RATE_MAX / RATE_MIN);
const defaults = { seed: 5, position: 0, rate: 0.25, sync: 1, division: 6, hostplaying: 0, amount: 0.5, running: 1, hold: 0, midiheld: 0, reverse: 0, bipolar: 1, marker1: 0, marker2: 0.25, marker3: 0.5, marker4: 0.75 };
const values = Object.assign({}, defaults);
const seeds = ["Ramp", "Triangle", "Needle", "Staircase", "Gate", "Landscape"];
const divisions = ["1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1 bar", "2 bars", "4 bars"];
// Duplicate x coordinates deliberately draw the exact, hard step edges.
const contours = [
    [[0, 0], [1, 1]],
    [[0, 0], [0.5, 1], [1, 0]],
    [[0, 0], [0.35, 0], [0.40, 1], [0.50, 0], [1, 0]],
    [[0, 0], [0.25, 0], [0.25, 1 / 3], [0.50, 1 / 3], [0.50, 2 / 3], [0.75, 2 / 3], [0.75, 1], [1, 1]],
    [[0, 0], [0.25, 0], [0.25, 1], [0.75, 1], [0.75, 0], [1, 0]],
    [[0, 0.12], [0.16, 0.12], [0.22, 1], [0.28, 0.80], [0.36, 0.80], [0.36, 0.58], [0.46, 0.58], [0.46, 0.36], [0.58, 0.36], [1, 0.12]]
];
const color = {
    background: [0.075, 0.084, 0.085, 1],
    panel: [0.102, 0.115, 0.115, 1],
    inset: [0.054, 0.064, 0.065, 1],
    line: [0.21, 0.25, 0.25, 1],
    muted: [0.54, 0.60, 0.59, 1],
    text: [0.88, 0.88, 0.83, 1],
    teal: [0.34, 0.83, 0.72, 1],
    tealDim: [0.14, 0.26, 0.24, 1],
    amber: [0.96, 0.69, 0.40, 1],
    amberDim: [0.29, 0.23, 0.16, 1],
    error: [1, 0.48, 0.38, 1]
};
const graph = { x: 17, y: 54, w: 450, h: 60 };
const ruler = { x: 12, y: 117, w: 460, h: 23 };
const markerNames = ["marker1", "marker2", "marker3", "marker4"];
const markerNotes = ["C#0", "D0", "D#0", "E0"];
const controls = [
    { name: "position", label: "POSITION", x: 491, y: 58, w: 77, h: 79 },
    { name: "rate", label: "RATE", x: 579, y: 58, w: 77, h: 79 },
    { name: "amount", label: "AMOUNT", x: 667, y: 58, w: 77, h: 79 }
];
const seedButtons = [];
const seedWidths = [45, 61, 52, 69, 44, 77];
let seedX = 144;
for (let i = 0; i < seeds.length; i++) {
    seedButtons.push({ x: seedX, y: 10, w: seedWidths[i], h: 22, seed: i });
    seedX += seedWidths[i] + 3;
}
const buttons = {
    running: { x: 13, y: 144, w: 64, h: 20 },
    reverse: { x: 83, y: 144, w: 76, h: 20 },
    bipolar: { x: 165, y: 144, w: 96, h: 20 },
    sync: { x: 587, y: 54, w: 62, h: 17 },
    map: { x: 616, y: 10, w: 59, h: 22 },
    unmap: { x: 681, y: 10, w: 63, h: 22 }
};
let actualPosition = 0;
let actualValue = 0.12;
let activeMarker = -1;
let mappingArmed = false;
let targetName = "";
let statusText = "";
let errorText = "";
let hover = "";
let drag = null;

function clamp(v, lo, hi) { return Math.max(lo, Math.min(hi, v)); }
function inside(x, y, r) { return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h; }
function source(c) { mgraphics.set_source_rgba(c); }
function line(x1, y1, x2, y2, c, width) {
    source(c);
    mgraphics.set_line_width(width || 1);
    mgraphics.move_to(x1, y1);
    mgraphics.line_to(x2, y2);
    mgraphics.stroke();
}
function panel(x, y, w, h, c, radius) {
    source(c);
    mgraphics.rectangle_rounded(x, y, w, h, radius || 4, radius || 4);
    mgraphics.fill();
}
function text(s, x, y, size, c, align, width) {
    mgraphics.select_font_face("Arial");
    mgraphics.set_font_size(size);
    let display = String(s);
    if (width !== undefined && mgraphics.text_measure(display)[0] > width) {
        while (display.length && mgraphics.text_measure(display + "…")[0] > width) display = display.slice(0, -1);
        display += "…";
    }
    const tw = mgraphics.text_measure(display)[0];
    source(c);
    mgraphics.move_to(align === "center" ? x - tw / 2 : align === "right" ? x - tw : x, y);
    mgraphics.show_text(display);
}
function button(r, label, active, hot, accent) {
    panel(r.x, r.y, r.w, r.h, active ? (accent ? color.amberDim : color.tealDim) : hot ? color.line : color.panel, 4);
    text(label, r.x + r.w / 2, r.y + r.h / 2 + 3.5, 10, active ? (accent ? color.amber : color.teal) : color.muted, "center", r.w - 8);
}
function controlParam(name) { return name === "rate" && values.sync ? "division" : name; }
function normalized(name) {
    if (name === "division") return values.division / (divisions.length - 1);
    return name === "rate" ? Math.log(values.rate / RATE_MIN) / RATE_LOG : values[name];
}
function valueText(name) {
    if (name === "rate") return values.sync ? divisions[values.division] : values.rate.toFixed(values.rate < 1 ? 2 : 1) + " Hz";
    return (values[name] * 100).toFixed(1) + "%";
}
function drawControl(c) {
    const cx = c.x + c.w / 2;
    const cy = 91;
    const active = drag && drag.name === c.name;
    if (c.name !== "rate") text(c.label, cx, 64, 9, active ? color.teal : color.muted, "center");
    source(color.line);
    mgraphics.set_line_width(3);
    mgraphics.arc(cx, cy, 17, Math.PI * 0.75, Math.PI * 2.25);
    mgraphics.stroke();
    const angle = Math.PI * (0.75 + clamp(normalized(controlParam(c.name)), 0, 1) * 1.5);
    source(color.teal);
    mgraphics.arc(cx, cy, 17, Math.PI * 0.75, angle);
    mgraphics.stroke();
    source(hover === c.name || active ? color.line : color.panel);
    mgraphics.arc(cx, cy, 12.5, 0, TAU);
    mgraphics.fill();
    line(cx + Math.cos(angle) * 5, cy + Math.sin(angle) * 5, cx + Math.cos(angle) * 10, cy + Math.sin(angle) * 10, color.text, 1.7);
    text(valueText(c.name), cx, 128, 12, color.text, "center");
}
function contourPath(points) {
    mgraphics.move_to(graph.x + points[0][0] * graph.w, graph.y + (1 - points[0][1]) * graph.h);
    for (let i = 1; i < points.length; i++) {
        mgraphics.line_to(graph.x + points[i][0] * graph.w, graph.y + (1 - points[i][1]) * graph.h);
    }
}
// Separate vertical label lanes keep coincident markers individually reachable.
function markerRect(index) {
    const x = graph.x + values[markerNames[index]] * graph.w;
    return { x: clamp(x - 16, graph.x, graph.x + graph.w - 32), y: graph.y + index * 15, w: 32, h: 14 };
}
function drawMarkers() {
    for (let i = 0; i < markerNames.length; i++) {
        const x = graph.x + values[markerNames[i]] * graph.w;
        line(x, graph.y, x, graph.y + graph.h, activeMarker === i ? color.amberDim : color.tealDim);
    }
    for (let i = 0; i < markerNames.length; i++) {
        const name = markerNames[i];
        const x = graph.x + values[name] * graph.w;
        const r = markerRect(i);
        const active = activeMarker === i;
        const hot = hover === name || (drag && drag.name === name);
        panel(r.x, r.y, r.w, r.h, hot ? color.line : active ? color.amberDim : color.panel, 2);
        text(markerNotes[i], r.x + r.w / 2, r.y + 10, 9, active ? color.amber : color.teal, "center");
        // The notch, unlike the edge-clamped label, is at the exact saved position.
        line(x, r.y + 11, x, r.y + 14, active ? color.amber : color.teal, 2);
    }
}
function drawGraph() {
    panel(12, 49, 460, 67, color.inset, 3);
    for (let i = 0; i <= 4; i++) {
        const x = graph.x + graph.w * i / 4;
        line(x, graph.y, x, graph.y + graph.h, [0.19, 0.23, 0.23, 0.55]);
    }
    line(graph.x, graph.y + graph.h / 2, graph.x + graph.w, graph.y + graph.h / 2, [0.19, 0.23, 0.23, 0.55]);
    const points = contours[values.seed];
    contourPath(points);
    mgraphics.line_to(graph.x + graph.w, graph.y + graph.h);
    mgraphics.line_to(graph.x, graph.y + graph.h);
    mgraphics.close_path();
    source([0.34, 0.83, 0.72, 0.09]);
    mgraphics.fill();
    contourPath(points);
    source(color.teal);
    mgraphics.set_line_width(1.7);
    mgraphics.stroke();
    const px = graph.x + actualPosition * graph.w;
    const py = graph.y + (1 - actualValue) * graph.h;
    line(px, graph.y - 2, px, graph.y + graph.h + 2, [0.96, 0.69, 0.40, 0.65]);
    source(color.amber);
    mgraphics.arc(px, py, 3, 0, TAU);
    mgraphics.fill();
    drawMarkers();
    panel(ruler.x, ruler.y, ruler.w, ruler.h, hover === "grab" || (drag && drag.name === "grab") ? color.panel : color.background, 3);
    line(graph.x, 126, graph.x + graph.w, 126, color.line);
    for (let i = 0; i <= 8; i++) {
        const x = graph.x + graph.w * i / 8;
        line(x, 124, x, i % 2 ? 128 : 130, color.muted);
    }
    panel(px - 5, 120, 10, 13, color.amber, 3);
    line(px, 123, px, 130, color.amberDim);
    text("CONTOUR / " + seeds[values.seed].toUpperCase(), 17, 44, 9, color.muted);
    text("READ " + (actualPosition * 100).toFixed(1) + "%", 467, 44, 9, color.amber, "right");
}
function hint() {
    if (errorText) return errorText;
    if (mappingArmed) return "Mapping armed · click a parameter in Live · Map to cancel";
    const name = drag ? drag.name : hover;
    const marker = markerNames.indexOf(name);
    if (marker !== -1) return markerNotes[marker] + " · click to seek · drag to save " + (values[name] * 100).toFixed(1) + "%";
    if (name === "grab") return !values.running ? "Scrub · stays stopped on release" : values.hold || values.midiheld ? "Scrub · stays held on release" : values.sync ? "Scrub · release returns to grid" : "Scrub · release resumes here";
    if (name === "position") return "Phase offset · drag up/down · Shift: fine · double-click: reset";
    if (name === "rate") return values.sync ? "Whole contour cycle: " + divisions[values.division] + (values.division < 6 ? " note" : "") : "Cycles per second · logarithmic · Shift: fine · double-click: reset";
    if (name === "sync") return values.sync ? "Sync: Live grid · knob sets whole cycle" : "Free: Hz rate · click for Live Sync";
    if (name === "amount") return "Modulation depth · base value unchanged · double-click: reset";
    if (name === "running") return values.hold || values.midiheld ? values.sync ? "Held · release button + note to grid" : "Hold keeps travel paused" : values.sync ? "Stop freezes · Run returns to Live grid" : "Stop freezes · Run resumes";
    if (name === "reverse") return values.sync ? "Reverse mirrors the Live grid · scrubbing stays direct" : "Reverse automatic travel only · scrubbing stays direct";
    if (name === "bipolar") return "Bipolar: −1…+1 · Unipolar: 0…1 · target defines response";
    if (name === "map") return "Map: arm, then click the destination parameter in Live";
    if (name === "unmap") return "Unmap: release the destination without changing its base";
    if (name === "seed") return "Choose a contour · keep the current position and transport";
    if (name === "graph") return "Drag note labels to place markers · ruler to scrub";
    if (name === "target" && targetName) return targetName;
    if (values.sync) {
        if (!values.running) return "Stopped · Run returns to Live grid";
        if (values.hold || values.midiheld) return "Held · release button + note to grid";
        if (!values.hostplaying) return "Sync waiting · start Live transport";
    }
    if (statusText) return statusText;
    if (!values.running) return "Stopped · drag ruler to scrub · Run resumes travel";
    if (values.hold || values.midiheld) return "Held · release button and note to resume";
    return "Drag the ruler to scrub · map a Live parameter to modulate";
}
function paint() {
    source(color.background);
    mgraphics.rectangle(0, 0, W, H);
    mgraphics.fill();
    text("SCRUBBLES", 14, 25, 17, color.text);
    for (let i = 0; i < seedButtons.length; i++) button(seedButtons[i], seeds[i], values.seed === i, hover === "seed" && hoverSeed === i, false);
    button(buttons.map, mappingArmed ? "Cancel" : "Map", mappingArmed, hover === "map", true);
    button(buttons.unmap, "Unmap", false, hover === "unmap", false);
    text(mappingArmed ? "SELECT A PARAMETER IN LIVE" : targetName ? "TO  " + targetName : "NO TARGET · CLICK MAP", 494, 46, 10, mappingArmed ? color.amber : targetName ? color.teal : color.muted, "left", 250);
    drawGraph();
    line(481, 53, 481, 134, color.line);
    for (let i = 0; i < controls.length; i++) drawControl(controls[i]);
    button(buttons.sync, values.sync ? "Sync" : "Free", !!values.sync, hover === "sync", false);
    line(13, 141, 744, 141, color.line);
    button(buttons.running, values.running ? values.hold || values.midiheld ? "HELD" : values.sync && !values.hostplaying ? "WAITING" : "RUNNING" : "STOPPED", !!values.running, hover === "running", !!(values.hold || values.midiheld));
    button(buttons.reverse, "REVERSE", !!values.reverse, hover === "reverse", false);
    button(buttons.bipolar, values.bipolar ? "BIPOLAR ±" : "UNIPOLAR +", !!values.bipolar, hover === "bipolar", false);
    text(hint(), 340, 158, 10, errorText ? color.error : mappingArmed ? color.amber : color.muted, "left", 235);
    text("C0 FREEZE", 744, 158, 9, values.midiheld ? color.amber : color.muted, "right");
}

function state(name, value) {
    if (!Object.prototype.hasOwnProperty.call(defaults, name)) return;
    const number = Number(value);
    if (!Number.isFinite(number)) return;
    values[name] = name === "seed" ? Math.round(clamp(number, 0, 5)) : name === "division" ? Math.round(clamp(number, 0, divisions.length - 1)) : name === "rate" ? clamp(number, RATE_MIN, RATE_MAX) : name === "sync" || name === "hostplaying" || name === "running" || name === "hold" || name === "reverse" || name === "bipolar" ? (number >= 0.5 ? 1 : 0) : clamp(number, 0, 1);
    mgraphics.redraw();
}
function markeractive(index) {
    const number = Number(index);
    if (!Number.isInteger(number) || number < 0 || number >= markerNames.length) return;
    activeMarker = number;
    mgraphics.redraw();
}
function telemetry(position, value) {
    const p = Number(position);
    const v = Number(value);
    if (Number.isFinite(p)) actualPosition = clamp(p, 0, 1);
    if (Number.isFinite(v)) actualValue = clamp(v, 0, 1);
    mgraphics.redraw();
}
function target() {
    targetName = Array.from(arguments).join(" ");
    errorText = "";
    mgraphics.redraw();
}
function mapping(active) {
    mappingArmed = Number(active) !== 0;
    if (mappingArmed) errorText = "";
    mgraphics.redraw();
}
function status() {
    statusText = Array.from(arguments).join(" ");
    errorText = "";
    mgraphics.redraw();
}
function error() {
    errorText = Array.from(arguments).join(" ");
    mgraphics.redraw();
}
function bang() { mgraphics.redraw(); }
function sendParam(name, value) {
    state(name, value);
    outlet(0, "param", name, values[name]);
}
let hoverSeed = -1;
function hit(x, y) {
    hoverSeed = -1;
    for (let i = 0; i < seedButtons.length; i++) {
        if (inside(x, y, seedButtons[i])) { hoverSeed = i; return "seed"; }
    }
    for (const name in buttons) if (inside(x, y, buttons[name])) return name;
    for (let i = 0; i < controls.length; i++) if (inside(x, y, controls[i])) return controls[i].name;
    for (let i = 0; i < markerNames.length; i++) if (inside(x, y, markerRect(i))) return markerNames[i];
    if (inside(x, y, ruler)) return "grab";
    if (inside(x, y, graph)) return "graph";
    if (x >= 494 && x <= 744 && y >= 34 && y < 55) return "target";
    return "";
}
function scrubPosition(x) { return clamp((x - graph.x) / graph.w, 0, 1); }
function beginGesture(x, y) {
    hover = hit(x, y);
    errorText = "";
    if (hover === "seed") sendParam("seed", hoverSeed);
    else if (hover === "sync" || hover === "running" || hover === "reverse" || hover === "bipolar") sendParam(hover, 1 - values[hover]);
    else if (hover === "map") {
        mappingArmed = !mappingArmed;
        outlet(0, "map", mappingArmed ? 1 : 0);
    } else if (hover === "unmap") {
        mappingArmed = false;
        outlet(0, "unmap");
    } else if (markerNames.indexOf(hover) !== -1) {
        drag = { name: hover, index: markerNames.indexOf(hover), x: x, position: values[hover], moved: false };
    } else if (hover === "grab") {
        drag = { name: "grab", position: scrubPosition(x) };
        outlet(0, "grab", 1, drag.position);
    } else if (hover === "position" || hover === "rate" || hover === "amount") {
        const param = controlParam(hover);
        drag = { name: hover, param: param, x: x, y: y, normalized: normalized(param) };
    }
    mgraphics.redraw();
}
function updateGesture(x, y, buttonDown, shift) {
    if (!drag) return;
    if (drag.name === "grab") {
        drag.position = scrubPosition(x);
        // Pointer-up supplies the release explicitly, regardless of its buttons mask.
        outlet(0, "grab", buttonDown ? 1 : 0, drag.position);
    } else if (markerNames.indexOf(drag.name) !== -1) {
        if (Math.abs(x - drag.x) >= 3) drag.moved = true;
        if (drag.moved) sendParam(drag.name, clamp(drag.position + (x - drag.x) / graph.w, 0, 1));
        else if (!buttonDown) {
            markeractive(drag.index);
            outlet(0, "seekmarker", drag.index);
        }
    } else {
        const scale = shift ? 0.1 : 1;
        drag.normalized = clamp(drag.normalized + ((drag.y - y) / 110 + (x - drag.x) / 330) * scale, 0, 1);
        drag.x = x;
        drag.y = y;
        sendParam(drag.param, drag.param === "rate" ? RATE_MIN * Math.exp(drag.normalized * RATE_LOG) : drag.param === "division" ? drag.normalized * (divisions.length - 1) : drag.normalized);
    }
    if (!buttonDown) {
        drag = null;
        hover = hit(x, y);
    }
    mgraphics.redraw();
}
function resetControl(x, y) {
    const name = hit(x, y);
    if (name === "position" || name === "rate" || name === "amount") {
        const param = controlParam(name);
        sendParam(param, defaults[param]);
        if (drag && drag.name === name) {
            drag.param = param;
            drag.normalized = normalized(param);
            drag.x = x;
            drag.y = y;
        }
    }
    mgraphics.redraw();
}
function updateHover(x, y) {
    hover = hit(x, y);
    mgraphics.redraw();
}
function clearHover() {
    // Leaving the object is not a release: Max still owns the held gesture.
    hover = "";
    mgraphics.redraw();
}

let pointerGesture = null;
let lastControlClick = null;
function onpointerdown(event) {
    if (!(event.buttons & 1) || pointerGesture) return;
    const x = event.clientX;
    const y = event.clientY;
    const name = hit(x, y);
    const now = Date.now();
    const doubleClick = lastControlClick && lastControlClick.name === name &&
        now - lastControlClick.time <= 400 &&
        Math.abs(x - lastControlClick.x) <= 4 && Math.abs(y - lastControlClick.y) <= 4;
    pointerGesture = { id: event.pointerId, name: name, x: x, y: y, moved: false, doubleClick: doubleClick };
    lastControlClick = null;
    beginGesture(x, y);
    if (doubleClick) resetControl(x, y);
}
function onpointermove(event) {
    if (!pointerGesture) {
        updateHover(event.clientX, event.clientY);
        return;
    }
    if (event.pointerId !== pointerGesture.id) return;
    if (Math.abs(event.clientX - pointerGesture.x) >= 3 || Math.abs(event.clientY - pointerGesture.y) >= 3) pointerGesture.moved = true;
    updateGesture(event.clientX, event.clientY, true, max.shiftkeydown);
}
function onpointerup(event) {
    if (!pointerGesture || event.pointerId !== pointerGesture.id) return;
    const gesture = pointerGesture;
    pointerGesture = null;
    // Max can report buttons == 1 on pointer-up; the event itself owns release.
    updateGesture(event.clientX, event.clientY, false, max.shiftkeydown);
    if (!gesture.moved && !gesture.doubleClick &&
        Math.abs(event.clientX - gesture.x) < 3 && Math.abs(event.clientY - gesture.y) < 3 &&
        (gesture.name === "position" || gesture.name === "rate" || gesture.name === "amount")) {
        lastControlClick = { name: gesture.name, x: event.clientX, y: event.clientY, time: Date.now() };
    }
    updateHover(event.clientX, event.clientY);
}
function onpointerenter(event) { updateHover(event.clientX, event.clientY); }
function onpointerleave(event) { clearHover(); }
function notifydeleted() {
    if (drag && drag.name === "grab") outlet(0, "grab", 0, drag.position);
}

beginGesture.local = 1;
updateGesture.local = 1;
resetControl.local = 1;
updateHover.local = 1;
clearHover.local = 1;
onpointerdown.local = 1;
onpointermove.local = 1;
onpointerup.local = 1;
onpointerenter.local = 1;
onpointerleave.local = 1;
notifydeleted.local = 1;
