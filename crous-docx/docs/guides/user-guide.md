---
sidebar_position: 2
description: Comprehensive tutorials and examples for using Crous, from basics to advanced techniques
---

import Tabs from '@theme/Tabs';
import TabItem from '@theme/TabItem';

# User Guide 📚

Master Crous with comprehensive tutorials, practical examples, and best practices.

## 🚀 Getting Started

### Your First Program

Let's get you up and running with your first Crous script:

```python
import crous

# Create some data
data = {
    'name': 'Alice',
    'age': 30,
    'tags': ['python', 'developer']
}

# Serialize to bytes
binary = crous.dumps(data)
print(f"✓ Serialized {len(binary)} bytes")

# Deserialize back
result = crous.loads(binary)
print(f"✓ Deserialized: {result}")
assert result == data
```

:::tip **Installation**
If you haven't installed Crous yet, check the [Installation Guide](installation.md).
:::

## 📦 Basic Serialization

### Supported Types

Crous handles Python's fundamental types:

<Tabs groupId="types">
  <TabItem value="primitives" label="Primitives">

```python
import crous

data = {
    'none': None,
    'bool_true': True,
    'bool_false': False,
    'integer': 42,
    'float': 3.14,
    'string': 'Hello, World!',
    'bytes': b'binary data',
}

binary = crous.dumps(data)
result = crous.loads(binary)
assert result == data
```

  </TabItem>
  <TabItem value="collections" label="Collections">

```python
import crous

data = {
    'list': [1, 2, 3, 4, 5],
    'nested': [1, [2, 3], [4, 5]],
    'dict': {'name': 'Bob', 'age': 25},
    'tuple': (1, 2, 3),  # Preserved as tuple!
}

binary = crous.dumps(data)
result = crous.loads(binary)
assert result == data
```

  </TabItem>
  <TabItem value="mixed" label="Mixed Data">

```python
import crous

# Complex, deeply nested structure
data = {
    'users': [
        {'id': 1, 'name': 'Alice', 'active': True},
        {'id': 2, 'name': 'Bob', 'active': False},
    ],
    'metadata': {
        'total': 2,
        'timestamp': '2024-01-01T00:00:00',
    },
    'scores': [95.5, 87.3, 92.1],
}

binary = crous.dumps(data)
result = crous.loads(binary)
```

  </TabItem>
</Tabs>

### Type Preservation ✨

Unlike JSON, Crous preserves Python types:

```python
import crous
import json

data = {
    'int_value': 1,
    'float_value': 1.0,
    'bool_value': True,
    'bytes_value': b'data',
    'tuple_value': (1, 2, 3),
}

# ✓ With Crous - Types are preserved!
crous_binary = crous.dumps(data)
crous_result = crous.loads(crous_binary)
assert type(crous_result['int_value']) is int       # ✓
assert type(crous_result['float_value']) is float   # ✓
assert type(crous_result['tuple_value']) is tuple   # ✓

# ✗ With JSON - Some types are lost
json_str = json.dumps(data)  # Oops! Tuples become lists, bytes not supported
```

:::info **Why This Matters**
Type preservation is critical for applications where:
- Configuration needs exact types (e.g., `1` vs `1.0`)
- Round-trip integrity is required
- You're building type-safe systems
:::

## 📁 File I/O

### Working with Files

<Tabs groupId="fileio">
  <TabItem value="path" label="File Path">

```python
import crous

data = {'config': {'debug': True, 'port': 8080}}

# Save to file
crous.dump(data, 'config.crous')

# Load from file
config = crous.load('config.crous')
assert config == data
```

  </TabItem>
  <TabItem value="fileobj" label="File Object">

```python
import crous

data = {'users': [{'id': 1, 'name': 'Alice'}]}

# Write with file object
with open('users.crous', 'wb') as f:
    crous.dump(data, f)

# Read with file object
with open('users.crous', 'rb') as f:
    users = crous.load(f)
    assert users == data
```

  </TabItem>
  <TabItem value="pathlib" label="Pathlib">

