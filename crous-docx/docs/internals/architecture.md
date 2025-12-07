---
sidebar_position: 1
---

# Architecture and Design

## Overview

Crous is implemented as a C extension for Python with three main components:

1. **C Core Library** (`crous.c`, `crous.h`): Binary serialization logic
2. **Python C Extension** (`pycrous.c`): Python bindings
3. **Python Wrapper** (`__init__.py`): High-level API and convenience functions

## Component Architecture

### C Core Library (`crous.c`)

The core library provides:

- **Value representation**: Tree-based `crous_value` structure
- **Encoding**: Convert values to binary format
- **Decoding**: Parse binary data to value tree
- **Streaming**: Input/output stream abstractions
- **Error handling**: Error codes and recovery

#### Key Structures

```c
/* Main value structure */
struct crous_value {
    crous_type_t type;        /* Type identifier */
    crous_value_data data;    /* Type-specific data */
};

/* Type union for all possible values */
typedef union {
    int b;                    /* Boolean */
    int64_t i;                /* Integer */
    double f;                 /* Float */
    struct { char *data; size_t len; } s;      /* String */
    struct { uint8_t *data; size_t len; } bytes;  /* Bytes */
    struct { crous_value **items; size_t len; } list;  /* List/tuple */
    struct { crous_dict_entry *entries; size_t len; } dict;  /* Dict */
    crous_tagged tagged;      /* Tagged value */
} crous_value_data;
```

#### Type System

```c
typedef enum {
    CROUS_TYPE_NULL,
    CROUS_TYPE_BOOL,
    CROUS_TYPE_INT,
    CROUS_TYPE_FLOAT,
    CROUS_TYPE_STRING,
    CROUS_TYPE_BYTES,
    CROUS_TYPE_LIST,
    CROUS_TYPE_TUPLE,
    CROUS_TYPE_DICT,
    CROUS_TYPE_TAGGED,
} crous_type_t;
```

#### Encoding Process

1. Recursively traverse value tree
2. Write type tag for each value
3. Write value-specific encoding:
   - Null/Bool: Type tag only
   - Int/Float: Type tag + binary representation
   - String/Bytes: Type tag + varint length + data
   - List/Tuple: Type tag + varint count + encoded items
   - Dict: Type tag + varint count + key-value pairs
   - Tagged: Type tag + varint tag ID + encoded value

#### Decoding Process

1. Read and validate magic bytes (0x43 0x52 0x4F 0x55, "CROU")
2. Read and check version (0x02)
3. Recursively decode values:
   - Read type tag
   - Dispatch to type-specific decoder
   - Validate and construct value
   - Return or error

#### Stream Abstraction

```c
/* Input stream interface */
typedef struct {
    void* user_data;
    size_t (*read)(void* user_data, uint8_t* buf, size_t max_len);
} crous_input_stream;

/* Output stream interface */
typedef struct {
    void* user_data;
    size_t (*write)(void* user_data, const uint8_t* buf, size_t len);
} crous_output_stream;
```

Allows:
- File-based I/O
- Memory buffers
- Network sockets
- Compression wrappers
- Custom implementations

### Python C Extension (`pycrous.c`)

The C extension provides Python bindings:

#### Main Functions

```c
/* Exported functions */
static PyObject* crous_dumps(PyObject *self, PyObject *args, PyObject *kwargs);
static PyObject* crous_loads(PyObject *self, PyObject *args, PyObject *kwargs);
static PyObject* crous_dump(PyObject *self, PyObject *args, PyObject *kwargs);
static PyObject* crous_load(PyObject *self, PyObject *args, PyObject *kwargs);
static PyObject* crous_register_serializer(PyObject *self, PyObject *args);
static PyObject* crous_unregister_serializer(PyObject *self, PyObject *args);
static PyObject* crous_register_decoder(PyObject *self, PyObject *args);
static PyObject* crous_unregister_decoder(PyObject *self, PyObject *args);
```

#### Type Conversion

- **Python → C**: Convert Python objects to `crous_value` tree
- **C → Python**: Convert `crous_value` tree back to Python objects

**Conversion table:**

| Python Type | C Type | Encoding |
|-------------|--------|----------|
| `None` | `NULL` | Type tag 0x00 |
| `True` | `BOOL(1)` | Type tag 0x02 |
| `False` | `BOOL(0)` | Type tag 0x01 |
| `int` | `INT64` | Type tag 0x03 + i64 |
| `float` | `FLOAT64` | Type tag 0x04 + f64 |
| `str` | `STRING` | Type tag 0x05 + varint + UTF-8 |
| `bytes` | `BYTES` | Type tag 0x06 + varint + data |
| `list` | `LIST` | Type tag 0x07 + varint count + items |
| `tuple` | `TUPLE` | Type tag 0x08 + varint count + items |
| `dict` | `DICT` | Type tag 0x09 + varint count + pairs |

#### Exception Handling

Three exception classes:

```c
static PyObject *CrousError;        /* Base exception */
static PyObject *CrousEncodeError;  /* Encoding failed */
static PyObject *CrousDecodeError;  /* Decoding failed */
```

Raised via `PyErr_SetString()` or `PyErr_Format()`.

#### Reference Counting

- Python objects reference counted using `Py_INCREF`/`Py_DECREF`
- C values use manual memory management (`malloc`/`free`)
- Clear ownership rules prevent leaks

### Python Wrapper (`__init__.py`)

High-level convenience layer:

