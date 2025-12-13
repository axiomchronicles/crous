# CROUS - Modular Architecture

## Directory Structure

```
crous/
├── include/              # Public headers
│   ├── crous.h          # Main header (includes all)
│   ├── crous_types.h    # Core type definitions
│   ├── crous_errors.h   # Error handling
│   ├── crous_arena.h    # Memory arena allocator
│   ├── crous_token.h    # Token definitions
│   ├── crous_lexer.h    # Lexer interface
│   ├── crous_parser.h   # Parser interface
│   ├── crous_value.h    # Value API
│   └── crous_binary.h   # Binary encoding/decoding
│
├── src/c/
│   ├── core/            # Core components
│   │   ├── errors.c     # Error handling implementation
│   │   ├── arena.c      # Memory arena implementation
│   │   └── value.c      # Value constructors/destructors/operations
│   │
│   ├── lexer/           # Tokenization
│   │   └── lexer.c      # Lexer implementation
│   │
│   ├── parser/          # Parsing
│   │   └── parser.c     # Parser implementation
│   │
│   ├── binary/          # Serialization
│   │   └── binary.c     # Binary encoding/decoding implementation
│   │
│   └── utils/           # Utilities
│       └── token.c      # Token utility functions
│
├── pycrous.c           # Python C extension bindings
├── crous.c             # (Legacy - kept for reference)
└── crous.h             # (Legacy - kept for reference)
```

## Module Responsibilities

### Types (`crous_types.h`)
- Core type definitions (enum, structs)
- Value type enumeration
- Error codes
- Stream interfaces
- Constants (magic numbers, max sizes, etc.)

### Errors (`crous_errors.h` / `core/errors.c`)
- Human-readable error messages
- Error severity classification

### Arena (`crous_arena.h` / `core/arena.c`)
- Efficient memory allocation
- Chunk-based allocation strategy
- Reset and cleanup

### Token (`crous_token.h` / `utils/token.c`)
- Token type definitions
- Token factory functions
- Token type string conversion

### Lexer (`crous_lexer.h` / `lexer/lexer.c`)
- Text tokenization
- Comment handling
- String/number parsing
- Location tracking

### Parser (`crous_parser.h` / `parser/parser.c`)
- Token stream to value tree conversion
- Recursive descent parser
- Error reporting with location info

### Value (`crous_value.h` / `core/value.c`)
- Value constructors (null, bool, int, float, string, bytes, list, tuple, dict, tagged)
- Value getters
- List/Tuple operations (get, set, append)
- Dictionary operations (get, set, entries)
- Tree memory cleanup

### Binary (`crous_binary.h` / `binary/binary.c`)
- Encoding: value → binary stream
- Decoding: binary stream → value
- Varint encoding/decoding
- UTF-8 validation
- File I/O convenience functions
- Buffer stream helpers

## Compilation

The refactored code compiles to a single `.so` file in the `crous/` directory:
```
crous/crous.cpython-314-darwin.so
```

### Build Process
```bash
python setup.py build_ext --inplace
```

This compiles all source files:
- 8 C files from the modular architecture
- Python bindings (pycrous.c)
- Generates a single extension module

## Key Design Decisions

1. **Modular Headers**: Each component has a clear interface in `include/`
2. **Opaque Types**: Internal structures are hidden (e.g., `crous_arena`)
3. **Stream-based I/O**: Encode/decode work with stream interfaces for flexibility
4. **Memory Management**: Value trees are allocated with malloc/free; arena is optional
5. **Error Handling**: All C functions return error codes; Python layer raises exceptions
6. **Single Extension**: Compiles to one `.so` file for simplicity

## Future Enhancements

- Parser: Full escape sequence handling in strings
- Lexer: Support for raw strings (r"...")
- Arena: Optional arena-based value allocation
- Binary: Custom serializer/deserializer registry
- Fuzz Testing: Dedicated fuzz testing harness
- Performance: Benchmarking and optimization
