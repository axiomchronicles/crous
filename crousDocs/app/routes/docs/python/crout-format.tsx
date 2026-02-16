import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/crout-format";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "CROUT Text Format - Human-Readable Debug Format - Crous" },
    { name: "description", content: "Learn CROUT text format for debugging binary data. Human-readable syntax, Python API, conversion tools, and inspection utilities." },
    { name: "keywords", content: "crout format, text format, human readable binary, debug format, data inspection, binary to text" },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/python/crout-format" },
  ];
}

export default function CroutFormat() {
  return (
    <>
      <h1>CROUT Format</h1>
      <p>
        CROUT (Crous Text) is a human-readable text serialization format. It's designed
        for debugging, inspection, and scenarios where readability matters more than size.
        CROUT features a token compression system that replaces repeated dictionary keys
        with single characters.
      </p>

      <h2>Basic Usage</h2>
      <CodeBlock
        filename="crout_basic.py"
        code={`import crous

data = {"name": "Alice", "age": 30, "active": True}

# Encode to CROUT text
text = crous.to_crout(data)
print(text)
# Output:
# CROUT1
# {s5:Alice , i30 , T}

# Decode from CROUT text
result = crous.from_crout(text)
assert result == data`}
      />

      <h2>Value Syntax</h2>
      <p>
        Every value in CROUT has a type prefix that indicates how to parse it:
      </p>
      <table>
        <thead>
          <tr>
            <th>Prefix</th>
            <th>Type</th>
            <th>Example</th>
            <th>Description</th>
          </tr>
        </thead>
        <tbody>
          <tr><td><code>N</code></td><td>Null</td><td><code>N</code></td><td>Python <code>None</code></td></tr>
          <tr><td><code>T</code></td><td>True</td><td><code>T</code></td><td>Boolean true</td></tr>
          <tr><td><code>F</code></td><td>False</td><td><code>F</code></td><td>Boolean false</td></tr>
          <tr><td><code>i</code></td><td>Integer</td><td><code>i42</code>, <code>i-7</code></td><td>64-bit signed integer</td></tr>
          <tr><td><code>f</code></td><td>Float</td><td><code>f3.14</code></td><td>IEEE 754 double</td></tr>
          <tr><td><code>s</code></td><td>String</td><td><code>s5:hello</code></td><td>Length-prefixed, binary-safe</td></tr>
          <tr><td><code>b</code></td><td>Bytes</td><td><code>b4:deadbeef</code></td><td>Length-prefixed, hex-encoded</td></tr>
          <tr><td><code>#</code></td><td>Tagged</td><td><code>#90:[i1,i2]</code></td><td>Tagged value with numeric tag</td></tr>
          <tr><td><code>{"{}"}</code></td><td>Dict</td><td><code>{"{s3:key:i42}"}</code></td><td>Key-value mapping</td></tr>
          <tr><td><code>[]</code></td><td>List</td><td><code>[i1,i2,i3]</code></td><td>Ordered sequence</td></tr>
          <tr><td><code>()</code></td><td>Tuple</td><td><code>(i1,i2,i3)</code></td><td>Immutable sequence</td></tr>
        </tbody>
      </table>

      <h2>Token Compression</h2>
      <p>
        CROUT features a token table that replaces frequently-used dictionary keys with
        single-character tokens. This significantly reduces size for data with repeated keys.
      </p>
      <CodeBlock
        filename="token_compression.py"
        code={`import crous

# Data with repeated keys
data = [
    {"name": "Alice", "age": 30},
    {"name": "Bob", "age": 25},
    {"name": "Charlie", "age": 35},
]

text = crous.to_crout(data)
print(text)
# Output:
# CROUT1
# @ a=name
# @ c=age
# [{a:s5:Alice , c:i30} , {a:s3:Bob , c:i25} , {a:s7:Charlie , c:i35}]

# "name" → "a", "age" → "c" (single-character tokens)`}
      />

      <Callout type="info" title="Token Assignment">
        Tokens are assigned from a safe alphabet that avoids type prefixes (<code>s</code>, <code>i</code>, <code>f</code>,
        <code>b</code>, <code>N</code>, <code>T</code>, <code>F</code>). Keys appearing ≥ 2 times get tokens,
        sorted by frequency (most frequent first). Maximum 64 tokens.
      </Callout>

      <h2>Special Float Values</h2>
      <CodeBlock
        code={`# Special float values
crous.to_crout(float('inf'))      # "finf"
crous.to_crout(float('-inf'))     # "f-inf"
crous.to_crout(float('nan'))      # "fnan"`}
      />

      <h2>CROUT ↔ FLUX Conversion</h2>
      <p>
        Crous provides direct conversion between CROUT text and FLUX binary without
        going through Python objects:
      </p>
      <CodeBlock
        filename="conversion.py"
        code={`import crous

data = {"name": "Alice", "scores": [98, 95, 100]}

# Python → CROUT text
crout_text = crous.to_crout(data)

# CROUT text → FLUX binary (direct, no Python intermediary)
flux_binary = crous.crout_to_flux(crout_text)

# FLUX binary → CROUT text (direct)
crout_back = crous.flux_to_crout(flux_binary)

# All representations are equivalent
assert crous.from_crout(crout_text) == crous.loads(flux_binary)`}
      />

      <h2>CROUT Format Header</h2>
      <p>
        Every CROUT document starts with the magic string <code>CROUT1</code> followed by
        optional token definitions:
      </p>
      <CodeBlock
        code={`CROUT1                     ← magic + version
@ a=name                   ← token "a" maps to key "name"
@ c=age                    ← token "c" maps to key "age"
[{a:s5:Alice , c:i30}]    ← data using tokens`}
      />

      <h2>String Encoding</h2>
      <p>
        Strings in CROUT use a length-prefix encoding: <code>s{"{length}"}:{"{data}"}</code>.
        This is binary-safe — strings can contain any bytes including null bytes, newlines,
        and other special characters.
      </p>
      <CodeBlock
        code={`# String encoding examples:
# s0:        → empty string ""
# s5:hello   → "hello"
# s11:hello world → "hello world"
# s3:a\\nb    → "a\\nb" (newline in string, length includes it)`}
      />

      <h2>Bytes Encoding</h2>
      <p>
        Bytes use hex encoding with a length prefix: <code>b{"{length}"}:{"{hex}"}</code>.
        The length is the number of decoded bytes (not the hex string length).
      </p>
      <CodeBlock
        code={`# Bytes encoding examples:
# b0:           → b""
# b3:414243     → b"ABC"  (hex for 0x41, 0x42, 0x43)
# b4:deadbeef   → b"\\xde\\xad\\xbe\\xef"`}
      />
    </>
  );
}
