import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { runInNewContext } from 'node:vm';

const source = readFileSync(new URL('./scrubbles-midi.js', import.meta.url), 'utf8');
function filter(bytes) {
    const output = [];
    const context = { outlet: (_port, byte) => output.push(byte) };
    runInNewContext(source, context);
    for (const byte of bytes) context.msg_int(byte);
    return output;
}

test('reserved controls never sound or send per-note pressure on any channel', () => {
    const input = [];
    const expected = [];
    for (let channel = 0; channel < 16; channel++) {
        for (const kind of [0x90, 0x80, 0xa0]) {
            for (let note = 23; note <= 29; note++) {
                const message = [kind | channel, note, 64];
                input.push(...message);
                if (note === 23 || note === 29) expected.push(...message);
            }
        }
    }
    assert.deepEqual(filter(input), expected);
});

test('running-status musical notes survive intervening filtered note-ons and releases', () => {
    assert.deepEqual(filter([
        0x92, 60, 100, 24, 127, 61, 90, 28, 0, 62, 0,
        0x82, 60, 40, 25, 0, 61, 30
    ]), [0x92, 60, 100, 0x92, 61, 90, 0x92, 62, 0, 0x82, 60, 40, 0x82, 61, 30]);
});

test('dropping a status-changing control note cannot reinterpret downstream data', () => {
    assert.deepEqual(filter([
        0xb0, 1, 80,
        0x90, 24, 100, 64, 100,
        0xa0, 25, 50, 64, 60,
        0x80, 24, 0, 64, 0
    ]), [0xb0, 1, 80, 0x90, 64, 100, 0xa0, 64, 60, 0x80, 64, 0]);
});

test('real-time interruptions do not corrupt filtering or running status', () => {
    assert.deepEqual(filter([
        0x90, 24, 0xf8, 100, 60, 0xfa, 100, 28, 0xfc, 0, 61, 90
    ]), [0xf8, 0xfa, 0x90, 60, 100, 0xfc, 0x90, 61, 90]);
});

test('system messages and other channel events are not mistaken for control notes', () => {
    assert.deepEqual(filter([
        0x90, 24, 100,
        0xf0, 0x7d, 24, 25, 0xf8, 26, 0xf7,
        0xf2, 24, 28, 0xf1, 25, 0xf3, 26, 0xf6,
        0xc4, 24, 28, 0xd4, 25, 26, 0xe4, 24, 28,
        0xb4, 24, 28, 123, 0, 120, 0
    ]), [
        0xf0, 0x7d, 24, 25, 0xf8, 26, 0xf7,
        0xf2, 24, 28, 0xf1, 25, 0xf3, 26, 0xf6,
        0xc4, 24, 0xc4, 28, 0xd4, 25, 0xd4, 26, 0xe4, 24, 28,
        0xb4, 24, 28, 0xb4, 123, 0, 0xb4, 120, 0
    ]);
});
