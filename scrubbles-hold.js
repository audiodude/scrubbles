// A dedicated v8ui parameter lets Live map and automate Hold independently.
autowatch = 1;
inlets = 1;
outlets = 1;
mgraphics.init();
mgraphics.relative_coords = 0;
mgraphics.autofill = 0;

const color = {
    background: [0.075, 0.084, 0.085, 1],
    panel: [0.102, 0.115, 0.115, 1],
    line: [0.21, 0.25, 0.25, 1],
    muted: [0.54, 0.60, 0.59, 1],
    amber: [0.96, 0.69, 0.40, 1],
    amberDim: [0.29, 0.23, 0.16, 1]
};
let value = 0;
let mouseOwned = false;
let hover = false;
let midiHeld = false;

function midiheld(v) {
    midiHeld = Number(v) !== 0;
    mgraphics.redraw();
}

function paint() {
    const size = mgraphics.size;
    mgraphics.set_source_rgba(color.background);
    mgraphics.rectangle(0, 0, size[0], size[1]);
    mgraphics.fill();
    mgraphics.set_source_rgba(value || midiHeld ? color.amberDim : hover ? color.line : color.panel);
    mgraphics.rectangle_rounded(0, 0, size[0], size[1], 4, 4);
    mgraphics.fill();
    mgraphics.select_font_face("Arial");
    mgraphics.set_font_size(10);
    mgraphics.set_source_rgba(value || midiHeld ? color.amber : color.muted);
    mgraphics.move_to((size[0] - mgraphics.text_measure("HOLD")[0]) / 2, size[1] / 2 + 3.5);
    mgraphics.show_text("HOLD");
}

function update(v, emit) {
    const number = Number(v);
    if (!Number.isFinite(number)) return;
    const next = number >= 0.5 ? 1 : 0;
    if (next !== value) {
        value = next;
        // Max reads getvalueof() when notified: assign before notifying.
        notifyclients();
    }
    mgraphics.redraw();
    if (emit) outlet(0, value);
}
update.local = 1;

function msg_int(v) { update(v, true); }
function msg_float(v) { update(v, true); }
function set(v) { update(v, false); }
function getvalueof() { return value; }
function setvalueof(v) { update(v, true); }
function outputvalue() { outlet(0, value); }
function bang() { outputvalue(); }

function inside(x, y) {
    const size = mgraphics.size;
    return x >= 0 && x <= size[0] && y >= 0 && y <= size[1];
}
inside.local = 1;

// Use paired pointer events, not legacy click/double-click/drag callbacks.
// A double-click is still two momentary presses, never a separate action.
function onpointerdown(event) {
    if (!(event.buttons & 1)) return;
    mouseOwned = true;
    hover = true;
    update(1, true);
}
function onpointerup(event) {
    // The up event is authoritative; Max may still report buttons == 1 here.
    if (!mouseOwned) return;
    mouseOwned = false;
    hover = inside(event.clientX, event.clientY);
    update(0, true);
}
function onpointermove(event) {
    hover = inside(event.clientX, event.clientY);
    mgraphics.redraw();
}
function onpointerenter(event) {
    hover = true;
    mgraphics.redraw();
}
function onpointerleave(event) {
    // Leaving the button is not releasing the captured pointer.
    hover = false;
    mgraphics.redraw();
}
function notifydeleted() {
    // Never overwrite restored/automated Hold merely because the UI closes.
    if (mouseOwned) {
        mouseOwned = false;
        update(0, true);
    }
}
onpointerdown.local = 1;
onpointerup.local = 1;
onpointermove.local = 1;
onpointerenter.local = 1;
onpointerleave.local = 1;
notifydeleted.local = 1;
