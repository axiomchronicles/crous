/**
 * test_python_compat.js - Test compatibility with Python Crous
 * 
 * Tests that Node.js can read files created by Python and vice versa
 */

'use strict';

const crous = require('../index');
const fs = require('path');
const { execSync } = require('child_process');
const assert = require('assert');

console.log('Crous Python-Node.js Compatibility Tests\n');
console.log('='.repeat(60));

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
    }
}

// Check if Python and Crous are available
let pythonAvailable = false;
try {
    execSync('python3 -c "import crous"', { stdio: 'ignore' });
    pythonAvailable = true;
    console.log('✓ Python Crous is available\n');
} catch (e) {
    console.log('⚠ Python Crous not available - skipping cross-compatibility tests\n');
}

if (pythonAvailable) {
    // ============================================================================
    // Python → Node.js Tests
    // ============================================================================
    
    test('read Python-created file - simple object', () => {
        const pythonCode = `
import crous
data = {'name': 'Alice', 'age': 30, 'active': True}
crous.dump(data, '/tmp/test_py_to_node.crous')
`;
        execSync(`python3 -c "${pythonCode}"`, { stdio: 'ignore' });
        
        const result = crous.load('/tmp/test_py_to_node.crous');
        assert.strictEqual(result.name, 'Alice');
        assert.strictEqual(result.age, 30);
        assert.strictEqual(result.active, true);
    });
    
    test('read Python-created file - array', () => {
        const pythonCode = `
import crous
data = [1, 2, 3, 'hello', True, None]
crous.dump(data, '/tmp/test_py_array.crous')
`;
        execSync(`python3 -c "${pythonCode}"`, { stdio: 'ignore' });
        
        const result = crous.load('/tmp/test_py_array.crous');
        assert.strictEqual(result.length, 6);
        assert.strictEqual(result[0], 1);
        assert.strictEqual(result[3], 'hello');
        assert.strictEqual(result[4], true);
        assert.strictEqual(result[5], null);
    });
    
    test('read Python-created file - nested structure', () => {
        const pythonCode = `
import crous
data = {
    'users': [
        {'id': 1, 'name': 'Alice'},
        {'id': 2, 'name': 'Bob'}
    ],
    'metadata': {'version': 1, 'count': 2}
}
crous.dump(data, '/tmp/test_py_nested.crous')
`;
        execSync(`python3 -c "${pythonCode}"`, { stdio: 'ignore' });
        
        const result = crous.load('/tmp/test_py_nested.crous');
        assert.strictEqual(result.users.length, 2);
        assert.strictEqual(result.users[0].name, 'Alice');
        assert.strictEqual(result.metadata.version, 1);
    });
    
    test('read Python-created file - bytes', () => {
        const pythonCode = `
import crous
data = {'data': b'\\x01\\x02\\x03\\x04\\x05'}
crous.dump(data, '/tmp/test_py_bytes.crous')
`;
        execSync(`python3 -c "${pythonCode}"`, { stdio: 'ignore' });
        
        const result = crous.load('/tmp/test_py_bytes.crous');
        assert(Buffer.isBuffer(result.data));
        assert.strictEqual(result.data.length, 5);
        assert.strictEqual(result.data[0], 1);
        assert.strictEqual(result.data[4], 5);
    });
    
    // ============================================================================
    // Node.js → Python Tests
    // ============================================================================
    
    test('Python reads Node-created file - simple object', () => {
        const data = { name: 'Bob', age: 25, active: false };
        crous.dump(data, '/tmp/test_node_to_py.crous');
        
        const pythonCode = `
import crous
data = crous.load('/tmp/test_node_to_py.crous')
assert data['name'] == 'Bob'
assert data['age'] == 25
assert data['active'] == False
print('OK')
`;
        const output = execSync(`python3 -c "${pythonCode}"`).toString().trim();
        assert.strictEqual(output, 'OK');
    });
    
    test('Python reads Node-created file - array', () => {
        const data = [10, 20, 30, 'test', true, null];
        crous.dump(data, '/tmp/test_node_array.crous');
        
        const pythonCode = `
import crous
data = crous.load('/tmp/test_node_array.crous')
assert len(data) == 6
assert data[0] == 10
assert data[3] == 'test'
assert data[4] == True
assert data[5] is None
print('OK')
`;
        const output = execSync(`python3 -c "${pythonCode}"`).toString().trim();
        assert.strictEqual(output, 'OK');
    });
    
    test('Python reads Node-created file - nested', () => {
        const data = {
            products: [
                { id: 1, name: 'Product A', price: 19.99 },
                { id: 2, name: 'Product B', price: 29.99 }
            ],
            total: 2
        };
        crous.dump(data, '/tmp/test_node_nested.crous');
        
        const pythonCode = `
import crous
data = crous.load('/tmp/test_node_nested.crous')
assert len(data['products']) == 2
assert data['products'][0]['name'] == 'Product A'
assert abs(data['products'][0]['price'] - 19.99) < 0.01
assert data['total'] == 2
print('OK')
`;
        const output = execSync(`python3 -c "${pythonCode}"`).toString().trim();
        assert.strictEqual(output, 'OK');
    });
    
    test('Python reads Node-created file - Buffer', () => {
        const data = { data: Buffer.from([10, 20, 30, 40, 50]) };
        crous.dump(data, '/tmp/test_node_buffer.crous');
        
        const pythonCode = `
import crous
data = crous.load('/tmp/test_node_buffer.crous')
assert isinstance(data['data'], bytes)
assert len(data['data']) == 5
assert data['data'][0] == 10
assert data['data'][4] == 50
print('OK')
`;
        const output = execSync(`python3 -c "${pythonCode}"`).toString().trim();
        assert.strictEqual(output, 'OK');
    });
    
    test('roundtrip Node → Python → Node', () => {
        const original = {
            name: 'Roundtrip Test',
            values: [1, 2, 3, 4, 5],
            metadata: { created: 'nodejs', version: 1 }
        };
        
        crous.dump(original, '/tmp/test_roundtrip1.crous');
        
        const pythonCode = `
import crous
data = crous.load('/tmp/test_roundtrip1.crous')
data['metadata']['processed'] = 'python'
crous.dump(data, '/tmp/test_roundtrip2.crous')
`;
        execSync(`python3 -c "${pythonCode}"`, { stdio: 'ignore' });
        
        const result = crous.load('/tmp/test_roundtrip2.crous');
        assert.strictEqual(result.name, 'Roundtrip Test');
        assert.strictEqual(result.values.length, 5);
        assert.strictEqual(result.metadata.created, 'nodejs');
        assert.strictEqual(result.metadata.processed, 'python');
    });
    
    // Cleanup
    try {
        const { unlinkSync } = require('fs');
        unlinkSync('/tmp/test_py_to_node.crous');
        unlinkSync('/tmp/test_py_array.crous');
        unlinkSync('/tmp/test_py_nested.crous');
        unlinkSync('/tmp/test_py_bytes.crous');
        unlinkSync('/tmp/test_node_to_py.crous');
        unlinkSync('/tmp/test_node_array.crous');
        unlinkSync('/tmp/test_node_nested.crous');
        unlinkSync('/tmp/test_node_buffer.crous');
        unlinkSync('/tmp/test_roundtrip1.crous');
        unlinkSync('/tmp/test_roundtrip2.crous');
    } catch (e) {
        // Ignore cleanup errors
    }
} else {
    console.log('To test cross-compatibility, install Python Crous:');
    console.log('  pip install crous\n');
}

