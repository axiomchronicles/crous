import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/serialization";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Python Serialization Guide - Encoding & Decoding - Crous" },
    { 
      name: "description", 
      content: "Learn Python binary serialization with Crous. Encode and decode Python objects with encode(), decode(), FLUX and CROUT formats. Performance optimization tips." 
    },
    { 
      name: "keywords", 
      content: "python serialization, binary encoding python, decode python objects, flux format, crout format, python data encoding" 
    },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/python/serialization" },
  ];
}

export default function Serialization() {
  return (
    <>
      <h1>Serialization</h1>
      <p>
        Crous provides four core functions for serialization: <code>dumps</code>/<code>loads</code> for
        in-memory operations and <code>dump</code>/<code>load</code> for file/stream I/O.
      </p>

      <h2>In-Memory Serialization</h2>

      <h3>dumps(obj, default=None)</h3>
      <p>
        Serialize a Python object to FLUX binary bytes.
      </p>
      <CodeBlock
        filename="dumps_example.py"
        code={`import crous

# Serialize to bytes
data = {"key": "value", "numbers": [1, 2, 3]}
binary = crous.dumps(data)
print(type(binary))  # <class 'bytes'>
print(len(binary))   # compact binary representation`}
      />

      <h3>loads(data, object_hook=None)</h3>
      <p>
        Deserialize FLUX binary bytes back to a Python object. Automatically detects
        FLUX vs legacy CROUS format from the magic bytes.
      </p>
      <CodeBlock
        filename="loads_example.py"
        code={`import crous

binary = crous.dumps({"name": "Alice", "age": 30})

# Deserialize from bytes
result = crous.loads(binary)
print(result)  # {'name': 'Alice', 'age': 30}`}
      />

      <h2>File I/O</h2>

      <h3>dump(obj, file, default=None)</h3>
      <p>
        Serialize a Python object and write it to a file. Accepts both file paths (strings)
        and file-like objects.
      </p>
      <CodeBlock
        filename="dump_example.py"
        code={`import crous

data = {"users": [{"name": "Alice"}, {"name": "Bob"}]}

# Using a file path
crous.dump(data, "users.crous")

# Using a file object
with open("users.crous", "wb") as f:
    crous.dump(data, f)`}
      />

      <h3>load(file, object_hook=None)</h3>
      <p>
        Read and deserialize from a file. Accepts both file paths and file-like objects.
      </p>
      <CodeBlock
        filename="load_example.py"
        code={`import crous

# From file path
data = crous.load("users.crous")

# From file object
with open("users.crous", "rb") as f:
    data = crous.load(f)

print(data)  # {'users': [{'name': 'Alice'}, {'name': 'Bob'}]}`}
      />

      <h2>The default Parameter</h2>
      <p>
        The <code>default</code> parameter lets you handle types that Crous doesn't natively support.
        It receives the unsupported object and should return a serializable value.
      </p>
      <CodeBlock
        filename="default_example.py"
        code={`import crous
from datetime import datetime, date
from decimal import Decimal

def default_handler(obj):
    if isinstance(obj, datetime):
        return obj.isoformat()
    if isinstance(obj, date):
        return obj.isoformat()
    if isinstance(obj, Decimal):
        return float(obj)
    raise TypeError(f"Cannot serialize {type(obj)}")

data = {
    "timestamp": datetime.now(),
    "price": Decimal("19.99"),
}

binary = crous.dumps(data, default=default_handler)
result = crous.loads(binary)
print(result)  # {'timestamp': '2024-...', 'price': 19.99}`}
      />

      <Callout type="warning" title="One-way with default">
        The <code>default</code> parameter converts objects to basic types, so the original type information
        is lost on decode. For round-trip type preservation, use{" "}
        <a href="/docs/python/custom-types">custom serializers</a> instead.
      </Callout>

      <h2>The object_hook Parameter</h2>
      <p>
        The <code>object_hook</code> parameter is called for every decoded dictionary, letting you
        transform the result.
      </p>
      <CodeBlock
        filename="object_hook_example.py"
        code={`import crous

def hook(d):
    if "type" in d and d["type"] == "user":
        return User(d["name"], d["age"])
    return d

class User:
    def __init__(self, name, age):
        self.name = name
        self.age = age

data = {"type": "user", "name": "Alice", "age": 30}
binary = crous.dumps(data)

user = crous.loads(binary, object_hook=hook)
print(f"{user.name}, {user.age}")  # Alice, 30`}
      />

      <h2>Encoder & Decoder Classes</h2>
      <p>
        For repeated operations with the same options, use the class-based API:
      </p>
      <CodeBlock
        filename="encoder_decoder.py"
        code={`import crous

# Create reusable encoder with default handler
encoder = crous.CrousEncoder(default=my_default_handler)
binary = encoder.encode(data)

# Create reusable decoder with object hook
decoder = crous.CrousDecoder(object_hook=my_hook)
result = decoder.decode(binary)`}
      />

      <h2>FLUX Binary Format</h2>
      <p>
        As of v2.0.0, Crous uses the FLUX binary format by default. FLUX provides:
      </p>
      <ul>
        <li><strong>Zigzag varint integers</strong> — small integers encoded in 1-2 bytes instead of 8</li>
        <li><strong>Small-int optimization</strong> — integers 0–24 use a single byte</li>
        <li><strong>Big-endian floats</strong> — IEEE 754 double precision</li>
        <li><strong>Varint lengths</strong> — container sizes use 7-bit LEB128 encoding</li>
      </ul>

      <h3>FLUX Header</h3>
      <CodeBlock
        code={`# FLUX binary header structure:
# [F][L][U][X] [version: 1 byte] [flags: 1 byte]
#   4 bytes      0x01               0x00

# Total header: 6 bytes`}
      />

      <Callout type="info" title="Auto-Detection">
        <code>crous.loads()</code> automatically detects the format from magic bytes.
        FLUX data starts with <code>b"FLUX"</code>, legacy CROUS data starts with <code>b"CROU"</code>.
        Both formats are transparently decoded.
      </Callout>
    </>
  );
}
