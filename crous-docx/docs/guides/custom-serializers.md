---
sidebar_position: 3
---

# Custom Serializers Guide

## Overview

Crous supports extensible serialization through custom serializers and decoders. This allows you to serialize any Python type, not just the built-in types.

## When to Use Custom Serializers

### Built-in Types (No Serializer Needed)

```python
import crous

# These work out of the box
data = {
    'null': None,
    'bool': True,
    'int': 42,
    'float': 3.14,
    'str': 'hello',
    'bytes': b'data',
    'list': [1, 2, 3],
    'dict': {'key': 'value'},
}
crous.dumps(data)  # Works!
```

### Non-Built-in Types (Need Serializer)

```python
import crous
from datetime import datetime

# This fails without serializer
crous.dumps({'now': datetime.now()})  # ❌ CrousEncodeError

# With serializer, it works
crous.register_serializer(datetime, lambda x: x.isoformat() if isinstance(x, datetime) else None)
crous.dumps({'now': datetime.now()})  # ✅ Works!
```

## Basic Serializer Pattern

### Minimal Serializer

```python
import crous

def my_serializer(obj):
    """Serializer function."""
    # Check if this is the type we handle
    if isinstance(obj, MyType):
        # Return a serializable value (dict, list, str, int, etc.)
        return obj.to_dict()
    # Raise TypeError if not our type
    raise TypeError(f"Cannot serialize {type(obj)}")

# Register it
crous.register_serializer(MyType, my_serializer)

# Use it
data = {'object': MyType(...)}
binary = crous.dumps(data)

# Unregister when done
crous.unregister_serializer(MyType)
```

### Full Serializer Example

```python
from datetime import datetime
import crous

def datetime_serializer(obj):
    """Serialize datetime to ISO string."""
    if isinstance(obj, datetime):
        return {
            '__type__': 'datetime',
            '__value__': obj.isoformat(),
        }
    raise TypeError(f"Cannot serialize {type(obj)}")

# Register
crous.register_serializer(datetime, datetime_serializer)

# Use
data = {'created': datetime(2023, 12, 25, 10, 30)}
binary = crous.dumps(data)

# Result after load
result = crous.loads(binary)
# result = {'created': {'__type__': 'datetime', '__value__': '2023-12-25T10:30:00'}}
```

## Common Patterns

### Pattern 1: Type Marker

Add a marker field to identify the type during deserialization:

```python
import crous
from datetime import datetime

def datetime_serializer(obj):
    if isinstance(obj, datetime):
        return {
            '__datetime__': True,
            'value': obj.isoformat(),
        }
    raise TypeError()

crous.register_serializer(datetime, datetime_serializer)

data = {'created': datetime.now()}
binary = crous.dumps(data)
result = crous.loads(binary)
# result = {'created': {'__datetime__': True, 'value': '2023-12-25T10:30:00'}}
```

### Pattern 2: Type Conversion

Convert to simple serializable type:

```python
import crous
from datetime import datetime
from decimal import Decimal

# Datetime as ISO string
crous.register_serializer(
    datetime,
    lambda x: x.isoformat() if isinstance(x, datetime) else None
)

# Decimal as string (preserves precision)
def decimal_serializer(obj):
    if isinstance(obj, Decimal):
        return str(obj)
    raise TypeError()

crous.register_serializer(Decimal, decimal_serializer)

data = {
    'created': datetime.now(),
    'price': Decimal('19.99'),
}
binary = crous.dumps(data)
# Result: {'created': '2024-01-15T10:30:00', 'price': '19.99'}
```

### Pattern 3: Nested Structures

Serialize complex objects as nested dicts:

```python
import crous

class Address:
    def __init__(self, street, city, country):
        self.street = street
        self.city = city
        self.country = country

class Person:
    def __init__(self, name, age, address):
        self.name = name
        self.age = age
        self.address = address

def person_serializer(obj):
    if isinstance(obj, Person):
        return {
            '__person__': True,
            'name': obj.name,
            'age': obj.age,
            'address': {
                '__address__': True,
                'street': obj.address.street,
                'city': obj.address.city,
                'country': obj.address.country,
            }
        }
    raise TypeError()

crous.register_serializer(Person, person_serializer)

address = Address('123 Main St', 'Anytown', 'USA')
person = Person('Alice', 30, address)
binary = crous.dumps({'person': person})
result = crous.loads(binary)
```

## Advanced Patterns

### Multiple Serializers

```python
import crous
from datetime import datetime, date
from decimal import Decimal
import uuid

# Register multiple types
crous.register_serializer(
    datetime,
    lambda x: x.isoformat() if isinstance(x, datetime) else None
)
crous.register_serializer(
    date,
    lambda x: x.isoformat() if isinstance(x, date) else None
)
crous.register_serializer(
    Decimal,
    lambda x: str(x) if isinstance(x, Decimal) else None
)
crous.register_serializer(
    uuid.UUID,
    lambda x: str(x) if isinstance(x, uuid.UUID) else None
)

# Use all together
data = {
    'created': datetime.now(),
    'due': date.today(),
    'amount': Decimal('99.99'),
    'id': uuid.uuid4(),
}
binary = crous.dumps(data)
```

### Conditional Serialization

```python
import crous

class CustomType:
    def __init__(self, value, exportable=True):
        self.value = value
        self.exportable = exportable

def custom_serializer(obj):
    if isinstance(obj, CustomType):
        if obj.exportable:
            return {'value': obj.value}
        else:
            raise TypeError("This object is not exportable")
    raise TypeError()

crous.register_serializer(CustomType, custom_serializer)

# Exportable - works
data1 = {'obj': CustomType(42, exportable=True)}
binary1 = crous.dumps(data1)

# Not exportable - fails
try:
    data2 = {'obj': CustomType(42, exportable=False)}
    binary2 = crous.dumps(data2)
except crous.CrousEncodeError:
    print("Object not exportable")
```