```python
import crous
from pathlib import Path

data = {'example': 'data'}
data_dir = Path('data')
data_dir.mkdir(exist_ok=True)

filepath = data_dir / 'output.crous'

# Pathlib strings must be converted
crous.dump(data, str(filepath))
result = crous.load(str(filepath))
```

  </TabItem>
</Tabs>

## 🔧 Custom Types

### Why Custom Serializers?

By default, Crous handles basic Python types. For other types like `datetime`, `Decimal`, or your own classes, you need custom serializers:

:::caution **Dictionary Keys Must Be Strings**
Crous dictionaries only support string keys. If you need non-string keys, convert them to strings as part of your serializer.
:::

<Tabs groupId="custom">
  <TabItem value="datetime" label="Datetime">

```python
from datetime import datetime
import crous

# Register a serializer
def serialize_datetime(obj):
    if isinstance(obj, datetime):
        return obj.isoformat()  # Convert to ISO string
    raise TypeError(f"Cannot serialize {type(obj)}")

crous.register_serializer(datetime, serialize_datetime)

# Now datetime works!
data = {
    'created': datetime(2024, 1, 15, 10, 30),
    'updated': datetime.now(),
}
binary = crous.dumps(data)
result = crous.loads(binary)
print(result['created'])  # '2024-01-15T10:30:00'
```

  </TabItem>
  <TabItem value="decimal" label="Decimal">

```python
from decimal import Decimal
import crous

def serialize_decimal(obj):
    if isinstance(obj, Decimal):
        return {'__decimal__': str(obj)}
    raise TypeError()

crous.register_serializer(Decimal, serialize_decimal)

# Use with prices/financial data
data = {
    'items': [
        {'name': 'Widget', 'price': Decimal('19.99')},
        {'name': 'Gadget', 'price': Decimal('49.99')},
    ]
}
binary = crous.dumps(data)
result = crous.loads(binary)

# Recreate Decimal from result
price = Decimal(result['items'][0]['price']['__decimal__'])
```

  </TabItem>
  <TabItem value="uuid" label="UUID">

```python
import uuid
import crous

def serialize_uuid(obj):
    if isinstance(obj, uuid.UUID):
        return str(obj)  # UUID to string
    raise TypeError()

crous.register_serializer(uuid.UUID, serialize_uuid)

data = {
    'id': uuid.uuid4(),
    'session_id': uuid.uuid4(),
}
binary = crous.dumps(data)
result = crous.loads(binary)
print(result['id'])  # UUID string like '550e8400-e29b-41d4-a716-446655440000'
```

  </TabItem>
  <TabItem value="set" label="Set/Frozenset">

```python
import crous

def serialize_set(obj):
    if isinstance(obj, (set, frozenset)):
        return {
            '__type__': 'frozenset' if isinstance(obj, frozenset) else 'set',
            'items': sorted(list(obj))  # Sort for deterministic output
        }
    raise TypeError()

crous.register_serializer(set, serialize_set)
crous.register_serializer(frozenset, serialize_set)

data = {
    'tags': {'python', 'rust', 'javascript'},
    'immutable_ids': frozenset([1, 2, 3]),
}
binary = crous.dumps(data)
```

  </TabItem>
  <TabItem value="custom" label="Custom Class">

```python
import crous

class Person:
    def __init__(self, name, age, email=None):
        self.name = name
        self.age = age
        self.email = email

def serialize_person(obj):
    if isinstance(obj, Person):
        return {
            '__person__': True,
            'name': obj.name,
            'age': obj.age,
            'email': obj.email,
        }
    raise TypeError()

crous.register_serializer(Person, serialize_person)

# Use it!
data = {
    'owner': Person('Alice', 30, 'alice@example.com'),
    'developer': Person('Bob', 28),
}
binary = crous.dumps(data)
result = crous.loads(binary)

# Reconstruct objects
owner = Person(
    result['owner']['name'],
    result['owner']['age'],
    result['owner']['email']
)
```

  </TabItem>
