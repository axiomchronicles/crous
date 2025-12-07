---
sidebar_position: 1
---

# API Reference

Complete reference for the Crous API.

## Table of Contents

1. [Serialization Functions](#serialization-functions)
2. [File I/O Functions](#file-io-functions)
3. [Custom Serializers](#custom-serializers)
4. [Custom Decoders](#custom-decoders)
5. [Exception Classes](#exception-classes)
6. [Classes](#classes)

## Serialization Functions

### `dumps(obj, *, default=None, allow_custom=True) -> bytes`

Serialize a Python object to Crous binary format.

**Signature:**
```python
def dumps(
    obj: Any,
    *,
    default: Optional[Callable[[Any], Any]] = None,
    allow_custom: bool = True
) -> bytes
```

**Parameters:**

- `obj` (Any): Python object to serialize. Must be one of:
  - `None`, `bool`, `int`, `float`, `str`, `bytes`
  - `list`, `tuple`, `dict` (string keys only)
  - Any type with a registered custom serializer

- `default` (callable, optional): Called when object cannot be serialized.
  Function should take object and return serializable value, or raise TypeError.
  Default behavior: raise CrousEncodeError.

- `allow_custom` (bool): Whether to permit custom types via serializers.
  Default: True

**Returns:**

- `bytes`: Binary data in Crous format

**Raises:**

- `CrousEncodeError`: If object cannot be serialized
- `TypeError`: If default handler rejects object

**Example:**
```python
import crous

# Basic types
data = {
    'name': 'Alice',
    'age': 30,
    'active': True,
    'balance': 100.50,
    'tags': ['python', 'rust'],
}
binary = crous.dumps(data)

# With custom type via serializer
from datetime import datetime
crous.register_serializer(datetime, lambda x: x.isoformat() if isinstance(x, datetime) else None)
binary = crous.dumps({'created': datetime.now()})
```

---

### `loads(data, *, object_hook=None, decoder=None) -> object`

Deserialize Crous binary data to Python object.

**Signature:**
```python
def loads(
    data: Union[bytes, bytearray],
    *,
    object_hook: Optional[Callable[[dict], dict]] = None,
    decoder: Optional['CrousDecoder'] = None
) -> Any
```

**Parameters:**

- `data` (bytes-like): Crous-encoded binary data

- `object_hook` (callable, optional): Called with every dict decoded.
  Should take dict and return modified dict. Default: None (no modification).

- `decoder` (CrousDecoder, optional): Custom decoder instance. Default: None (use built-in).

**Returns:**

- `Any`: Deserialized Python object

**Raises:**

- `CrousDecodeError`: If data is malformed, truncated, or contains unknown types

**Example:**
```python
import crous

binary = crous.dumps({'name': 'Alice', 'age': 30})
obj = crous.loads(binary)
assert obj == {'name': 'Alice', 'age': 30}

# With object_hook to modify dicts
def hook(d):
    return {k.upper(): v for k, v in d.items()}

obj = crous.loads(binary, object_hook=hook)
assert obj == {'NAME': 'Alice', 'AGE': 30}
```

---

## File I/O Functions

### `dump(obj, fp, *, default=None) -> None`

Serialize object to file.

**Signature:**
```python
def dump(
    obj: Any,
    fp: Union[str, BinaryIO],
    *,
    default: Optional[Callable[[Any], Any]] = None
) -> None
```

**Parameters:**

- `obj` (Any): Object to serialize (same constraints as dumps())

- `fp` (str or file-like): 
  - String: file path, will be opened in 'wb' mode
  - File-like: must have `write(bytes)` method and be opened in 'wb' mode

- `default` (callable, optional): Custom serializer (same as dumps())

**Returns:**

- `None`

**Raises:**

- `CrousEncodeError`: If object cannot be serialized
- `IOError`: If file operation fails
- `TypeError`: If fp is not str or file-like
- `FileNotFoundError`: If directory doesn't exist

**Example:**
```python
import crous

data = {'users': [{'id': 1, 'name': 'Alice'}]}

# Using file path
crous.dump(data, 'users.crous')

# Using file object
with open('users.crous', 'wb') as f:
    crous.dump(data, f)

# Using Path object (converted to str)
from pathlib import Path
crous.dump(data, str(Path('data') / 'users.crous'))
```

---

### `load(fp, *, object_hook=None) -> object`

Deserialize object from file.

**Signature:**
```python
def load(
    fp: Union[str, BinaryIO],
    *,
    object_hook: Optional[Callable[[dict], dict]] = None
) -> Any
```

**Parameters:**

- `fp` (str or file-like):
  - String: file path, will be opened in 'rb' mode
  - File-like: must have `read(n)` method and be opened in 'rb' mode

- `object_hook` (callable, optional): Modify dicts during decoding

**Returns:**

- `Any`: Deserialized Python object

**Raises:**

- `CrousDecodeError`: If data is malformed
- `IOError`: If file operation fails
- `FileNotFoundError`: If file doesn't exist
- `TypeError`: If fp is not str or file-like

**Example:**
```python
import crous

# Using file path
obj = crous.load('users.crous')

# Using file object
with open('users.crous', 'rb') as f:
    obj = crous.load(f)

# Check if file exists
import os
if os.path.exists('users.crous'):
    obj = crous.load('users.crous')
```

---

## Custom Serializers

### `register_serializer(typ, func) -> None`

Register a custom serializer for a type.

**Signature:**
```python
def register_serializer(
    typ: type,
    func: Callable[[Any], Any]
) -> None
```

**Parameters:**

- `typ` (type): Type class to handle (e.g., `datetime`, custom class)

- `func` (callable): Serialization function:
  - Takes: object of type `typ`
  - Returns: Serializable value (must be one of supported types)
  - Raises: `TypeError` if cannot serialize

**Returns:**

- `None`

**Side Effects:**

- Modifies global serializer registry
- Overwrites existing serializer for type if present

**Example:**
```python
import crous
from datetime import datetime
from decimal import Decimal

# Serialize datetime as ISO string
crous.register_serializer(datetime, lambda x: x.isoformat() if isinstance(x, datetime) else None)

# Serialize Decimal as string (preserves precision)
def decimal_serializer(obj):
    if isinstance(obj, Decimal):
        return str(obj)
    raise TypeError(f"Cannot serialize {type(obj)}")

crous.register_serializer(Decimal, decimal_serializer)

# Now these can be serialized
data = {
    'created': datetime.now(),
    'price': Decimal('19.99'),
}
binary = crous.dumps(data)
```

---

### `unregister_serializer(typ) -> None`

Unregister a custom serializer.

**Signature:**
```python
def unregister_serializer(typ: type) -> None
```

**Parameters:**

- `typ` (type): Type class to unregister

**Returns:**

- `None`

**Side Effects:**

- Removes type from serializer registry
- Subsequent attempts to serialize type will fail

**Example:**
```python
import crous
from datetime import datetime

# Register, use, then unregister
crous.register_serializer(datetime, lambda x: x.isoformat() if isinstance(x, datetime) else None)
binary = crous.dumps({'now': datetime.now()})
crous.unregister_serializer(datetime)

# This now fails
try:
    crous.dumps({'now': datetime.now()})
except crous.CrousEncodeError:
    print("datetime no longer registered")
```

---

## Custom Decoders

### `register_decoder(tag, func) -> None`

Register a custom decoder for tagged values.

**Signature:**
```python
def register_decoder(
    tag: int,
    func: Callable[[Any], Any]
) -> None
```

**Parameters:**

- `tag` (int): Integer tag identifier:
  - 1-7: Reserved for built-in types
  - 100-199: User-defined tags (recommended)

- `func` (callable): Decoding function:
  - Takes: Tagged value data
  - Returns: Decoded Python object

**Returns:**

- `None`

**Example:**
```python
import crous

# Define custom decoder for tag 100
def my_decoder(data):
    # Reconstruct object from serialized data
    return MyType(data)

crous.register_decoder(100, my_decoder)
```

---

### `unregister_decoder(tag) -> None`

Unregister a custom decoder.

**Signature:**
```python
def unregister_decoder(tag: int) -> None
```

**Parameters:**

- `tag` (int): Tag identifier to unregister

**Returns:**

- `None`

---

## Exception Classes

### `CrousError`

Base exception for all Crous errors.

**Inheritance:** `Exception`

**Usage:**
```python
import crous

try:
    # Any Crous operation
    crous.dumps(unknown_object)
except crous.CrousError as e:
    print(f"Crous error occurred: {e}")
```

---

### `CrousEncodeError`

Raised when serialization fails.

**Inheritance:** `CrousError`

**Common causes:**
- Unsupported type without custom serializer
- Dict with non-string key
- Type converter raises exception
- Circular references (if not handled)

**Example:**
```python
import crous

# Unsupported type
try:
    crous.dumps({1: 'value'})  # Non-string dict key
except crous.CrousEncodeError as e:
    print(f"Cannot serialize: {e}")
```

---

### `CrousDecodeError`

Raised when deserialization fails.

**Inheritance:** `CrousError`

**Common causes:**
- Truncated or incomplete data
- Invalid type tag
- Corrupted binary format
- Unknown tagged type

**Example:**
```python
import crous

# Malformed data
try:
    crous.loads(b'\x00\x01')  # Incomplete
except crous.CrousDecodeError as e:
    print(f"Cannot deserialize: {e}")
```

---

## Classes

### `CrousEncoder`

Encoder class for structured encoding (if exposed).

**Note:** This class may not be fully exposed in the public API. Use `dumps()` function instead.

---

### `CrousDecoder`

Decoder class for structured decoding (if exposed).

**Note:** This class may not be fully exposed in the public API. Use `loads()` function instead.

---

## Type Preservation

Crous preserves Python types during round-trip serialization:

| Type | Preserved | Example |
|------|-----------|---------|
| `int` vs `float` | Yes | `1` stays int, `1.0` stays float |
| `str` vs `bytes` | Yes | `'hello'` stays str, `b'hello'` stays bytes |
| `bool` vs `int` | Yes | `True` stays bool, `1` stays int |
| `None` vs `''` | Yes | `None` distinct from empty string |
| `list` vs `tuple` | Tuple → list | Tuples become lists |
| Dict key order | Yes (3.7+) | Insertion order preserved |

---

## Complete Example

```python
import crous
from datetime import datetime

# Define custom serializer
def datetime_to_string(obj):
    if isinstance(obj, datetime):
        return obj.isoformat()
    raise TypeError(f"Cannot serialize {type(obj)}")

crous.register_serializer(datetime, datetime_to_string)

# Prepare complex data
data = {
    'users': [
        {'id': 1, 'name': 'Alice', 'joined': datetime(2023, 1, 15)},
        {'id': 2, 'name': 'Bob', 'joined': datetime(2023, 6, 20)},
    ],
    'metadata': {
        'total_users': 2,
        'last_updated': datetime.now(),
    }
}

# Serialize to bytes
binary = crous.dumps(data)
print(f"Serialized: {len(binary)} bytes")

# Save to file
crous.dump(data, 'users.crous')

# Load from file
loaded = crous.load('users.crous')

# Verify data (note: datetime converted to string)
assert loaded['users'][0]['name'] == 'Alice'

# Clean up
crous.unregister_serializer(datetime)
```