### Chained Serializers

```python
import crous

class SerializerChain:
    def __init__(self):
        self.handlers = []
    
    def add_handler(self, type_, handler):
        self.handlers.append((type_, handler))
        crous.register_serializer(type_, self._chain_handler)
    
    def _chain_handler(self, obj):
        for type_, handler in self.handlers:
            if isinstance(obj, type_):
                return handler(obj)
        raise TypeError(f"Cannot serialize {type(obj)}")

chain = SerializerChain()

# Add handlers
chain.add_handler(int, lambda x: x * 2)  # Example: double integers
chain.add_handler(str, lambda x: x.upper())  # Example: uppercase strings
```

## Error Handling

### Exception Handling

```python
import crous

def safe_serializer(obj):
    try:
        if isinstance(obj, MyType):
            return obj.to_dict()
    except Exception as e:
        raise TypeError(f"Serialization failed: {e}")
    raise TypeError()

crous.register_serializer(MyType, safe_serializer)

try:
    binary = crous.dumps({'obj': MyType()})
except crous.CrousEncodeError as e:
    print(f"Encoding error: {e}")
```

### Validation

```python
import crous

def validated_serializer(obj):
    if isinstance(obj, MyType):
        # Validate before serializing
        if not obj.is_valid():
            raise TypeError("Object is invalid")
        return obj.to_dict()
    raise TypeError()

crous.register_serializer(MyType, validated_serializer)
```

## Performance Considerations

### Avoid Expensive Operations

```python
import crous

# Bad: Expensive computation in serializer
def slow_serializer(obj):
    if isinstance(obj, MyType):
        # Avoid heavy computation
        expensive_result = expensive_computation(obj.data)
        return expensive_result
    raise TypeError()

# Better: Pre-compute before serialization
def fast_serializer(obj):
    if isinstance(obj, MyType):
        # Use already-computed data
        return obj.cached_result
    raise TypeError()
```

### Reuse Serializers

```python
import crous
from datetime import datetime

# Register once
def datetime_serializer(obj):
    if isinstance(obj, datetime):
        return obj.isoformat()
    raise TypeError()

crous.register_serializer(datetime, datetime_serializer)

# Reuse many times
for i in range(1000):
    data = {'timestamp': datetime.now()}
    binary = crous.dumps(data)
```

## Cleanup and Resource Management

### Context Manager

```python
from contextlib import contextmanager
import crous
from datetime import datetime

@contextmanager
def serializer_scope(type_, handler):
    """Context manager for automatic cleanup."""
    crous.register_serializer(type_, handler)
    try:
        yield
    finally:
        crous.unregister_serializer(type_)

# Usage
with serializer_scope(datetime, lambda x: x.isoformat() if isinstance(x, datetime) else None):
    data = {'now': datetime.now()}
    binary = crous.dumps(data)
# Serializer automatically unregistered
```

### Multiple Serializers Context

```python
from contextlib import contextmanager
import crous
from datetime import datetime
from decimal import Decimal

@contextmanager
def serializers(**kwargs):
    """Register multiple serializers."""
    for type_, handler in kwargs.items():
        crous.register_serializer(type_, handler)
    try:
        yield
    finally:
        for type_ in kwargs:
            crous.unregister_serializer(type_)

# Usage
with serializers(
    datetime=lambda x: x.isoformat() if isinstance(x, datetime) else None,
    Decimal=lambda x: str(x) if isinstance(x, Decimal) else None,
):
    data = {'created': datetime.now(), 'price': Decimal('19.99')}
    binary = crous.dumps(data)
```

## Testing

### Testing Custom Serializers

```python
import crous
import pytest
from datetime import datetime

def test_datetime_serializer():
    # Register
    crous.register_serializer(
        datetime,
        lambda x: x.isoformat() if isinstance(x, datetime) else None
    )
    
    try:
        # Test serialization
        dt = datetime(2023, 12, 25, 10, 30)
        data = {'created': dt}
        binary = crous.dumps(data)
        
        # Test deserialization
        result = crous.loads(binary)
        assert result['created'] == '2023-12-25T10:30:00'
        
        # Verify it's a string after load
        assert isinstance(result['created'], str)
    finally:
        crous.unregister_serializer(datetime)
```

## Troubleshooting

### Serializer Not Called

```python
import crous

class MyType:
    pass

# Wrong: Registering but forgetting to use it
crous.register_serializer(MyType, lambda x: {'data': x.data})

# Check if registered
data = MyType()
try:
    crous.dumps({'obj': data})
except crous.CrousEncodeError as e:
    print(f"Error: {e}")

# Debug: Check what's in the object
print(type(data))
print(isinstance(data, MyType))
```

### Circular References

```python
import crous

class Node:
    def __init__(self, value):
        self.value = value
        self.next = None

# Create circular reference
node1 = Node(1)
node2 = Node(2)
node1.next = node2
node2.next = node1  # Circular!

# Serializer must handle this
def node_serializer(obj):
    if isinstance(obj, Node):
        # Avoid infinite recursion
        return {'value': obj.value}  # Don't serialize 'next'
    raise TypeError()

crous.register_serializer(Node, node_serializer)

data = {'node': node1}
binary = crous.dumps(data)  # Works now
```

---

See [User Guide](user-guide.md) for more examples and [API Reference](../api/reference.md) for complete API documentation.
