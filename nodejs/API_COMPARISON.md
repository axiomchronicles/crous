# Crous: Python vs Node.js API Comparison

This document compares the Python and Node.js APIs for Crous, showing how similar operations are performed in each language.

## Installation

### Python
```bash
pip install crous
```

### Node.js
```bash
npm install crous
```

---

## Basic Serialization

### Python
```python
import crous

# Serialize
data = {'name': 'Alice', 'age': 30}
binary = crous.dumps(data)

# Deserialize
result = crous.loads(binary)
```

### Node.js
```javascript
const crous = require('crous');

// Serialize
const data = {name: 'Alice', age: 30};
const binary = crous.dumps(data);

// Deserialize
const result = crous.loads(binary);
```

---

## File I/O

### Python
```python
import crous

# Write to file
data = {'users': [{'id': 1, 'name': 'Alice'}]}
crous.dump(data, 'output.crous')

# Read from file
result = crous.load('output.crous')
```

### Node.js
```javascript
const crous = require('crous');

// Write to file
const data = {users: [{id: 1, name: 'Alice'}]};
crous.dump(data, 'output.crous');

// Read from file
const result = crous.load('output.crous');
```

---

## Custom Serializers

### Python
```python
import crous
from datetime import datetime

class Person:
    def __init__(self, name, age):
        self.name = name
        self.age = age

# Register serializer
def serialize_person(person):
    return {'name': person.name, 'age': person.age}

crous.register_serializer(Person, serialize_person)

# Register decoder
def decode_person(value):
    return Person(value['name'], value['age'])

crous.register_decoder(100, decode_person)

# Use it
person = Person('Alice', 30)
binary = crous.dumps(person)
result = crous.loads(binary)
```

### Node.js
```javascript
const crous = require('crous');

class Person {
    constructor(name, age) {
        this.name = name;
        this.age = age;
    }
}

// Register serializer
crous.registerSerializer(Person, (person) => {
    return {name: person.name, age: person.age};
});

// Register decoder
crous.registerDecoder(100, (value) => {
    return new Person(value.name, value.age);
});

// Use it
const person = new Person('Alice', 30);
const binary = crous.dumps(person);
const result = crous.loads(binary);
```

---

## Object Hook

### Python
```python
import crous

data = {'user': {'name': 'Alice', 'role': 'admin'}}
binary = crous.dumps(data)

# Transform objects during deserialization
result = crous.loads(binary, object_hook=lambda obj: {
    **obj,
    '__loaded': True
})
```

### Node.js
```javascript
const crous = require('crous');

const data = {user: {name: 'Alice', role: 'admin'}};
const binary = crous.dumps(data);

// Transform objects during deserialization
const result = crous.loads(binary, {
    objectHook: (obj) => {
        obj.__loaded = true;
        return obj;
    }
});
```

---

## Default Function

### Python
```python
import crous
from datetime import datetime

data = {
    'date': datetime.now(),
    'value': 42
}

binary = crous.dumps(data, default=lambda obj: (
    obj.isoformat() if isinstance(obj, datetime) else str(obj)
))
```

### Node.js
```javascript
const crous = require('crous');

const data = {
    date: new Date(),
    value: 42
};

const binary = crous.dumps(data, {
    default: (obj) => {
        if (obj instanceof Date) {
            return obj.toISOString();
        }
        return obj.toString();
    }
});
```

---

## Encoder/Decoder Classes

### Python
```python
import crous

# Create encoder with custom settings
encoder = crous.CrousEncoder(
    default=lambda obj: str(obj),
    allow_custom=True
)

binary = encoder.encode(data)

# Create decoder with custom settings
decoder = crous.CrousDecoder(
    object_hook=lambda obj: obj
)

result = decoder.decode(binary)
```

### Node.js
```javascript
const crous = require('crous');

// Create encoder with custom settings
const encoder = new crous.CrousEncoder({
    default: (obj) => obj.toString(),
    allowCustom: true
});

const binary = encoder.encode(data);

// Create decoder with custom settings
const decoder = new crous.CrousDecoder({
    objectHook: (obj) => obj
});

const result = decoder.decode(binary);
```

---

