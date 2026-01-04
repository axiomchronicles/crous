# Crous for Node.js

High-performance binary serialization format for Node.js, based on the Crous C library.

## Features

- **Fast**: Native C implementation with N-API bindings
- **Efficient**: 40-60% smaller than JSON while remaining deterministic
- **Type-safe**: Full TypeScript support with comprehensive type definitions
- **Flexible**: Support for custom serializers and decoders
- **Compatible**: Works with Node.js 14+ on all platforms

## Supported Types

### Built-in Types
- **Primitives**: `null`, `boolean`, `number` (int/float), `string`, `Buffer`
- **Collections**: `Array`, `Object`, `Set`
- **Custom Types**: Via tagged values with custom serializers/decoders

## Installation

```bash
npm install crous
```

Or from source:

```bash
git clone https://github.com/axiomchronicles/crous.git
cd crous/nodejs
npm install
npm run build
```

## Quick Start

### Basic Usage

```javascript
const crous = require('crous');

// Serialize data
const data = { name: 'Alice', age: 30, tags: ['developer', 'nodejs'] };
const binary = crous.dumps(data);
console.log(binary); // <Buffer ...>

// Deserialize data
const result = crous.loads(binary);
console.log(result); // { name: 'Alice', age: 30, tags: ['developer', 'nodejs'] }
```

### TypeScript Usage

```typescript
import * as crous from 'crous';

interface User {
    name: string;
    age: number;
    tags: string[];
}

const user: User = { name: 'Alice', age: 30, tags: ['developer', 'nodejs'] };
const binary: Buffer = crous.dumps(user);
const result: User = crous.loads(binary);
```

### File I/O

```javascript
const crous = require('crous');
const fs = require('fs');

// Write to file
const data = { users: [{ id: 1, name: 'Alice' }] };
crous.dump(data, 'output.crous');

// Read from file
const loaded = crous.load('output.crous');
console.log(loaded); // { users: [{ id: 1, name: 'Alice' }] }

// Or use streams
const writeStream = fs.createWriteStream('output.crous');
crous.dump(data, writeStream);

const readStream = fs.createReadStream('output.crous');
const result = crous.load(readStream);
```

## Advanced Features

### Custom Serializers

Register custom serializers for your own classes:

```javascript
const crous = require('crous');

class Point {
    constructor(x, y) {
        this.x = x;
        this.y = y;
    }
}

// Register serializer
crous.registerSerializer(Point, (point) => {
    return { x: point.x, y: point.y };
});

// Now you can serialize Point instances
const point = new Point(10, 20);
const binary = crous.dumps(point);

// To deserialize back to Point, register a decoder
crous.registerDecoder(100, (value) => {
    return new Point(value.x, value.y);
});

const result = crous.loads(binary);
console.log(result instanceof Point); // true
```

### Custom Decoders

```javascript
const crous = require('crous');

class DateTime {
    constructor(timestamp) {
        this.date = new Date(timestamp);
    }
}

// Register decoder for tag 101
crous.registerDecoder(101, (value) => {
    return new DateTime(value);
});

// Now when loading data with tag 101, it will be decoded to DateTime
```

### Object Hooks

Transform dictionaries during deserialization:

```javascript
const crous = require('crous');

const data = { user: { name: 'Alice', role: 'admin' } };
const binary = crous.dumps(data);

// Transform all objects during deserialization
const result = crous.loads(binary, {
    objectHook: (obj) => {
        // Add metadata to all objects
        obj.__loaded = true;
        return obj;
    }
});

console.log(result.user.__loaded); // true
```

### Default Function

Handle unsupported types during serialization:

```javascript
const crous = require('crous');

const data = {
    date: new Date(),
    special: new CustomType()
};

const binary = crous.dumps(data, {
    default: (obj) => {
        if (obj instanceof Date) {
            return obj.toISOString();
        }
        if (obj instanceof CustomType) {
            return obj.toString();
        }
        throw new Error(`Cannot serialize ${obj}`);
    }
});
```

## API Reference

### Core Functions

#### `dumps(obj, options?)`
Serialize a JavaScript value to binary format.

