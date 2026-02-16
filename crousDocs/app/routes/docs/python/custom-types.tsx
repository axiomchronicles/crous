import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/custom-types";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Python Custom Types - Extend Serialization - Crous" },
    { name: "description", content: "Register custom serializers and decoders for any Python type. Extend Crous with datetime, Decimal, dataclasses, and domain-specific types." },
    { name: "keywords", content: "python custom types, custom serializers, python type extension, dataclass serialization, datetime encoding, custom encoders" },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/python/custom-types" },
  ];
}

export default function CustomTypes() {
  return (
    <>
      <h1>Custom Types</h1>
      <p>
        Crous provides a registry system for custom type serialization. You can register
        serializers for any Python class and decoders to reconstruct them. Tags are
        auto-assigned starting from 100.
      </p>

      <h2>Registering a Serializer</h2>
      <p>
        Use <code>register_serializer(type, serializer_fn, tag=None)</code> to register a
        serializer function for a Python type. The serializer receives the object and must
        return a serializable value (dict, list, string, etc.).
      </p>
      <CodeBlock
        filename="register_serializer.py"
        code={`import crous
from datetime import datetime

# Define serializer function
def serialize_datetime(dt):
    return {
        "year": dt.year,
        "month": dt.month,
        "day": dt.day,
        "hour": dt.hour,
        "minute": dt.minute,
        "second": dt.second,
    }

# Register it
crous.register_serializer(datetime, serialize_datetime)

# Now datetime objects serialize automatically!
data = {"created": datetime(2024, 1, 15, 10, 30, 0)}
binary = crous.dumps(data)`}
      />

      <h2>Registering a Decoder</h2>
      <p>
        Use <code>register_decoder(tag, decoder_fn)</code> to register a decoder that
        reconstructs the custom type from the serialized data.
      </p>
      <CodeBlock
        filename="register_decoder.py"
        code={`import crous
from datetime import datetime

# Define decoder function
def decode_datetime(data):
    return datetime(
        data["year"], data["month"], data["day"],
        data["hour"], data["minute"], data["second"]
    )

# Register decoder for the same tag
# Tags are auto-assigned starting at 100
crous.register_decoder(100, decode_datetime)

# Full round-trip!
data = {"created": datetime(2024, 1, 15, 10, 30, 0)}
binary = crous.dumps(data)
result = crous.loads(binary)

assert isinstance(result["created"], datetime)  # ✓
assert result["created"] == data["created"]      # ✓`}
      />

      <Callout type="info" title="Tag Auto-Assignment">
        Tags are auto-assigned starting from 100. The first call to <code>register_serializer</code>
        gets tag 100, the second gets 101, and so on. You can also specify a tag explicitly.
      </Callout>

      <h2>Complete Example: Multiple Custom Types</h2>
      <CodeBlock
        filename="multiple_types.py"
        code={`import crous
from datetime import datetime, date
from decimal import Decimal
from uuid import UUID
from pathlib import Path

# --- Serializers ---
crous.register_serializer(datetime, lambda dt: dt.isoformat())
crous.register_serializer(date, lambda d: d.isoformat())
crous.register_serializer(Decimal, lambda d: str(d))
crous.register_serializer(UUID, lambda u: str(u))
crous.register_serializer(Path, lambda p: str(p))

# --- Decoders ---
crous.register_decoder(100, lambda s: datetime.fromisoformat(s))
crous.register_decoder(101, lambda s: date.fromisoformat(s))
crous.register_decoder(102, lambda s: Decimal(s))
crous.register_decoder(103, lambda s: UUID(s))
crous.register_decoder(104, lambda s: Path(s))

# Use it!
data = {
    "timestamp": datetime.now(),
    "birthday": date(1990, 5, 15),
    "price": Decimal("29.99"),
    "id": UUID("12345678-1234-5678-1234-567812345678"),
    "config": Path("/etc/config.json"),
}

binary = crous.dumps(data)
result = crous.loads(binary)

# All types preserved!
assert isinstance(result["timestamp"], datetime)
assert isinstance(result["birthday"], date)
assert isinstance(result["price"], Decimal)
assert isinstance(result["id"], UUID)
assert isinstance(result["config"], Path)`}
      />

      <h2>MRO-Aware Lookup</h2>
      <p>
        The serializer registry uses Python's <strong>Method Resolution Order (MRO)</strong> for
        type lookup. If no exact serializer is registered for a type, Crous walks the MRO
        chain to find the closest registered parent.
      </p>
      <CodeBlock
        filename="mro_example.py"
        code={`import crous

class Animal:
    def __init__(self, name):
        self.name = name

class Dog(Animal):
    def __init__(self, name, breed):
        super().__init__(name)
        self.breed = breed

# Register serializer for the base class
crous.register_serializer(Animal, lambda a: {"name": a.name})

# Dog inherits the Animal serializer via MRO!
dog = Dog("Rex", "Labrador")
binary = crous.dumps(dog)  # Uses Animal's serializer`}
      />

      <h2>Unregistering</h2>
      <p>
        Remove serializers and decoders when they're no longer needed:
      </p>
      <CodeBlock
        code={`# Remove serializer
crous.unregister_serializer(datetime)

# Remove decoder
crous.unregister_decoder(100)`}
      />

      <h2>Thread Safety</h2>
      <p>
        The custom serializer/decoder registry is protected by a thread lock. Multiple
        threads can safely:
      </p>
      <ul>
        <li>Register/unregister serializers concurrently</li>
        <li>Call <code>dumps</code>/<code>loads</code> concurrently</li>
        <li>Mix registration and serialization across threads</li>
      </ul>

      <Callout type="success" title="Thread Safe">
        All registry operations acquire a <code>PyThread_type_lock</code> before accessing
        the global serializer/decoder dictionaries. The core encode/decode functions are
        stateless and can run fully in parallel.
      </Callout>

      <h2>How It Works Internally</h2>
      <p>
        When encoding, Crous follows this flow for each object:
      </p>
      <ol>
        <li>Check if the type is a built-in type (None, bool, int, float, str, bytes, list, tuple, dict, set, frozenset)</li>
        <li>If not, check the custom serializer registry (with MRO fallback)</li>
        <li>If found, call the serializer, convert the result to a <code>crous_value</code>, wrap it in a <code>TAGGED</code> value with the assigned tag</li>
        <li>If not found, call the <code>default</code> function (if provided)</li>
        <li>If no default, raise <code>CrousEncodeError</code></li>
      </ol>
      <p>
        When decoding, for each <code>TAGGED</code> value:
      </p>
      <ol>
        <li>Tag 90 → reconstruct as <code>set</code></li>
        <li>Tag 91 → reconstruct as <code>frozenset</code></li>
        <li>Other tags → look up in custom decoder registry</li>
        <li>If decoder found, call it with the inner value</li>
        <li>If not found, return the inner value as-is</li>
      </ol>
    </>
  );
}
