/**
 * Example usage of Crous for Node.js
 */

const crous = require('./index');

console.log('Crous Node.js Examples\n');
console.log('='.repeat(60));

// Example 1: Basic serialization
console.log('\n1. Basic Serialization');
console.log('-'.repeat(60));
const data1 = {
    name: 'Alice',
    age: 30,
    email: 'alice@example.com',
    tags: ['developer', 'nodejs', 'javascript']
};
console.log('Original:', JSON.stringify(data1));
const binary1 = crous.dumps(data1);
console.log('Binary size:', binary1.length, 'bytes');
console.log('JSON size:', JSON.stringify(data1).length, 'bytes');
console.log('Compression:', ((1 - binary1.length / JSON.stringify(data1).length) * 100).toFixed(1) + '%');
const result1 = crous.loads(binary1);
console.log('Deserialized:', JSON.stringify(result1));
console.log('Match:', JSON.stringify(data1) === JSON.stringify(result1) ? '✓' : '✗');

// Example 2: Complex nested structure
console.log('\n2. Complex Nested Structure');
console.log('-'.repeat(60));
const data2 = {
    company: 'TechCorp',
    employees: [
        {
            id: 1,
            name: 'Alice',
            department: 'Engineering',
            skills: ['JavaScript', 'Node.js', 'Python'],
            active: true
        },
        {
            id: 2,
            name: 'Bob',
            department: 'Design',
            skills: ['UI/UX', 'Figma', 'Sketch'],
            active: true
        }
    ],
    metadata: {
        version: 1,
        created: '2024-01-01',
        stats: {
            total_employees: 2,
            departments: 2
        }
    }
};
const binary2 = crous.dumps(data2);
console.log('Complex structure serialized');
console.log('Binary size:', binary2.length, 'bytes');
console.log('JSON size:', JSON.stringify(data2).length, 'bytes');
const result2 = crous.loads(binary2);
console.log('Match:', JSON.stringify(data2) === JSON.stringify(result2) ? '✓' : '✗');

// Example 3: Binary data (Buffer)
console.log('\n3. Binary Data (Buffer)');
console.log('-'.repeat(60));
const binaryData = Buffer.from([0x48, 0x65, 0x6C, 0x6C, 0x6F]); // "Hello" in hex
const data3 = {
    filename: 'data.bin',
    content: binaryData
};
console.log('Original buffer:', binaryData);
const binary3 = crous.dumps(data3);
const result3 = crous.loads(binary3);
console.log('Deserialized buffer:', result3.content);
console.log('Match:', binaryData.equals(result3.content) ? '✓' : '✗');

// Example 4: Set support
console.log('\n4. Set Support');
console.log('-'.repeat(60));
const data4 = {
    unique_ids: new Set([1, 2, 3, 4, 5]),
    tags: new Set(['important', 'urgent', 'priority'])
};
console.log('Original Set:', data4.unique_ids);
const binary4 = crous.dumps(data4);
const result4 = crous.loads(binary4);
console.log('Deserialized Set:', result4.unique_ids);
console.log('Is Set:', result4.unique_ids instanceof Set ? '✓' : '✗');
console.log('Size match:', result4.unique_ids.size === data4.unique_ids.size ? '✓' : '✗');

// Example 5: Custom serializer
console.log('\n5. Custom Serializer');
console.log('-'.repeat(60));

class Person {
    constructor(name, age) {
        this.name = name;
        this.age = age;
    }
    
    toString() {
        return `Person(${this.name}, ${this.age})`;
    }
}

// Register serializer
crous.registerSerializer(Person, (person) => {
    return {
        name: person.name,
        age: person.age,
        __type__: 'Person'
    };
});

// Register decoder
crous.registerDecoder(100, (value) => {
    if (value.__type__ === 'Person') {
        return new Person(value.name, value.age);
    }
    return value;
});

const person = new Person('Charlie', 35);
console.log('Original:', person.toString());
const binaryPerson = crous.dumps(person);
const resultPerson = crous.loads(binaryPerson);
console.log('Deserialized:', resultPerson.toString());
console.log('Is Person:', resultPerson instanceof Person ? '✓' : '✗');

// Example 6: File I/O
console.log('\n6. File I/O');
console.log('-'.repeat(60));
const fs = require('fs');
const path = require('path');

const data6 = {
    title: 'File I/O Example',
    data: [1, 2, 3, 4, 5],
    metadata: {
        created: new Date().toISOString(),
        version: '1.0'
    }
};

const filepath = path.join(__dirname, 'example_output.crous');

try {
    // Write to file
    crous.dump(data6, filepath);
    console.log('Written to:', filepath);
    
    // Read from file
    const result6 = crous.load(filepath);
    console.log('Read from file successfully');
    console.log('Match:', JSON.stringify(data6) === JSON.stringify(result6) ? '✓' : '✗');
    
    // Cleanup
    fs.unlinkSync(filepath);
    console.log('Cleaned up test file');
} catch (error) {
    console.error('Error:', error.message);
}

// Example 7: Version info
console.log('\n7. Version Information');
console.log('-'.repeat(60));
const version = crous.versionInfo();
console.log('Version:', version.string);
console.log('Major:', version.major);
console.log('Minor:', version.minor);
console.log('Patch:', version.patch);

console.log('\n' + '='.repeat(60));
console.log('All examples completed successfully!');