```python
def dump(obj, fp, *, default=None):
    """Serialize to file path or file object."""
    if isinstance(fp, str):
        with open(fp, 'wb') as f:
            _crous_ext.dump(obj, f, default=default)
    else:
        _crous_ext.dump(obj, fp, default=default)

def load(fp, *, object_hook=None):
    """Deserialize from file path or file object."""
    if isinstance(fp, str):
        with open(fp, 'rb') as f:
            return _crous_ext.load(f, object_hook=object_hook)
    else:
        return _crous_ext.load(fp, object_hook=object_hook)
```

Also handles:
- Module initialization
- API validation
- Docstrings for IDE support
- Version information

## Data Flow

### Encoding Flow

```
Python Object
    ↓
Python C Extension (pycrous.c)
    ↓
Convert to crous_value tree
    ↓
C Core Library (crous.c)
    ↓
Encode to binary format
    ↓
Output Stream / Buffer
    ↓
Bytes
```

### Decoding Flow

```
Bytes
    ↓
Input Stream / Buffer
    ↓
C Core Library (crous.c)
    ↓
Decode from binary format
    ↓
crous_value tree
    ↓
Python C Extension (pycrous.c)
    ↓
Convert to Python Object
    ↓
Python Object
```

## Memory Management

### Python Objects

- Reference counting via `Py_INCREF`/`Py_DECREF`
- Automatic garbage collection
- Clear ownership semantics

### C Values

- Manual allocation with `malloc`
- Manual deallocation with `free`
- Type-specific cleanup functions

#### Allocation Functions

```c
crous_value *crous_value_new_null(void);
crous_value *crous_value_new_bool(int b);
crous_value *crous_value_new_int(int64_t v);
crous_value *crous_value_new_float(double d);
crous_value *crous_value_new_string(const char *data, size_t len);
crous_value *crous_value_new_bytes(const uint8_t *data, size_t len);
crous_value *crous_value_new_list(size_t capacity);
crous_value *crous_value_new_tuple(size_t capacity);
crous_value *crous_value_new_dict(size_t capacity);
crous_value *crous_value_new_tagged(uint32_t tag, crous_value *inner);
```

#### Accessor Functions

```c
crous_type_t crous_value_get_type(const crous_value *v);
int crous_value_get_bool(const crous_value *v);
int64_t crous_value_get_int(const crous_value *v);
double crous_value_get_float(const crous_value *v);
const char *crous_value_get_string(const crous_value *v, size_t *out_len);
const uint8_t *crous_value_get_bytes(const crous_value *v, size_t *out_len);
```

## Error Handling

### Error Codes

```c
typedef enum {
    CROUS_OK = 0,
    CROUS_ERR_INVALID_TYPE = 1,
    CROUS_ERR_DECODE = 2,
    CROUS_ERR_ENCODE = 3,
    CROUS_ERR_OOM = 4,
    CROUS_ERR_OVERFLOW = 5,
    CROUS_ERR_INTERNAL = 6,
    CROUS_ERR_STREAM = 7,
    CROUS_ERR_TAG_UNKNOWN = 8,
    CROUS_ERR_TRUNCATED = 9,
    CROUS_ERR_INVALID_HEADER = 10,
} crous_err_t;
```

### Error Propagation

C functions return error codes. Python wrapper converts to exceptions:

```
C Error Code
    ↓
Python C Extension detects error
    ↓
PyErr_SetString(CrousDecodeError, message)
    ↓
Exception raised to Python code
```

## Thread Safety

- Encoding/decoding is thread-safe at the function level
- No shared mutable state
- Each call has its own value tree
- Python's GIL provides synchronization for Python objects

**Caveat:** Do not modify objects while they're being serialized in another thread.

## Performance Considerations

### Optimization Strategies

1. **Small Integer Encoding**: Single-byte encoding for -32 to 28
2. **Varint Encoding**: Variable-length integers reduce size
3. **Buffer Management**: Pre-allocation with exponential growth
4. **Type Optimization**: Direct type tag dispatch in decoder
5. **Stream Abstraction**: Allows buffering and custom I/O

### Benchmarks

- Encoding: ~50-100 MB/s
- Decoding: ~100-200 MB/s
- Output size: 20-40% smaller than JSON

## Future Enhancements

### Planned Features

1. **Streaming API**: Multi-record serialization
2. **Compression**: Built-in gzip/zstd support
3. **Schema Validation**: Type checking and validation
4. **Async I/O**: Non-blocking file operations
5. **Built-in Tagged Types**: Direct datetime, Decimal support

### Extension Points

- Custom serializer registry (already present)
- Stream implementations (abstraction ready)
- Type system extensibility (tagged values)

## Integration Points

### Python Integration

- Module initialization via `PyInit_crous()`
- Exception class registration
- Method registration via `PyMethodDef`
- Module-level state management

### C Integration

- Pure C99 (no compiler-specific extensions)
- No external dependencies
- POSIX-compatible memory management
- Portable integer types via `<stdint.h>`

## Testing Architecture

### Test Categories

1. **Unit Tests**: Individual functions
2. **Integration Tests**: End-to-end workflows
3. **Regression Tests**: Known issues
4. **Performance Tests**: Benchmarks
5. **Stress Tests**: Large data, deep nesting

### Test Coverage

- Basic types: 100%
- Containers: 100%
- Error paths: 90%+
- Custom serializers: 100%
- File I/O: 100%
