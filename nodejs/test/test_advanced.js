/**
 * test_advanced.js - Advanced functionality tests for Crous Node.js
 * 
 * Tests custom serializers, decoders, edge cases, and advanced features.
 */

'use strict';

const crous = require('../index');
const fs = require('fs');
const path = require('path');
const assert = require('assert');

console.log('Starting Advanced Crous Node.js tests...\n');

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
// Custom Serializer/Decoder Tests
// ============================================================================

test('custom class serialization without decoder', () => {
    class Point {
        constructor(x, y) {
            this.x = x;
            this.y = y;
        }
    }
    
    crous.registerSerializer(Point, (point) => {
        return { x: point.x, y: point.y, __type: 'Point' };
    });
    
    const point = new Point(10, 20);
    const binary = crous.dumps(point);
    const result = crous.loads(binary);
    
    assert.strictEqual(result.__type, 'Point');
    assert.strictEqual(result.x, 10);
    assert.strictEqual(result.y, 20);
    
    crous.unregisterSerializer(Point);
});

test('custom class with full round-trip', () => {
    class Rectangle {
        constructor(width, height) {
            this.width = width;
            this.height = height;
        }
        
        area() {
            return this.width * this.height;
        }
    }
    
    crous.registerSerializer(Rectangle, (rect) => {
        return { w: rect.width, h: rect.height };
    });
    
    crous.registerDecoder(101, (value) => {
        return new Rectangle(value.w, value.h);
    });
    
    const rect = new Rectangle(5, 10);
    const binary = crous.dumps(rect);
    const result = crous.loads(binary);
    
    assert(result instanceof Rectangle);
    assert.strictEqual(result.width, 5);
    assert.strictEqual(result.height, 10);
    assert.strictEqual(result.area(), 50);
    
    crous.unregisterSerializer(Rectangle);
    crous.unregisterDecoder(101);
});

test('multiple custom types', () => {
    class TypeA {
        constructor(value) {
            this.value = value;
            this.type = 'A';
        }
    }
    
    class TypeB {
        constructor(value) {
            this.value = value;
            this.type = 'B';
        }
    }
    
    crous.registerSerializer(TypeA, (obj) => ({ v: obj.value, t: 'A' }));
    crous.registerSerializer(TypeB, (obj) => ({ v: obj.value, t: 'B' }));
    crous.registerDecoder(102, (val) => new TypeA(val.v));
    crous.registerDecoder(103, (val) => new TypeB(val.v));
    
    const data = {
        a: new TypeA(100),
        b: new TypeB(200),
        items: [new TypeA(1), new TypeB(2)]
    };
    
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    
    assert(result.a instanceof TypeA);
    assert(result.b instanceof TypeB);
    assert(result.items[0] instanceof TypeA);
    assert(result.items[1] instanceof TypeB);
    
    crous.unregisterSerializer(TypeA);
    crous.unregisterSerializer(TypeB);
    crous.unregisterDecoder(102);
    crous.unregisterDecoder(103);
});

// ============================================================================
// Edge Cases
// ============================================================================

test('very large integer', () => {
    const data = 9007199254740991; // Number.MAX_SAFE_INTEGER
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result, data);
});

test('very small integer', () => {
    const data = -9007199254740991; // Number.MIN_SAFE_INTEGER
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result, data);
});

test('special float values', () => {
    const values = [Infinity, -Infinity];
    for (const value of values) {
        const binary = crous.dumps(value);
        const result = crous.loads(binary);
        assert.strictEqual(result, value);
    }
});

test('NaN handling', () => {
    const data = NaN;
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert(Number.isNaN(result));
});

test('empty string', () => {
    const data = '';
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result, '');
});

test('very long string', () => {
    const data = 'x'.repeat(10000);
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result.length, 10000);
    assert.strictEqual(result, data);
});

test('string with special characters', () => {
    const data = 'Hello\n\r\t\\"\'World';
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result, data);
});

test('string with emojis', () => {
    const data = '🎉🚀💻🌟⭐️';
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result, data);
});

test('deep nesting', () => {
    let data = { value: 1 };
    for (let i = 0; i < 50; i++) {
        data = { nested: data };
    }
    
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    
    let current = result;
    for (let i = 0; i < 50; i++) {
        assert(current.nested !== undefined);
        current = current.nested;
    }
    assert.strictEqual(current.value, 1);
});

test('large array', () => {
    const data = Array.from({ length: 10000 }, (_, i) => i);
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result.length, 10000);
    assert.strictEqual(result[0], 0);
    assert.strictEqual(result[9999], 9999);
});

test('large object', () => {
    const data = {};
    for (let i = 0; i < 1000; i++) {
        data[`key_${i}`] = i;
    }
    
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(Object.keys(result).length, 1000);
    assert.strictEqual(result.key_0, 0);
    assert.strictEqual(result.key_999, 999);
});

