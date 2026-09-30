#!/usr/bin/env node
// Structural checks follow ../remapperator/validate_maxpat.mjs, extended to
// embedded patchers. Object semantics still require loading the device in Max.
import { readFileSync } from 'node:fs';
import { pathToFileURL } from 'node:url';

export function validatePatcher(patcher, label = 'patcher') {
    if (!patcher || !Array.isArray(patcher.boxes)) throw new Error(`${label}: missing boxes`);
    const boxes = new Map();
    for (const { box } of patcher.boxes) {
        if (!box?.id || boxes.has(box.id)) throw new Error(`${label}: missing or duplicate box ID ${box?.id}`);
        boxes.set(box.id, box);
        if (box.patcher) validatePatcher(box.patcher, `${label}/${box.id}`);
    }
    for (const { patchline: line } of patcher.lines ?? []) {
        for (const [side, count] of [['source', 'numoutlets'], ['destination', 'numinlets']]) {
            const [id, port] = line?.[side] ?? [];
            const box = boxes.get(id);
            if (!box || !Number.isInteger(port) || port < 0 || port >= box[count])
                throw new Error(`${label}: invalid ${side} ${id}[${port}]`);
        }
    }
    return boxes.size;
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
    try {
        const files = process.argv.slice(2);
        if (files.length === 0) throw new Error('usage: node validate_maxpat.mjs <file.maxpat|file.gendsp> ...');
        for (const file of files) {
            const count = validatePatcher(JSON.parse(readFileSync(file, 'utf8')).patcher, file);
            console.log(`PASS ${file}: ${count} boxes; patch graph structurally valid`);
        }
    } catch (error) {
        console.error(error.message);
        process.exitCode = 1;
    }
}
