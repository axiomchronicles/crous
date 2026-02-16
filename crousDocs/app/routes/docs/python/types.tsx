import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/types";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Python Type System - Supported Data Types - Crous" },
    { 
      name: "description", 
      content: "Complete guide to Python types in Crous: None, bool, int, float, str, bytes, list, tuple, dict, and tagged types. Encoding rules and size optimization." 
    },
    { 
      name: "keywords", 
      content: "python data types, crous types, python serialization types, binary encoding types, python type mapping, data type support" 
    },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/python/types" },
  ];
}

export default function Types() {
  return (
    <>
      <h1>Type System</h1>
      <p>
        Crous maps Python types to an internal <code>crous_value</code> tree, which is then
        encoded to one of the wire formats. This page documents every supported type,
        its encoding behavior, and edge cases.
      </p>

      <h2>Core Types</h2>

      <h3>Null</h3>
      <p>Python's <code>None</code> maps to the CROUS NULL type (tag <code>0x00</code>).</p>
      <CodeBlock code={`crous.dumps(None)     # 7 bytes (6-byte header + 1 tag byte)
crous.loads(binary)   # None`} />

      <h3>Boolean</h3>
      <p>
        Booleans are encoded as single-byte tags: <code>0x01</code> for <code>False</code>,
        <code>0x02</code> for <code>True</code>. Since Python's <code>bool</code> is a subclass of <code>int</code>,
        Crous checks for <code>bool</code> first to preserve the type.
      </p>
      <CodeBlock code={`crous.dumps(True)    # tag 0x02
crous.dumps(False)   # tag 0x01

# Type preservation
assert type(crous.loads(crous.dumps(True))) is bool   # not int!`} />

      <h3>Integer</h3>
      <p>
        Integers are stored as 64-bit signed values using zigzag encoding in FLUX format.
        Small integers (0–24) are encoded in a single byte.
      </p>
      <CodeBlock code={`# Small int optimization (single byte!)
crous.dumps(0)     # tag 0x10 → 1 byte
crous.dumps(24)    # tag 0x28 → 1 byte
crous.dumps(-1)    # tag 0x29 → 1 byte
crous.dumps(-32)   # tag 0x48 → 1 byte

# Larger integers use zigzag varint
crous.dumps(1000)  # tag 0x03 + zigzag varint

# Full 64-bit range
crous.dumps(2**63 - 1)   # max int64
crous.dumps(-(2**63))    # min int64`} />

      <Callout type="warning" title="Integer Overflow">
        Integers outside the 64-bit signed range (-2⁶³ to 2⁶³-1) will raise a <code>CrousEncodeError</code>.
        Use <code>default</code> or a custom serializer to handle <code>int</code> values larger than 64 bits.
      </Callout>

      <h3>Float</h3>
      <p>
        Floating-point numbers are stored as 8-byte IEEE 754 doubles. Special values
        (<code>NaN</code>, <code>Infinity</code>, <code>-Infinity</code>) are preserved.
      </p>
      <CodeBlock code={`import math

crous.dumps(3.14)             # IEEE 754 double
crous.dumps(float('inf'))     # preserved
crous.dumps(float('-inf'))    # preserved
crous.dumps(float('nan'))     # preserved

# NaN comparison caveat
val = crous.loads(crous.dumps(float('nan')))
assert math.isnan(val)  # True (but val != val)`} />

      <h3>String</h3>
      <p>
        Strings are stored as UTF-8 with a varint length prefix. The encoder validates
        UTF-8 encoding and rejects invalid sequences.
      </p>
      <CodeBlock code={`crous.dumps("")           # empty string (length 0)
crous.dumps("hello")      # varint(5) + "hello"
crous.dumps("こんにちは")   # varint(15) + UTF-8 bytes

# Full Unicode support
crous.dumps("🎉🐍💚")     # emoji support
crous.dumps("مرحبا")       # Arabic
crous.dumps("Привет")      # Cyrillic`} />

      <h3>Bytes</h3>
      <p>
        Both <code>bytes</code> and <code>bytearray</code> are stored as raw byte sequences
        with a varint length prefix.
      </p>
      <CodeBlock code={`crous.dumps(b"\\x00\\x01\\x02")      # raw bytes
crous.dumps(bytearray([1, 2, 3]))  # also works

# Round-trip always returns bytes (not bytearray)
result = crous.loads(crous.dumps(bytearray([1, 2, 3])))
assert type(result) is bytes`} />

      <h2>Container Types</h2>

      <h3>List</h3>
      <p>Lists are encoded with a varint count followed by each element.</p>
      <CodeBlock code={`crous.dumps([1, 2, 3])          # varint(3) + elements
crous.dumps([])                 # empty list (count 0)
crous.dumps([1, "two", 3.0])   # mixed types OK
crous.dumps([[1, 2], [3, 4]])  # nested lists`} />

      <h3>Tuple</h3>
      <p>
        Tuples have their own type tag (<code>TUPLE</code>), distinct from lists.
        Type is preserved on round-trip.
      </p>
      <CodeBlock code={`# Tuples are NOT lists!
data_list = [1, 2, 3]
data_tuple = (1, 2, 3)

result_list = crous.loads(crous.dumps(data_list))
result_tuple = crous.loads(crous.dumps(data_tuple))

assert type(result_list) is list     # ✓
assert type(result_tuple) is tuple   # ✓ (preserved!)`} />

      <h3>Dictionary</h3>
      <p>
        Dictionaries are encoded with a varint count, then each key-value pair.
        <strong> Keys must be strings.</strong>
      </p>
      <CodeBlock code={`crous.dumps({"a": 1, "b": 2})  # varint(2) + pairs
crous.dumps({})                 # empty dict

# Keys MUST be strings
try:
    crous.dumps({1: "value"})
except crous.CrousEncodeError:
    print("Integer keys not supported!")`} />

      <Callout type="danger" title="String Keys Only">
        Crous dictionaries only support string keys. Attempting to serialize a dict with
        non-string keys (int, tuple, etc.) will raise a <code>CrousEncodeError</code>.
      </Callout>

      <h2>Extended Types</h2>

      <h3>Set</h3>
      <p>
        Sets are encoded as tagged values with tag 90, wrapping a list of the set's elements.
        On decode, the list is automatically reconstructed as a <code>set</code>.
      </p>
      <CodeBlock code={`data = {1, 2, 3, "four"}
binary = crous.dumps(data)
result = crous.loads(binary)

assert type(result) is set   # ✓
assert result == data         # ✓`} />

      <h3>Frozenset</h3>
      <p>
        Frozensets use tag 91 and are reconstructed as <code>frozenset</code> on decode.
      </p>
      <CodeBlock code={`data = frozenset([1, 2, 3])
result = crous.loads(crous.dumps(data))

assert type(result) is frozenset  # ✓`} />

      <h3>Tagged Values</h3>
      <p>
        Tagged values wrap any value with a numeric tag. Tags 90 and 91 are reserved for
        set/frozenset. Tags 100+ are used by the custom serializer registry.
      </p>

      <h2>Built-in Tag Assignments</h2>
      <table>
        <thead>
          <tr>
            <th>Tag</th>
            <th>Type</th>
            <th>Description</th>
          </tr>
        </thead>
        <tbody>
          <tr><td>80</td><td><code>datetime</code></td><td>Named tag (parser only)</td></tr>
          <tr><td>81</td><td><code>date</code></td><td>Named tag (parser only)</td></tr>
          <tr><td>82</td><td><code>time</code></td><td>Named tag (parser only)</td></tr>
          <tr><td>83</td><td><code>timedelta</code></td><td>Named tag (parser only)</td></tr>
          <tr><td>84</td><td><code>decimal</code></td><td>Named tag (parser only)</td></tr>
          <tr><td>90</td><td><code>set</code></td><td>Built-in set encoding</td></tr>
          <tr><td>91</td><td><code>frozenset</code></td><td>Built-in frozenset encoding</td></tr>
          <tr><td>92</td><td><code>complex</code></td><td>Named tag (parser only)</td></tr>
          <tr><td>100+</td><td>Custom</td><td>Auto-assigned by <code>register_serializer</code></td></tr>
        </tbody>
      </table>

      <h2>Type Encoding Summary</h2>
      <table>
        <thead>
          <tr>
            <th>Tag Byte</th>
            <th>Type</th>
            <th>Encoding</th>
          </tr>
        </thead>
        <tbody>
          <tr><td><code>0x00</code></td><td>NULL</td><td>1 byte</td></tr>
          <tr><td><code>0x01</code></td><td>FALSE</td><td>1 byte</td></tr>
          <tr><td><code>0x02</code></td><td>TRUE</td><td>1 byte</td></tr>
          <tr><td><code>0x03</code></td><td>INT</td><td>1 + zigzag varint</td></tr>
          <tr><td><code>0x04</code></td><td>FLOAT</td><td>1 + 8 bytes (big-endian)</td></tr>
          <tr><td><code>0x05</code></td><td>STRING</td><td>1 + varint(len) + data</td></tr>
          <tr><td><code>0x06</code></td><td>BYTES</td><td>1 + varint(len) + data</td></tr>
          <tr><td><code>0x07</code></td><td>LIST</td><td>1 + varint(count) + elements</td></tr>
          <tr><td><code>0x08</code></td><td>DICT</td><td>1 + varint(count) + pairs</td></tr>
          <tr><td><code>0x09</code></td><td>TAGGED</td><td>1 + varint(tag) + value</td></tr>
          <tr><td><code>0x0A</code></td><td>TUPLE</td><td>1 + varint(count) + elements</td></tr>
          <tr><td><code>0x10–0x28</code></td><td>POSINT</td><td>1 byte (integers 0–24)</td></tr>
          <tr><td><code>0x29–0x48</code></td><td>NEGINT</td><td>1 byte (integers -1 to -32)</td></tr>
        </tbody>
      </table>

      <h2>Nesting Limits</h2>
      <p>
        Crous enforces a maximum nesting depth of <strong>256 levels</strong> to prevent stack overflow.
        Attempting to encode deeper structures raises <code>CrousEncodeError</code>.
      </p>

      <h2>Size Limits</h2>
      <p>
        Individual strings and byte sequences are limited to <strong>64 MB</strong> (67,108,864 bytes).
        This prevents memory exhaustion from malicious or corrupted data.
      </p>
    </>
  );
}
