---
sidebar_position: 0
slug: /
description: High-performance binary serialization for Python with type preservation and compact format
---

import {Hero, Card, CardGrid} from '@site/src/components/index';

# Welcome to Crous 🚀

**Crous** is a high-performance binary serialization library for Python that provides compact, type-preserving serialization with a simple and intuitive API.

## What is Crous?

Crous (pronounced "crux") is a Python library that serializes Python objects to a compact binary format and deserializes them back. It's designed for developers who need:

- **⚡ Performance**: Optimized C core for blazing-fast serialization
- **📦 Compact output**: 30-50% smaller than JSON for most data
- **🎯 Type safety**: Preserves exact Python types (int vs float, str vs bytes, etc.)
- **🔧 Simplicity**: Familiar API like the standard `json` module
- **🛠️ Extensibility**: Support for custom types via serializers
- **✅ Reliability**: Comprehensive error handling and validation

## Quick Start

import Tabs from '@theme/Tabs';
import TabItem from '@theme/TabItem';

<Tabs>
  <TabItem value="install" label="Installation">

```bash
pip install crous
```

  </TabItem>
  <TabItem value="basic" label="Basic Usage">

```python
import crous

# Serialize to bytes
data = {'name': 'Alice', 'age': 30, 'active': True}
binary = crous.dumps(data)

# Deserialize from bytes
result = crous.loads(binary)
print(result)  # {'name': 'Alice', 'age': 30, 'active': True}
```

  </TabItem>
  <TabItem value="files" label="File I/O">

```python
import crous

# Save to file
crous.dump(data, 'data.crous')

# Load from file
data = crous.load('data.crous')
```

  </TabItem>
</Tabs>

## Supported Types

| Type | Example | Notes |
|------|---------|-------|
| **None** | `None` | NULL value |
| **Boolean** | `True`, `False` | Preserves bool type ✓ |
| **Integer** | `42`, `-1000`, `2**60` | 64-bit signed |
| **Float** | `3.14`, `inf`, `nan` | IEEE 754 double |
| **String** | `'hello'`, `'你好'` | Unicode UTF-8 |
| **Bytes** | `b'data'`, `b'\x00\xff'` | Binary data |
| **List** | `[1, 2, 3]`, `[1, 'a', None]` | Heterogeneous |
| **Dictionary** | `{'a': 1}`, `{'key': [1, 2]}` | String keys only |
| **Tuple** | `(1, 2, 3)`, `(1, 'a')` | Preserved as tuple ✓ |

## Why Choose Crous?

<CardGrid>
  <Card title="🚀 Lightning Fast" description="Optimized C implementation delivers sub-millisecond performance for most operations" />
  <Card title="📦 Ultra Compact" description="Typically 30-50% smaller than JSON, perfect for storage and network transfer" />
  <Card title="🎯 Type Safe" description="Distinguishes between int/float, str/bytes, and tuple/list to maintain data integrity" />
  <Card title="🔧 Easy to Use" description="Simple API familiar to anyone who has used the json module" />
  <Card title="🛠️ Extensible" description="Register custom serializers for datetime, UUID, Decimal, and any custom type" />
  <Card title="✅ Reliable" description="Clear error messages and comprehensive exception types for debugging" />
</CardGrid>

## Common Use Cases

:::info **Configuration Storage**
Store structured configuration with type safety instead of INI/YAML files.
:::

:::tip **Network Communication**
Serialize Python objects for efficient transmission between processes or services.
:::

:::success **Caching**
Cache complex Python objects efficiently with type preservation.
:::

:::note **Testing**
Serialize test fixtures to reproducible binary format for deterministic tests.
:::

## Example: Type Preservation in Action

```python
import crous

# JSON loses type information
import json
data = {'int': 1, 'float': 1.0, 'text': 'hello', 'bytes': b'data'}

json_bytes = json.dumps(data).encode()  # 53 bytes
crous_bytes = crous.dumps(data)         # 24 bytes
print(f"JSON: {len(json_bytes)}, Crous: {len(crous_bytes)}")

# When loaded back, Crous preserves types:
result = crous.loads(crous_bytes)
assert type(result['int']) is int        # ✓ Preserved!
assert type(result['float']) is float    # ✓ Preserved!
assert isinstance(result['bytes'], bytes)  # ✓ Preserved!
```

## Example: Custom Types

For types not directly supported (like datetime), use custom serializers:

```python
from datetime import datetime
import crous

# Register a custom serializer
crous.register_serializer(
    datetime,
    lambda x: x.isoformat() if isinstance(x, datetime) else None
)

# Now datetime works seamlessly
data = {'created': datetime.now(), 'name': 'Project'}
binary = crous.dumps(data)
result = crous.loads(binary)
print(f"Created: {result['created']}")  # ISO format string
```

:::info **Learn More**
See [Custom Serializers](/docs/guides/custom-serializers) for comprehensive examples with UUID, Decimal, Set, and more.
:::

## Documentation Structure

- 📚 **[Installation Guide](/docs/guides/installation)** - Setup for all platforms with troubleshooting
- 🎓 **[User Guide](/docs/guides/user-guide)** - Tutorials, examples, and best practices
- 🔌 **[API Reference](/docs/api/reference)** - Complete function and exception reference
- 🎯 **[Custom Serializers](/docs/guides/custom-serializers)** - Extend Crous with custom types
- ⚙️ **[Architecture](/docs/internals/architecture)** - System design and implementation details
- 🤝 **[Developer Guide](/docs/contributing/developer-guide)** - Contributing to Crous

## Project Info

import {Badge} from '@site/src/components/index';

<div style={{display: 'flex', gap: '10px', flexWrap: 'wrap', marginBottom: '20px'}}>
  <Badge label="Version" value="2.0.0" />
  <Badge label="Python" value="3.7+" />
  <Badge label="License" value="MIT" />
  <Badge label="Status" value="Production Ready" color="#25c2a0" />
</div>

| | |
|---|---|
| **Current Version** | 2.0.0 |
| **Python Support** | 3.7, 3.8, 3.9, 3.10, 3.11, 3.12+ |
| **Platforms** | Linux, macOS, Windows, ARM64 (experimental) |
| **License** | MIT |
| **GitHub** | [crous-project/crous](https://github.com/crous-project/crous) |
| **PyPI** | [crous](https://pypi.org/project/crous) |

## Getting Help

:::info **Have Questions?**
- 🐛 **Bug Reports**: [GitHub Issues](https://github.com/crous-project/crous/issues)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/crous-project/crous/discussions)
- 📖 **Full Documentation**: [Explore Docs](/docs)
:::

---

**Ready to get started?** [Install Crous now](/docs/guides/installation) or jump into the [User Guide](/docs/guides/user-guide)! 🎉
