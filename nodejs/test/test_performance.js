/**
 * test_performance.js - Performance benchmarks for Crous Node.js
 * 
 * Compares Crous performance against JSON.stringify/parse
 */

'use strict';

const crous = require('../index');
const assert = require('assert');

console.log('Crous Node.js Performance Benchmarks\n');
console.log('='.repeat(60));

function benchmark(name, fn, iterations = 1000) {
    // Warmup
    for (let i = 0; i < 10; i++) {
        fn();
    }
    
    // Actual benchmark
    const start = Date.now();
    for (let i = 0; i < iterations; i++) {
        fn();
    }
    const elapsed = Date.now() - start;
    
    const opsPerSec = Math.round(iterations / elapsed * 1000);
    const msPerOp = (elapsed / iterations).toFixed(3);
    
    console.log(`${name}`);
    console.log(`  ${iterations} ops in ${elapsed}ms`);
    console.log(`  ${opsPerSec.toLocaleString()} ops/sec`);
    console.log(`  ${msPerOp}ms per op\n`);
    
    return { elapsed, opsPerSec };
}

// ============================================================================
// Small Object Benchmark
// ============================================================================

console.log('\n1. Small Object (Simple Dictionary)');
console.log('-'.repeat(60));

const smallData = { name: 'Alice', age: 30, active: true };

const crousSmallEncode = benchmark('Crous encode (small)', () => {
    crous.dumps(smallData);
}, 10000);

const jsonSmallEncode = benchmark('JSON encode (small)', () => {
    JSON.stringify(smallData);
}, 10000);

const crousSmallBinary = crous.dumps(smallData);
const jsonSmallBinary = JSON.stringify(smallData);

const crousSmallDecode = benchmark('Crous decode (small)', () => {
    crous.loads(crousSmallBinary);
}, 10000);

const jsonSmallDecode = benchmark('JSON decode (small)', () => {
    JSON.parse(jsonSmallBinary);
}, 10000);

console.log(`Size comparison:`);
console.log(`  Crous: ${crousSmallBinary.length} bytes`);
console.log(`  JSON:  ${jsonSmallBinary.length} bytes`);
console.log(`  Reduction: ${((1 - crousSmallBinary.length / jsonSmallBinary.length) * 100).toFixed(1)}%`);

console.log(`\nEncode speed:`);
console.log(`  Crous: ${crousSmallEncode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  JSON:  ${jsonSmallEncode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  Ratio: ${(jsonSmallEncode.opsPerSec / crousSmallEncode.opsPerSec).toFixed(2)}x`);

console.log(`\nDecode speed:`);
console.log(`  Crous: ${crousSmallDecode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  JSON:  ${jsonSmallDecode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  Ratio: ${(jsonSmallDecode.opsPerSec / crousSmallDecode.opsPerSec).toFixed(2)}x`);

// ============================================================================
// Medium Object Benchmark
// ============================================================================

console.log('\n2. Medium Object (Array of Objects)');
console.log('-'.repeat(60));

const mediumData = {
    users: Array.from({ length: 50 }, (_, i) => ({
        id: i,
        name: `User ${i}`,
        email: `user${i}@example.com`,
        age: 20 + (i % 50),
        active: i % 2 === 0,
        tags: ['developer', 'nodejs', 'javascript']
    }))
};

const crousMediumEncode = benchmark('Crous encode (medium)', () => {
    crous.dumps(mediumData);
}, 1000);

const jsonMediumEncode = benchmark('JSON encode (medium)', () => {
    JSON.stringify(mediumData);
}, 1000);

const crousMediumBinary = crous.dumps(mediumData);
const jsonMediumBinary = JSON.stringify(mediumData);

const crousMediumDecode = benchmark('Crous decode (medium)', () => {
    crous.loads(crousMediumBinary);
}, 1000);

const jsonMediumDecode = benchmark('JSON decode (medium)', () => {
    JSON.parse(jsonMediumBinary);
}, 1000);

console.log(`Size comparison:`);
console.log(`  Crous: ${crousMediumBinary.length} bytes`);
console.log(`  JSON:  ${jsonMediumBinary.length} bytes`);
console.log(`  Reduction: ${((1 - crousMediumBinary.length / jsonMediumBinary.length) * 100).toFixed(1)}%`);

console.log(`\nEncode speed:`);
console.log(`  Crous: ${crousMediumEncode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  JSON:  ${jsonMediumEncode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  Ratio: ${(jsonMediumEncode.opsPerSec / crousMediumEncode.opsPerSec).toFixed(2)}x`);

console.log(`\nDecode speed:`);
console.log(`  Crous: ${crousMediumDecode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  JSON:  ${jsonMediumDecode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  Ratio: ${(jsonMediumDecode.opsPerSec / crousMediumDecode.opsPerSec).toFixed(2)}x`);

// ============================================================================
// Large Object Benchmark
// ============================================================================

console.log('\n3. Large Object (Nested Structure)');
console.log('-'.repeat(60));