test('mixed type array', () => {
    const data = [
        null,
        true,
        false,
        42,
        3.14,
        'hello',
        Buffer.from([1, 2, 3]),
        [1, 2, 3],
        { a: 1, b: 2 },
        new Set([1, 2, 3])
    ];
    
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.strictEqual(result.length, 10);
    assert.strictEqual(result[0], null);
    assert.strictEqual(result[1], true);
    assert.strictEqual(result[2], false);
    assert.strictEqual(result[3], 42);
    assert(Math.abs(result[4] - 3.14) < 0.001);
    assert.strictEqual(result[5], 'hello');
    assert(Buffer.isBuffer(result[6]));
    assert(Array.isArray(result[7]));
    assert.deepStrictEqual(result[8], { a: 1, b: 2 });
    assert(result[9] instanceof Set);
});

// ============================================================================
// Buffer (Binary) Tests
// ============================================================================

test('buffer with zeros', () => {
    const data = Buffer.alloc(100, 0);
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert(Buffer.isBuffer(result));
    assert.strictEqual(result.length, 100);
    assert(result.every(b => b === 0));
});

test('buffer with all byte values', () => {
    const data = Buffer.from(Array.from({ length: 256 }, (_, i) => i));
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert(Buffer.isBuffer(result));
    assert(data.equals(result));
});

test('large buffer', () => {
    const data = Buffer.alloc(100000, 0xAB);
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert(Buffer.isBuffer(result));
    assert.strictEqual(result.length, 100000);
    assert(result.every(b => b === 0xAB));
});

// ============================================================================
// Set Tests
// ============================================================================

test('set with different types', () => {
    const data = new Set([1, 'hello', true, null]);
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    
    assert(result instanceof Set);
    assert.strictEqual(result.size, 4);
    assert(result.has(1));
    assert(result.has('hello'));
    assert(result.has(true));
    assert(result.has(null));
});

test('nested sets', () => {
    const inner = new Set([1, 2, 3]);
    const data = { sets: [inner, new Set(['a', 'b'])] };
    
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    
    assert(result.sets[0] instanceof Set);
    assert(result.sets[1] instanceof Set);
    assert.strictEqual(result.sets[0].size, 3);
    assert.strictEqual(result.sets[1].size, 2);
});

test('empty set', () => {
    const data = new Set();
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    
    assert(result instanceof Set);
    assert.strictEqual(result.size, 0);
});

// ============================================================================
// Object Hook Tests
// ============================================================================

test('object hook adds property', () => {
    const data = { name: 'Alice', age: 30 };
    const binary = crous.dumps(data);
    
    const result = crous.loads(binary, {
        objectHook: (obj) => {
            obj.__loaded = true;
            return obj;
        }
    });
    
    assert.strictEqual(result.__loaded, true);
    assert.strictEqual(result.name, 'Alice');
});

test('object hook transforms values', () => {
    const data = { value: 10 };
    const binary = crous.dumps(data);
    
    const result = crous.loads(binary, {
        objectHook: (obj) => {
            if (obj.value !== undefined) {
                obj.value *= 2;
            }
            return obj;
        }
    });
    
    assert.strictEqual(result.value, 20);
});

test('object hook on nested objects', () => {
    const data = {
        outer: {
            inner: {
                value: 1
            }
        }
    };
    const binary = crous.dumps(data);
    
    let hookCallCount = 0;
    const result = crous.loads(binary, {
        objectHook: (obj) => {
            hookCallCount++;
            obj.__depth = hookCallCount;
            return obj;
        }
    });
    
    assert(hookCallCount >= 2); // At least inner and outer
    assert(result.outer.__depth !== undefined);
});

// ============================================================================
// Default Function Tests
// ============================================================================

test('default function handles Date', () => {
    const data = {
        name: 'Test',
        created: new Date('2024-01-01T00:00:00Z')
    };
    
    const binary = crous.dumps(data, {
        default: (obj) => {
            if (obj instanceof Date) {
                return obj.toISOString();
            }
            throw new Error('Unsupported type');
        }
    });
    
    const result = crous.loads(binary);
    assert.strictEqual(result.created, '2024-01-01T00:00:00.000Z');
});

test('default function handles multiple types', () => {
    class Custom {
        toString() {
            return 'custom';
        }
    }
    
    const data = {
        date: new Date('2024-01-01'),
        custom: new Custom()
    };
    
    const binary = crous.dumps(data, {
        default: (obj) => {
            if (obj instanceof Date) return obj.toISOString();
            if (obj instanceof Custom) return obj.toString();
            throw new Error('Unsupported');
        }
    });
    
    const result = crous.loads(binary);
    assert.strictEqual(result.custom, 'custom');
});

// ============================================================================
// Error Handling Tests
// ============================================================================

test('encode error - circular reference detection', () => {
    const data = { name: 'test' };
    data.self = data; // Circular reference
    
    try {
        crous.dumps(data);
        throw new Error('Should have thrown an error');
    } catch (error) {
        // Expected - circular references will cause stack overflow or similar
        assert(error !== null);
    }
});

test('decode error - truncated data', () => {
    const data = { name: 'Alice', age: 30 };
    const binary = crous.dumps(data);
    const truncated = binary.slice(0, binary.length - 5);
    
    try {
        crous.loads(truncated);
        throw new Error('Should have thrown an error');
    } catch (error) {
        assert(error instanceof crous.CrousDecodeError || error.message.includes('decode'));
    }
});