**Parameters:**
- `obj` (any): The object to serialize
- `options` (object, optional):
  - `default` (function): Handler for unsupported types
  - `allowCustom` (boolean): Whether to allow custom serializers (default: true)

**Returns:** `Buffer` - Binary encoded data

**Throws:** `CrousEncodeError` if encoding fails

---

#### `loads(data, options?)`
Deserialize binary data to a JavaScript value.

**Parameters:**
- `data` (Buffer): Binary data to deserialize
- `options` (object, optional):
  - `objectHook` (function): Post-process dictionaries during deserialization

**Returns:** Deserialized JavaScript value

**Throws:** `CrousDecodeError` if decoding fails

---

#### `dump(obj, filepath, options?)`
Serialize a JavaScript value and write to a file.

**Parameters:**
- `obj` (any): The object to serialize
- `filepath` (string | WritableStream): Path to output file or writable stream
- `options` (object, optional): Same as `dumps()`

**Throws:** `CrousEncodeError` if encoding fails

---

#### `load(filepath, options?)`
Deserialize a JavaScript value from a file.

**Parameters:**
- `filepath` (string | ReadableStream): Path to input file or readable stream
- `options` (object, optional): Same as `loads()`

**Returns:** Deserialized JavaScript value

**Throws:** `CrousDecodeError` if decoding fails

---

### Custom Serializers/Decoders

#### `registerSerializer(type, serializer)`
Register a custom serializer for a specific type.

**Parameters:**
- `type` (Function): The constructor/class to register a serializer for
- `serializer` (Function): Function to convert instances to serializable values

---

#### `unregisterSerializer(type)`
Unregister a custom serializer for a specific type.

---

#### `registerDecoder(tag, decoder)`
Register a custom decoder for a specific tag.

**Parameters:**
- `tag` (number): The tag identifier (integer)
- `decoder` (Function): Function to convert tagged values to objects

---

#### `unregisterDecoder(tag)`
Unregister a custom decoder for a specific tag.

---

### Classes

#### `CrousEncoder`
Encoder class for custom serialization control.

```javascript
const encoder = new crous.CrousEncoder({
    default: (obj) => obj.toString(),
    allowCustom: true
});

const binary = encoder.encode(data);
```

---

#### `CrousDecoder`
Decoder class for custom deserialization control.

```javascript
const decoder = new crous.CrousDecoder({
    objectHook: (obj) => {
        obj.__decoded = true;
        return obj;
    }
});

const data = decoder.decode(binary);
```

---

### Error Classes

- `CrousError`: Base error class for all Crous errors
- `CrousEncodeError`: Thrown during encoding/serialization
- `CrousDecodeError`: Thrown during decoding/deserialization

---

### Version Info

#### `versionInfo()`
Get version information about the Crous library.

**Returns:** Object with version information:
```javascript
{
    major: 2,
    minor: 0,
    patch: 0,
    string: '2.0.0',
    tuple: [2, 0, 0],
    hex: 0x020000
}
```

## Performance

Crous is designed for high performance with native C implementation:

- **Encoding**: ~2-5x faster than JSON.stringify()
- **Decoding**: ~2-4x faster than JSON.parse()
- **Size**: 40-60% smaller than JSON for typical data

## Testing

Run the test suite:

```bash
npm test
```

Or run individual tests:

```bash
node test/test_basic.js
```

## Building from Source

```bash
# Install dependencies
npm install

# Build native addon
npm run build

# Run tests
npm test
```

## Platform Support

- **Linux**: x64, ARM64
- **macOS**: x64, ARM64 (Apple Silicon)
- **Windows**: x64

## License

MIT License - see [LICENSE](../LICENSE) file for details.

## Author

Pawan Kumar <aegis.invincible@gmail.com>

## Links

- GitHub: https://github.com/axiomchronicles/crous
- Issues: https://github.com/axiomchronicles/crous/issues

## Related Projects

- **Python Crous**: High-performance binary serialization for Python
- **FLUX**: Human-readable serialization format (part of Crous ecosystem)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Changelog

### Version 2.0.0
- Initial Node.js release
- Full N-API implementation
- TypeScript definitions
- Custom serializers/decoders
- Set support
- File I/O support
- Comprehensive test suite
