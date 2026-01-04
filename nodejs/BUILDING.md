# Crous Node.js Addon

This directory contains the Node.js/JavaScript bindings for the Crous serialization library.

## Structure

```
nodejs/
├── binding.gyp           # Node-gyp build configuration
├── package.json          # NPM package configuration
├── index.js             # Main JavaScript module
├── index.d.ts           # TypeScript type definitions
├── examples.js          # Usage examples
├── README.md            # Documentation
├── src/
│   └── crous_node.c     # N-API C bindings
├── test/
│   └── test_basic.js    # Test suite
└── build/               # Build output (generated)
```

## Building

### Prerequisites

- Node.js 14.0.0 or higher
- Python (for node-gyp)
- C compiler (GCC, Clang, or MSVC)
- node-gyp: `npm install -g node-gyp`

### Build Steps

```bash
# Install dependencies
npm install

# Build the native addon
npm run build

# Or use node-gyp directly
node-gyp configure
node-gyp build
```

## Testing

Run the test suite:

```bash
npm test
```

Run examples:

```bash
node examples.js
```

## Usage

### CommonJS (Node.js)

```javascript
const crous = require('crous');

const data = { name: 'Alice', age: 30 };
const binary = crous.dumps(data);
const result = crous.loads(binary);
```

### ES Modules

```javascript
import * as crous from 'crous';

const data = { name: 'Alice', age: 30 };
const binary = crous.dumps(data);
const result = crous.loads(binary);
```

### TypeScript

```typescript
import { dumps, loads } from 'crous';

interface User {
    name: string;
    age: number;
}

const user: User = { name: 'Alice', age: 30 };
const binary: Buffer = dumps(user);
const result: User = loads(binary);
```

## API Documentation

See [README.md](./README.md) for complete API documentation.

## Development

### Debugging

Build in debug mode:

```bash
node-gyp configure --debug
node-gyp build --debug
```

### Cleaning

Clean build artifacts:

```bash
npm run clean
# or
node-gyp clean
```

### Rebuilding

Force rebuild:

```bash
npm run build
# or
node-gyp rebuild
```

## Architecture

The Node.js addon consists of three layers:

1. **C Bindings** (`src/crous_node.c`): Native N-API bindings that interface with the core Crous C library
2. **JavaScript Wrapper** (`index.js`): High-level JavaScript API with error handling and convenience features
3. **TypeScript Definitions** (`index.d.ts`): Complete type definitions for TypeScript support

### N-API Benefits

- **Stable ABI**: Compatible across Node.js versions without recompilation
- **Performance**: Native speed for serialization/deserialization
- **Memory Safety**: Proper reference counting and garbage collection integration

## Platform Support

- **Linux**: x64, ARM64
- **macOS**: x64 (Intel), ARM64 (Apple Silicon)
- **Windows**: x64

## Troubleshooting

### Build fails with "node-gyp not found"

Install node-gyp globally:

```bash
npm install -g node-gyp
```

### Build fails with compiler errors

Make sure you have a C compiler installed:

- **Linux**: `sudo apt-get install build-essential`
- **macOS**: Install Xcode Command Line Tools: `xcode-select --install`
- **Windows**: Install Visual Studio Build Tools

### Module not found after building

Make sure the build completed successfully and the `.node` file exists in `build/Release/` or `build/Debug/`.

### Segmentation fault or crashes

This usually indicates a problem with the native code. Please report issues with:
- Node.js version
- Operating system
- Steps to reproduce
- Example code that triggers the crash

## Contributing

Contributions are welcome! Please ensure:

1. Tests pass: `npm test`
2. Code follows existing style
3. TypeScript definitions are updated if API changes
4. Examples are updated if needed

## License

MIT License - see [LICENSE](../LICENSE) file for details.