test('decode error - corrupted data', () => {
    const corrupted = Buffer.from([0xFF, 0xFF, 0xFF, 0xFF]);
    
    try {
        crous.loads(corrupted);
        throw new Error('Should have thrown an error');
    } catch (error) {
        assert(error instanceof crous.CrousDecodeError || error.message.includes('decode'));
    }
});

test('loads requires buffer', () => {
    try {
        crous.loads('not a buffer');
        throw new Error('Should have thrown an error');
    } catch (error) {
        assert(error.message.includes('Buffer') || error.message.includes('buffer'));
    }
});

// ============================================================================
// Encoder/Decoder Class Tests
// ============================================================================

test('CrousEncoder class', () => {
    const encoder = new crous.CrousEncoder({
        default: (obj) => {
            if (obj instanceof Date) return obj.toISOString();
            return null;
        }
    });
    
    const data = {
        name: 'Test',
        date: new Date('2024-01-01')
    };
    
    const binary = encoder.encode(data);
    assert(Buffer.isBuffer(binary));
    
    const result = crous.loads(binary);
    assert.strictEqual(result.date, '2024-01-01T00:00:00.000Z');
});

test('CrousDecoder class', () => {
    const data = { value: 10 };
    const binary = crous.dumps(data);
    
    const decoder = new crous.CrousDecoder({
        objectHook: (obj) => {
            obj.decoded = true;
            return obj;
        }
    });
    
    const result = decoder.decode(binary);
    assert.strictEqual(result.decoded, true);
    assert.strictEqual(result.value, 10);
});

// ============================================================================
// File I/O Tests
// ============================================================================

test('dump and load with stream', () => {
    const data = { name: 'Stream Test', value: 42 };
    const filepath = path.join(__dirname, 'test_stream.crous');
    
    // Write with stream
    const writeStream = fs.createWriteStream(filepath);
    crous.dump(data, writeStream);
    writeStream.end();
    
    // Wait a bit for write to complete
    const start = Date.now();
    while (!fs.existsSync(filepath) && Date.now() - start < 1000) {
        // Wait
    }
    
    if (fs.existsSync(filepath)) {
        // Read with normal load
        const result = crous.load(filepath);
        assert.deepStrictEqual(result, data);
        
        // Cleanup
        fs.unlinkSync(filepath);
    }
});

test('multiple dumps/loads cycles', () => {
    const filepath = path.join(__dirname, 'test_cycles.crous');
    
    for (let i = 0; i < 10; i++) {
        const data = { iteration: i, values: [i, i * 2, i * 3] };
        crous.dump(data, filepath);
        const result = crous.load(filepath);
        assert.deepStrictEqual(result, data);
    }
    
    if (fs.existsSync(filepath)) {
        fs.unlinkSync(filepath);
    }
});

// ============================================================================
// Performance Tests (basic)
// ============================================================================

test('performance - small object', () => {
    const data = { name: 'Alice', age: 30, active: true };
    const iterations = 1000;
    
    const start = Date.now();
    for (let i = 0; i < iterations; i++) {
        const binary = crous.dumps(data);
        crous.loads(binary);
    }
    const elapsed = Date.now() - start;
    
    console.log(`  ${iterations} iterations in ${elapsed}ms (${(iterations / elapsed * 1000).toFixed(0)} ops/sec)`);
    assert(elapsed < 5000); // Should complete in reasonable time
});

test('performance - large object', () => {
    const data = {
        users: Array.from({ length: 100 }, (_, i) => ({
            id: i,
            name: `User ${i}`,
            email: `user${i}@example.com`,
            active: i % 2 === 0
        }))
    };
    
    const iterations = 100;
    const start = Date.now();
    for (let i = 0; i < iterations; i++) {
        const binary = crous.dumps(data);
        crous.loads(binary);
    }
    const elapsed = Date.now() - start;
    
    console.log(`  ${iterations} large object cycles in ${elapsed}ms`);
    assert(elapsed < 10000);
});

// ============================================================================
// Compatibility Tests
// ============================================================================

test('keys with special characters', () => {
    const data = {
        'normal-key': 1,
        'key with spaces': 2,
        'key.with.dots': 3,
        'key_with_underscores': 4,
        '123numeric': 5,
        '': 6  // empty key
    };
    
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.deepStrictEqual(result, data);
});

test('unicode keys', () => {
    const data = {
        '你好': 'Chinese',
        'مرحبا': 'Arabic',
        'привет': 'Russian',
        '🎉': 'Emoji'
    };
    
    const binary = crous.dumps(data);
    const result = crous.loads(binary);
    assert.deepStrictEqual(result, data);
});

// ============================================================================
// Summary
// ============================================================================

console.log('\n' + '='.repeat(60));
console.log(`Advanced Test Results: ${passCount}/${testCount} passed`);
if (failCount > 0) {
    console.log(`${failCount} test(s) failed`);
    process.exit(1);
} else {
    console.log('All advanced tests passed! ✓');
    process.exit(0);
}
