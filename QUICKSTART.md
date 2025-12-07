# Crous Quick Start

A fast start guide to get up and running with Crous quickly.

## Installation

```bash
pip install crous
```

## 5-Minute Tutorial

### Basic Serialization

```python
import crous

# Serialize
data = {'name': 'Alice', 'scores': [95, 87, 92]}
binary = crous.dumps(data)
print(f"Serialized: {binary[:20]}...")  # First 20 bytes

# Deserialize
result = crous.loads(binary)
print(result)  # {'name': 'Alice', 'scores': [95, 87, 92]}
```

### File I/O

```python
import crous

# Write to file
crous.dump({'user': 'bob'}, 'data.crous')

# Read from file
data = crous.load('data.crous')
print(data)
```

### Supported Types

```python
import crous

# Crous supports all these types
data = {
    'none': None,
    'bool': True,
    'int': 42,
    'float': 3.14,
    'str': 'hello',
    'bytes': b'world',
    'list': [1, 2, 3],
    'tuple': (4, 5, 6),
    'nested': {'key': 'value'}
}

binary = crous.dumps(data)
result = crous.loads(binary)
print(result)
```

### Size Comparison

```python
import json
import crous

data = {'x': 1, 'y': 2, 'z': 3} * 100

json_bytes = json.dumps(data).encode()
crous_bytes = crous.dumps(data)

print(f"JSON:   {len(json_bytes)} bytes")
print(f"Crous:  {len(crous_bytes)} bytes")
print(f"Saving: {len(json_bytes) - len(crous_bytes)} bytes ({100 * (1 - len(crous_bytes)/len(json_bytes)):.1f}%)")
```

## Error Handling

```python
import crous

try:
    binary = crous.dumps({'data': object()})
except crous.CrousEncodeError as e:
    print(f"Encoding error: {e}")

try:
    crous.loads(b'invalid')
except crous.CrousDecodeError as e:
    print(f"Decoding error: {e}")
```

## Next Steps

- Read the [full documentation](https://crous.readthedocs.io)
- Check out [examples](https://github.com/crous-project/crous/tree/main/examples)
- Explore [API reference](https://crous.readthedocs.io/docs/api/reference)
- Review [architecture](https://crous.readthedocs.io/docs/internals/architecture)

## Getting Help

- GitHub Issues: https://github.com/crous-project/crous/issues
- GitHub Discussions: https://github.com/crous-project/crous/discussions
- Email: support@crous.dev