</Tabs>

## ❌ Error Handling

### Exception Types

<Tabs groupId="errors">
  <TabItem value="encode" label="Serialization Errors">

```python
import crous

# ❌ Non-string dictionary keys
try:
    crous.dumps({1: 'one', 2: 'two'})
except crous.CrousEncodeError as e:
    print(f"Encoding error: {e}")

# ❌ Unsupported type
try:
    crous.dumps({'obj': object()})
except crous.CrousEncodeError as e:
    print(f"Cannot serialize: {e}")
```

  </TabItem>
  <TabItem value="decode" label="Deserialization Errors">

```python
import crous

# ❌ Invalid/corrupted data
try:
    crous.loads(b'\xFF\xFF\xFF\xFF')
except crous.CrousDecodeError as e:
    print(f"Decoding error: {e}")

# ❌ Truncated data
incomplete = b'\x05\x00\x00\x00\x10hello'
try:
    crous.loads(incomplete)
except crous.CrousDecodeError as e:
    print(f"Data incomplete: {e}")
```

  </TabItem>
</Tabs>

## ⚡ Performance Tips

### 1. Batch Operations

```python
import crous
import time

# ❌ Slow: Many small operations
items = [{'id': i, 'value': i*2} for i in range(10000)]
start = time.time()
for item in items:
    crous.dumps(item)
slow_time = time.time() - start

# ✓ Fast: Serialize once
start = time.time()
binary = crous.dumps(items)
fast_time = time.time() - start

print(f"Slow: {slow_time:.3f}s, Fast: {fast_time:.3f}s")
print(f"Speedup: {slow_time/fast_time:.1f}x faster")
```

### 2. File I/O for Large Data

```python
import crous

large_data = {
    'records': [{'id': i, 'data': 'x' * 1000} for i in range(100000)]
}

# ✓ Better for large files
crous.dump(large_data, 'output.crous')
```

## 🎯 Advanced Usage

### Custom Object Hook

Modify dictionaries during deserialization:

```python
import crous

def uppercase_keys(d):
    """Convert all dict keys to uppercase."""
    return {k.upper(): v for k, v in d.items()}

data = {'name': 'alice', 'age': 30}
binary = crous.dumps(data)

# Normal
normal = crous.loads(binary)
print(normal)  # {'name': 'alice', 'age': 30}

# With hook
modified = crous.loads(binary, object_hook=uppercase_keys)
print(modified)  # {'NAME': 'alice', 'AGE': 30}
```

### Comparison with Other Formats

<Tabs groupId="comparison">
  <TabItem value="vs-json" label="vs JSON">

```python
import json
import crous

data = {
    'name': 'Alice',
    'scores': [95.5, 87.3, 92.1],
    'active': True,
}

# JSON: Text-based, human-readable
json_str = json.dumps(data)
json_bytes = json_str.encode('utf-8')
print(f"JSON: {len(json_bytes)} bytes")

# Crous: Binary, more compact
crous_bytes = crous.dumps(data)
print(f"Crous: {len(crous_bytes)} bytes")
print(f"Reduction: {(1 - len(crous_bytes)/len(json_bytes)) * 100:.1f}%")
```

  </TabItem>
  <TabItem value="vs-pickle" label="vs Pickle">

```python
import pickle
import crous

data = {'name': 'Alice', 'tags': ['python', 'rust']}

# Pickle: Python-specific, security concerns
pickle_bytes = pickle.dumps(data)
print(f"Pickle: {len(pickle_bytes)} bytes, Python-only")

# Crous: Language-agnostic, safe
crous_bytes = crous.dumps(data)
print(f"Crous: {len(crous_bytes)} bytes, language-agnostic")
```

  </TabItem>
</Tabs>

---

:::info **Next Steps**
- 🔌 Learn about [Custom Serializers](custom-serializers.md) in depth
- 📖 Check the [API Reference](../api/reference.md) for complete documentation
- ⚙️ Explore the [Architecture](../internals/architecture.md) to understand how Crous works
:::
