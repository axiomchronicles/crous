/**
 * test_basic.js - Core functionality tests for Crous Node.js
 * 
 * Tests basic dumps/loads/dump/load operations with fundamental types.
 */

'use strict';

const crous = require('../index');
const fs = require('fs');
const path = require('path');
const assert = require('assert');

console.log('Starting Crous Node.js tests...\n');

// Test counter
let testCount = 0;
let passCount = 0;
let failCount = 0;

function test(name, fn) {
    testCount++;
    try {
        fn();
        passCount++;
        console.log(`✓ ${name}`);
    } catch (error) {
        failCount++;
        console.error(`✗ ${name}`);
        console.error(`  Error: ${error.message}`);
        if (error.stack) {
            console.error(`  ${error.stack.split('\n').slice(1, 3).join('\n  ')}`);
        }
    }
}

// ============================================================================
// Basic Type Tests
// ============================================================================

test('null roundtrip', () => {
    const data = null;
    const binary = crous.dumps(data);
    assert(Buffer.isBuffer(binary), 'dumps should return a Buffer');
    const result = crous.loads(binary);
    assert.strictEqual(result, null, 'Result should be null');
});

test('boolean true roundtrip', () => {
    const data = true;
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result, true, 'Result should be true');
});

test('boolean false roundtrip', () => {
    const data = false;
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result, false, 'Result should be false');
});

test('integer zero', () => {
    const data = 0;
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result, 0, 'Result should be 0');
});

test('positive integers', () => {
    const values = [1, 42, 255, 65535, 1000000];
    for (const value of values) {
        const binary = crous.dumps(value);
        const result = crous.loads(binary);
        assert.strictEqual(result, value, `Result should be ${value}`);
    }
});

test('negative integers', () => {
    const values = [-1, -42, -255, -1000];
    for (const value of values) {
        const binary = crous.dumps(value);
        const result = crous.loads(binary);
        assert.strictEqual(result, value, `Result should be ${value}`);
    }
});

test('float zero', () => {
    const data = 0.0;
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result, 0.0, 'Result should be 0.0');
});

test('positive floats', () => {
    const values = [3.14159, 1.0, 1e10, 1e-10];
    for (const value of values) {
        const binary = crous.dumps(value);
        const result = crous.loads(binary);
        assert(Math.abs(result - value) < 1e-9, `Result should be approximately ${value}`);
    }
});

test('negative floats', () => {
    const values = [-3.14159, -1.0, -1e10];
    for (const value of values) {
        const binary = crous.dumps(value);
        const result = crous.loads(binary);
        assert(Math.abs(result - value) < 1e-9, `Result should be approximately ${value}`);
    }
});

test('string empty', () => {
    const data = '';
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result, '', 'Result should be empty string');
});

test('string simple', () => {
    const data = 'hello';
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result, 'hello', 'Result should be "hello"');
});

test('string with spaces', () => {
    const data = 'hello world';
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result, 'hello world', 'Result should be "hello world"');
});

test('string unicode', () => {
    const data = '你好世界 🌍';
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result, data, 'Result should match unicode string');
});

test('buffer (bytes) empty', () => {
    const data = Buffer.from([]);
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert(Buffer.isBuffer(result), 'Result should be a Buffer');
    assert.strictEqual(result.length, 0, 'Result should be empty');
});

test('buffer (bytes) simple', () => {
    const data = Buffer.from([1, 2, 3, 4, 5]);
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert(Buffer.isBuffer(result), 'Result should be a Buffer');
    assert(data.equals(result), 'Result should equal original buffer');
});

// ============================================================================
// Collection Type Tests
// ============================================================================

test('array empty', () => {
    const data = [];
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert(Array.isArray(result), 'Result should be an array');
    assert.strictEqual(result.length, 0, 'Result should be empty');
});

test('array simple', () => {
    const data = [1, 2, 3, 4, 5];
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.deepStrictEqual(result, data, 'Result should match array');
});

test('array mixed types', () => {
    const data = [1, 'hello', true, null, 3.14];
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.deepStrictEqual(result, data, 'Result should match mixed array');
});

test('array nested', () => {
    const data = [1, [2, 3], [4, [5, 6]]];
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.deepStrictEqual(result, data, 'Result should match nested array');
});

test('object empty', () => {
    const data = {};
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.deepStrictEqual(result, data, 'Result should be empty object');
});

test('object simple', () => {
    const data = { name: 'Alice', age: 30 };
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.deepStrictEqual(result, data, 'Result should match object');
});

test('object mixed types', () => {
    const data = {
        name: 'Bob',
        age: 25,
        active: true,
        score: 98.5,
        tags: ['developer', 'nodejs'],
        metadata: null
    };
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.deepStrictEqual(result, data, 'Result should match mixed object');
});

test('object nested', () => {
    const data = {
        user: {
            name: 'Charlie',
            address: {
                city: 'NYC',
                zip: '10001'
            }
        }
    };
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.deepStrictEqual(result, data, 'Result should match nested object');
});

// ============================================================================
// File I/O Tests
// ============================================================================

test('dump and load file', () => {
    const data = { name: 'Test', value: 42 };
    const filepath = path.join(__dirname, 'test_output.crous');
    
    // Write
    crous.dump(data, filepath);
    assert(fs.existsSync(filepath), 'File should exist');
    
    // Read
    const result = crous.load(filepath);
    assert.deepStrictEqual(result, data, 'Result should match original data');
    
    // Cleanup
    fs.unlinkSync(filepath);
});

test('complex data structure', () => {
    const data = {
        users: [
            { id: 1, name: 'Alice', active: true },
            { id: 2, name: 'Bob', active: false },
            { id: 3, name: 'Charlie', active: true }
        ],
        metadata: {
            version: 1,
            created: '2024-01-01',
            stats: {
                total: 3,
                active: 2
            }
        },
        tags: ['test', 'demo', 'example']
    };
    
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.deepStrictEqual(result, data, 'Result should match complex data');
});

// ============================================================================
// Set Support Tests
// ============================================================================

test('Set serialization', () => {
    const data = new Set([1, 2, 3, 4, 5]);
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    
    // Result should be a Set with same elements
    assert(result instanceof Set, 'Result should be a Set');
    assert.strictEqual(result.size, data.size, 'Set sizes should match');
    for (const item of data) {
        assert(result.has(item), `Set should contain ${item}`);
    }
});

// ============================================================================
// Error Handling Tests
// ============================================================================

test('loads with invalid data', () => {
    try {
        const badData = Buffer.from([1, 2, 3]);
        crous.loads(badData);
        throw new Error('Should have thrown an error');
    } catch (error) {
        assert(error instanceof crous.CrousDecodeError || error.message.includes('decode'),
               'Should throw decode error');
    }
});

// ============================================================================
// Version Info Test
// ============================================================================

test('version info', () => {
    const info = crous.versionInfo();
    assert(info.major !== undefined, 'Should have major version');
    assert(info.minor !== undefined, 'Should have minor version');
    assert(info.patch !== undefined, 'Should have patch version');
    assert(typeof info.string === 'string', 'Should have version string');
    console.log(`  Crous version: ${info.string}`);
});

// ============================================================================
// Summary
// ============================================================================

console.log('\n' + '='.repeat(60));
console.log(`Test Results: ${passCount}/${testCount} passed`);
if (failCount > 0) {
    console.log(`${failCount} test(s) failed`);
    process.exit(1);
} else {
    console.log('All tests passed! ✓');
    process.exit(0);
}