## Error Handling

### Python
```python
import crous

try:
    result = crous.loads(bad_data)
except crous.CrousDecodeError as e:
    print(f"Decode error: {e}")
except crous.CrousEncodeError as e:
    print(f"Encode error: {e}")
except crous.CrousError as e:
    print(f"Crous error: {e}")
```

### Node.js
```javascript
const crous = require('crous');

try {
    const result = crous.loads(badData);
} catch (error) {
    if (error instanceof crous.CrousDecodeError) {
        console.log(`Decode error: ${error.message}`);
    } else if (error instanceof crous.CrousEncodeError) {
        console.log(`Encode error: ${error.message}`);
    } else if (error instanceof crous.CrousError) {
        console.log(`Crous error: ${error.message}`);
    }
}
```

---

## Version Information

### Python
```python
import crous

info = crous.version_info()
print(f"Crous v{info.string}")
print(f"Version: {info.major}.{info.minor}.{info.patch}")
```

### Node.js
```javascript
const crous = require('crous');

const info = crous.versionInfo();
console.log(`Crous v${info.string}`);
console.log(`Version: ${info.major}.${info.minor}.${info.patch}`);
```

---

## TypeScript Support

Node.js has full TypeScript support with type definitions:

```typescript
import { dumps, loads, registerSerializer, registerDecoder } from 'crous';

interface User {
    name: string;
    age: number;
}

const user: User = { name: 'Alice', age: 30 };
const binary: Buffer = dumps(user);
const result: User = loads(binary);

// Custom types
class Point {
    constructor(public x: number, public y: number) {}
}

registerSerializer<Point>(Point, (point) => {
    return { x: point.x, y: point.y };
});

registerDecoder<Point>(100, (value) => {
    return new Point(value.x, value.y);
});
```

Python has type hints with `.pyi` stub files:

```python
from typing import Any, Callable, Optional
import crous

# Type hints are available
data: dict[str, Any] = {'name': 'Alice', 'age': 30}
binary: bytes = crous.dumps(data)
result: Any = crous.loads(binary)
```

---

## Supported Types Comparison

| Type | Python | Node.js | Notes |
|------|--------|---------|-------|
| Null | `None` | `null` | ✓ |
| Boolean | `True`, `False` | `true`, `false` | ✓ |
| Integer | `int` | `number` | ✓ |
| Float | `float` | `number` | ✓ |
| String | `str` | `string` | ✓ |
| Binary | `bytes`, `bytearray` | `Buffer` | ✓ |
| List | `list` | `Array` | ✓ |
| Tuple | `tuple` | `Array` | Python only |
| Dict | `dict` | `Object` | ✓ |
| Set | `set`, `frozenset` | `Set` | ✓ |

---

## Key Differences

### 1. Naming Convention
- **Python**: Snake case (`register_serializer`, `object_hook`)
- **Node.js**: Camel case (`registerSerializer`, `objectHook`)

### 2. Binary Type
- **Python**: `bytes` object
- **Node.js**: `Buffer` object

### 3. Options
- **Python**: Keyword arguments (`loads(data, object_hook=fn)`)
- **Node.js**: Options object (`loads(data, {objectHook: fn})`)

### 4. Tuples
- **Python**: Has native `tuple` type (immutable)
- **Node.js**: Uses `Array` (tuples become arrays)

### 5. Import Style
- **Python**: `import crous` or `from crous import dumps, loads`
- **Node.js**: `const crous = require('crous')` or `import * as crous from 'crous'`

---

## Performance Comparison

Both implementations use the same C core library, so performance is similar:

- **Encoding**: 2-5x faster than JSON
- **Decoding**: 2-4x faster than JSON
- **Size**: 40-60% smaller than JSON

---

## Summary

The Python and Node.js APIs are designed to be as similar as possible, with differences only where necessary to follow language conventions. This makes it easy to:

- Port code between Python and Node.js
- Learn one API if you know the other
- Share data between Python and Node.js applications
- Maintain consistent behavior across platforms

Both implementations provide:
- Same core functionality
- Same serialization format (fully compatible)
- Same custom serializer/decoder system
- Same error handling approach
- Same performance characteristics
