// Reserve Live C0..E0 (MIDI 24..28) for Freeze and four seek markers.
// Channel messages are emitted with explicit status so removing a control note
// cannot leave downstream running status pointing at a different message type.
autowatch = 1;
inlets = 1;
outlets = 1;

var channelStatus = 0;
var firstData = -1;

function msg_int(byte) {
    if (byte >= 0xf8) {
        // Real-time bytes may interrupt any message, including SysEx.
        outlet(0, byte);
        return;
    }
    if (byte >= 0x80) {
        firstData = -1;
        channelStatus = byte < 0xf0 ? byte : 0;
        if (!channelStatus) outlet(0, byte);
        return;
    }
    if (!channelStatus) {
        // System-common/SysEx bytes pass through; neither uses running status.
        outlet(0, byte);
        return;
    }
    var kind = channelStatus & 0xf0;
    if (kind === 0xc0 || kind === 0xd0) {
        outlet(0, channelStatus);
        outlet(0, byte);
    } else if (firstData < 0) {
        firstData = byte;
    } else {
        var controlNote = firstData >= 24 && firstData <= 28 &&
            (kind === 0x80 || kind === 0x90 || kind === 0xa0);
        if (!controlNote) {
            outlet(0, channelStatus);
            outlet(0, firstData);
            outlet(0, byte);
        }
        firstData = -1;
    }
}