// ============================================================================
// Binary Format Compatibility Tests (no Python needed)
// ============================================================================

console.log('\nBinary Format Tests (standalone):');
console.log('-'.repeat(60));

test('identical data produces identical binary', () => {
    const data = { name: 'Test', value: 42 };
    const binary1 = crous.dumps(data);
    const binary2 = crous.dumps(data);
    assert(binary1.equals(binary2));
});

test('binary roundtrip preserves data exactly', () => {
    const original = {
        string: 'hello',
        number: 123,
        float: 45.67,
        bool: true,
        null: null,
        array: [1, 2, 3],
        object: { a: 1, b: 2 }
    };
    
    const binary = crous.dumps(original);
    const result = crous.loads(binary);
    
    assert.deepStrictEqual(result, original);
});

test('binary format is deterministic', () => {
    // Same data should always produce same binary
    const runs = 10;
    const data = { x: 10, y: 20, z: 30 };
    
    const binaries = [];
    for (let i = 0; i < runs; i++) {
        binaries.push(crous.dumps(data));
    }
    
    // All binaries should be identical
    for (let i = 1; i < runs; i++) {
        assert(binaries[0].equals(binaries[i]));
    }
});

// ============================================================================
// Summary
// ============================================================================

console.log('\n' + '='.repeat(60));
console.log(`Compatibility Test Results: ${passCount}/${testCount} passed`);
if (failCount > 0) {
    console.log(`${failCount} test(s) failed`);
    process.exit(1);
} else {
    console.log('All compatibility tests passed! ✓');
    if (pythonAvailable) {
        console.log('\n✓ Python-Node.js cross-compatibility verified');
        console.log('✓ Binary format is fully compatible');
        console.log('✓ Data can be shared seamlessly between platforms');
    }
    process.exit(0);
}
