import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/getting-started";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Python Getting Started Guide - Crous Binary Serialization" },
    { 
      name: "description", 
      content: "Install Crous for Python with pip. Quick start guide for binary serialization, encoding and decoding Python objects with high performance C core." 
    },
    { 
      name: "keywords", 
      content: "python binary serialization, crous python, pip install crous, python encoding tutorial, binary data format python, python serialization library" 
    },
    { property: "og:title", content: "Python Getting Started - Crous" },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/python/getting-started" },
  ];
}

export default function GettingStarted() {
  return (
    <>
      <h1>Getting Started</h1>
      <p>
        This guide walks you through installing Crous and running your first serialization.
      </p>

      <h2>Requirements</h2>
      <ul>
        <li>Python 3.6 or later</li>
        <li>A C compiler (GCC, Clang, or MSVC) — required to build the C extension</li>
        <li>pip (Python package manager)</li>
      </ul>

      <h2>Installation</h2>
      <h3>From PyPI</h3>
      <CodeBlock
        code="pip install crous"
        language="bash"
      />

      <h3>From Source</h3>
      <CodeBlock
        code={`git clone https://github.com/axiomchronicles/crous.git
cd crous
pip install -e .`}
        language="bash"
      />

      <Callout type="info" title="Build from Source">
        When installing from source, the C extension is compiled automatically by <code>setup.py</code>.
        The build uses <code>-O3 -Wall -Wextra -std=c99</code> optimization flags.
      </Callout>

      <h2>Verify Installation</h2>
      <CodeBlock
        filename="verify.py"
        code={`import crous

# Check version
print(f"Crous version: {crous.__version__}")
# Output: Crous version: 2.0.0

# Quick round-trip test
data = {"hello": "world", "numbers": [1, 2, 3]}
encoded = crous.dumps(data)
decoded = crous.loads(encoded)
assert decoded == data
print("✓ Crous is working correctly!")`}
      />

      <h2>Your First Serialization</h2>
      <p>
        Crous provides a simple API similar to Python's <code>json</code> module:
      </p>

      <CodeBlock
        filename="first_example.py"
        code={`import crous

# Create some data
user = {
    "name": "Alice",
    "age": 30,
    "email": "alice@example.com",
    "scores": [98.5, 95.0, 100.0],
    "verified": True,
    "avatar": b"\\x89PNG\\r\\n...",  # binary data!
    "tags": {"admin", "developer"},   # sets too!
}

# Serialize to binary
binary_data = crous.dumps(user)
print(f"Serialized to {len(binary_data)} bytes")

# Save to file
crous.dump(user, "user.crous")

# Load from file
loaded = crous.load("user.crous")
assert loaded == user
print("✓ File round-trip successful!")`}
      />

      <h2>Comparison with JSON</h2>
      <table>
        <thead>
          <tr>
            <th>Feature</th>
            <th>Crous</th>
            <th>JSON</th>
          </tr>
        </thead>
        <tbody>
          <tr>
            <td>Binary data</td>
            <td>✅ Native <code>bytes</code></td>
            <td>❌ Base64 encoding</td>
          </tr>
          <tr>
            <td>Tuples</td>
            <td>✅ Preserved as tuples</td>
            <td>❌ Becomes list</td>
          </tr>
          <tr>
            <td>Sets</td>
            <td>✅ Native set/frozenset</td>
            <td>❌ Not supported</td>
          </tr>
          <tr>
            <td>Integer precision</td>
            <td>✅ Full 64-bit signed</td>
            <td>⚠️ May lose precision</td>
          </tr>
          <tr>
            <td>Size</td>
            <td>✅ 40-75% smaller</td>
            <td>—</td>
          </tr>
          <tr>
            <td>Human readable</td>
            <td>✅ CROUT text format</td>
            <td>✅ Native</td>
          </tr>
        </tbody>
      </table>
    </>
  );
}
