#!/usr/bin/env node
import { readFileSync, writeFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { validatePatcher } from './validate_maxpat.mjs';

const root = dirname(fileURLToPath(import.meta.url));
for (const file of ['scrubbles.maxpat', 'scrubbles-engine.gendsp'])
    validatePatcher(JSON.parse(readFileSync(join(root, file), 'utf8')).patcher, file);
for (const file of ['scrubbles-control.js', 'scrubbles-ui.js', 'scrubbles-hold.js'])
    readFileSync(join(root, file), 'utf8');

// Remapperator's editable AMPF shell plus same-folder abstractions. MIDI device
// type describes host I/O; its internal gen~ modulation still runs at signal rate.
const patcher = {
    fileversion: 1,
    appversion: { major: 9, minor: 1, revision: 5, architecture: 'x64', modernui: 1 },
    classnamespace: 'box',
    rect: [100, 100, 800, 430],
    openrect: [0, 0, 760, 169],
    openinpresentation: 1,
    devicewidth: 760,
    default_fontsize: 12,
    default_fontname: 'Arial',
    bgcolor: [0.075, 0.09, 0.095, 1],
    boxes: [
        { box: { id: 'midi-in', maxclass: 'newobj', text: 'midiin', numinlets: 1, numoutlets: 1, outlettype: ['int'], patching_rect: [20, 220, 60, 22] } },
        { box: { id: 'midi-out', maxclass: 'newobj', text: 'midiout', numinlets: 1, numoutlets: 0, patching_rect: [20, 280, 60, 22] } },
        { box: { id: 'device', maxclass: 'bpatcher', name: 'scrubbles.maxpat', varname: 'scrubbles', numinlets: 2, numoutlets: 1, outlettype: [''], patching_rect: [20, 20, 760, 169], presentation: 1, presentation_rect: [0, 0, 760, 169], offset: [0, 0], embed: 0, viewvisibility: 1, border: 0 } },
        { box: { id: 'description', maxclass: 'comment', text: 'Scrubbles: perform a modulation contour. MIDI passes through unchanged; Hold Note pauses automatic travel while held. No companion device needed. Keep unfrozen while editing.', numinlets: 1, numoutlets: 0, patching_rect: [150, 215, 550, 60] } }
    ],
    lines: [
        { patchline: { source: ['midi-in', 0], destination: ['midi-out', 0], order: 0 } },
        { patchline: { source: ['midi-in', 0], destination: ['device', 1], order: 1 } }
    ],
    parameters: {
        'device::seed': ['Seed', 'Seed', 0],
        'device::position': ['Position', 'Position', 0],
        'device::rate': ['Rate', 'Rate', 0],
        'device::amount': ['Amount', 'Amount', 0],
        'device::running': ['Run', 'Run', 0],
        'device::reverse': ['Reverse', 'Reverse', 0],
        'device::bipolar': ['Bipolar', 'Bipolar', 0],
        'device::hold': ['Hold', 'Hold', 0],
        'device::holdnote': ['Hold Note', 'Hold Note', 0],
        'device::sync': ['Sync', 'Sync', 0],
        'device::division': ['Division', 'Division', 0],
        parameterbanks: {
            '0': { index: 0, name: 'Scrubbles', parameters: ['Position', 'Rate', 'Amount', 'Seed', 'Run', 'Reverse', 'Bipolar', 'Hold'] },
            '1': { index: 1, name: 'Timing / Notes', parameters: ['Sync', 'Division', 'Hold Note', '-', '-', '-', '-', '-'] }
        },
        inherited_shortname: 1
    },
    latency: 0,
    minimum_live_version: '12.2',
    minimum_max_version: '9.0.0',
    project: {
        version: 1, autoorganize: 1, hideprojectwindow: 1, showdependencies: 1,
        autolocalize: 0, contents: { patchers: {} }, layout: {}, searchpath: {},
        amxdtype: 1835887981, readonly: 0, devpathtype: 0, devpath: '.', includepackages: 0
    },
    dependency_cache: [
        { name: 'scrubbles.maxpat', bootpath: '.', patcherrelativepath: '.', type: 'JSON', implicit: 1 },
        { name: 'scrubbles-engine.gendsp', bootpath: '.', patcherrelativepath: '.', type: 'gDSP', implicit: 1 },
        { name: 'scrubbles-control.js', bootpath: '.', patcherrelativepath: '.', type: 'TEXT', implicit: 1 },
        { name: 'scrubbles-hold.js', bootpath: '.', patcherrelativepath: '.', type: 'TEXT', implicit: 1 },
        { name: 'scrubbles-ui.js', bootpath: '.', patcherrelativepath: '.', type: 'TEXT', implicit: 1 }
    ],
    autosave: 0
};
validatePatcher(patcher, 'device shell');
const payload = Buffer.from(JSON.stringify({ patcher }, null, 2) + '\n\0');
const header = Buffer.alloc(32);
header.write('ampf', 0); header.writeUInt32LE(4, 4); header.write('mmmm', 8);
header.write('meta', 12); header.writeUInt32LE(4, 16); header.writeUInt32LE(1, 20);
header.write('ptch', 24); header.writeUInt32LE(payload.length, 28);
writeFileSync(join(root, 'scrubbles.amxd'), Buffer.concat([header, payload]));
console.log('Built editable scrubbles.amxd (MIDI Effect). Keep adjacent source files; device remains unfrozen.');