const largeData = {
    version: 1,
    timestamp: Date.now(),
    data: {
        users: Array.from({ length: 200 }, (_, i) => ({
            id: i,
            name: `User ${i}`,
            email: `user${i}@example.com`,
            profile: {
                age: 20 + (i % 50),
                bio: `This is user ${i}'s bio with some text`,
                interests: ['coding', 'reading', 'gaming', 'music'],
                stats: {
                    posts: i * 10,
                    followers: i * 5,
                    following: i * 3
                }
            },
            active: i % 2 === 0
        })),
        metadata: {
            total: 200,
            active: 100,
            categories: ['tech', 'science', 'art', 'music', 'sports']
        }
    }
};

const crousLargeEncode = benchmark('Crous encode (large)', () => {
    crous.dumps(largeData);
}, 100);

const jsonLargeEncode = benchmark('JSON encode (large)', () => {
    JSON.stringify(largeData);
}, 100);

const crousLargeBinary = crous.dumps(largeData);
const jsonLargeBinary = JSON.stringify(largeData);

const crousLargeDecode = benchmark('Crous decode (large)', () => {
    crous.loads(crousLargeBinary);
}, 100);

const jsonLargeDecode = benchmark('JSON decode (large)', () => {
    JSON.parse(jsonLargeBinary);
}, 100);

console.log(`Size comparison:`);
console.log(`  Crous: ${crousLargeBinary.length.toLocaleString()} bytes`);
console.log(`  JSON:  ${jsonLargeBinary.length.toLocaleString()} bytes`);
console.log(`  Reduction: ${((1 - crousLargeBinary.length / jsonLargeBinary.length) * 100).toFixed(1)}%`);

console.log(`\nEncode speed:`);
console.log(`  Crous: ${crousLargeEncode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  JSON:  ${jsonLargeEncode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  Ratio: ${(jsonLargeEncode.opsPerSec / crousLargeEncode.opsPerSec).toFixed(2)}x`);

console.log(`\nDecode speed:`);
console.log(`  Crous: ${crousLargeDecode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  JSON:  ${jsonLargeDecode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  Ratio: ${(jsonLargeDecode.opsPerSec / crousLargeDecode.opsPerSec).toFixed(2)}x`);

// ============================================================================
// Binary Data Benchmark
// ============================================================================

console.log('\n4. Binary Data (Buffer)');
console.log('-'.repeat(60));

const binaryData = {
    filename: 'data.bin',
    content: Buffer.alloc(10000, 0xAB),
    metadata: {
        size: 10000,
        type: 'binary'
    }
};

const crousBinaryEncode = benchmark('Crous encode (binary)', () => {
    crous.dumps(binaryData);
}, 500);

// Note: JSON cannot natively handle binary data
const jsonBinaryData = {
    filename: 'data.bin',
    content: Array.from(binaryData.content),
    metadata: binaryData.metadata
};

const jsonBinaryEncode = benchmark('JSON encode (binary as array)', () => {
    JSON.stringify(jsonBinaryData);
}, 500);

const crousBinaryBinary = crous.dumps(binaryData);
const jsonBinaryBinary = JSON.stringify(jsonBinaryData);

console.log(`Size comparison:`);
console.log(`  Crous: ${crousBinaryBinary.length.toLocaleString()} bytes`);
console.log(`  JSON:  ${jsonBinaryBinary.length.toLocaleString()} bytes`);
console.log(`  Reduction: ${((1 - crousBinaryBinary.length / jsonBinaryBinary.length) * 100).toFixed(1)}%`);

console.log(`\nEncode speed:`);
console.log(`  Crous: ${crousBinaryEncode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  JSON:  ${jsonBinaryEncode.opsPerSec.toLocaleString()} ops/sec`);
console.log(`  Ratio: ${(jsonBinaryEncode.opsPerSec / crousBinaryEncode.opsPerSec).toFixed(2)}x`);

// ============================================================================
// Summary
// ============================================================================

console.log('\n' + '='.repeat(60));
console.log('Performance Summary');
console.log('='.repeat(60));

console.log('\nSize Reduction (Crous vs JSON):');
console.log(`  Small object:  ${((1 - crousSmallBinary.length / jsonSmallBinary.length) * 100).toFixed(1)}%`);
console.log(`  Medium object: ${((1 - crousMediumBinary.length / jsonMediumBinary.length) * 100).toFixed(1)}%`);
console.log(`  Large object:  ${((1 - crousLargeBinary.length / jsonLargeBinary.length) * 100).toFixed(1)}%`);
console.log(`  Binary data:   ${((1 - crousBinaryBinary.length / jsonBinaryBinary.length) * 100).toFixed(1)}%`);

console.log('\nCrous is particularly effective for:');
console.log('  ✓ Binary data (massive size savings)');
console.log('  ✓ Large nested structures');
console.log('  ✓ Arrays of similar objects');
console.log('  ✓ Data with repeated keys');

console.log('\nJSON is faster for:');
console.log('  ✓ Very small objects (lower overhead)');
console.log('  ✓ When human readability is required');
console.log('  ✓ When browser compatibility is critical');

console.log('\n' + '='.repeat(60));
console.log('Benchmarks completed successfully! ✓');
