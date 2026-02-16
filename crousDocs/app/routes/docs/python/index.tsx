import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/index";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Python SDK Documentation - Crous Binary Serialization" },
    { name: "description", content: "Complete Python SDK documentation for Crous. High-performance binary serialization library with C core. Installation, API reference, and guides." },
    { name: "keywords", content: "python sdk, crous python, python documentation, binary serialization python, python api docs, python library guide" },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/python" },
  ];
}

export default function PythonIntro() {
  return (
    <>
      <h1>Introduction</h1>
      <p>
        <strong>Crous</strong> is a high-performance binary serialization library implemented as a C extension for Python.
        It provides compact, type-preserving serialization that is significantly smaller and faster than JSON for most workloads.
      </p>

      <Callout type="info" title="Version 2.0.0">
        Crous v2.0.0 introduces the FLUX binary format as the default encoding, CROUT text format for human-readable output,
        and comprehensive custom serializer support.
      </Callout>

      <h2>Key Features</h2>
      <ul>
        <li><strong>Blazing fast</strong> — C extension core with zero-copy optimizations</li>
        <li><strong>Ultra compact</strong> — Zigzag varint encoding, small-integer optimization, up to 75% smaller than JSON</li>
        <li><strong>Type preserving</strong> — Native support for <code>int</code>, <code>float</code>, <code>bytes</code>, <code>tuple</code>, <code>set</code>, <code>frozenset</code>, and tagged values</li>
        <li><strong>Three formats</strong> — FLUX binary (default), CROUT text (human-readable), and legacy CROUS binary</li>
        <li><strong>Streaming</strong> — File and stream-based I/O with <code>dump</code>/<code>load</code></li>
        <li><strong>Custom serializers</strong> — Register handlers for any Python type with MRO-aware lookup</li>
        <li><strong>Thread safe</strong> — Lock-protected custom serializer registry for concurrent use</li>
        <li><strong>Cross-platform</strong> — Compatible with Node.js SDK for polyglot applications</li>
      </ul>

      <h2>Quick Example</h2>
      <CodeBlock
        filename="example.py"
        language="python"
        code={`import crous

# Serialize Python data to compact binary
data = {
    "name": "Alice",
    "scores": [98, 95, 100],
    "active": True,
    "tags": {"python", "developer"},
}

# Encode to FLUX binary
binary = crous.dumps(data)
print(f"Encoded: {len(binary)} bytes")

# Decode back to Python
result = crous.loads(binary)
assert result == data
print(f"Decoded: {result}")`}
      />

      <h2>Supported Types</h2>

      <table>
        <thead>
          <tr>
            <th>Python Type</th>
            <th>Crous Type</th>
            <th>Round-trip</th>
          </tr>
        </thead>
        <tbody>
          <tr><td><code>None</code></td><td>NULL</td><td>✅</td></tr>
          <tr><td><code>bool</code></td><td>BOOL</td><td>✅</td></tr>
          <tr><td><code>int</code></td><td>INT (64-bit signed)</td><td>✅</td></tr>
          <tr><td><code>float</code></td><td>FLOAT (64-bit IEEE 754)</td><td>✅</td></tr>
          <tr><td><code>str</code></td><td>STRING (UTF-8)</td><td>✅</td></tr>
          <tr><td><code>bytes</code> / <code>bytearray</code></td><td>BYTES</td><td>✅</td></tr>
          <tr><td><code>list</code></td><td>LIST</td><td>✅</td></tr>
          <tr><td><code>tuple</code></td><td>TUPLE</td><td>✅</td></tr>
          <tr><td><code>dict</code></td><td>DICT (string keys only)</td><td>✅</td></tr>
          <tr><td><code>set</code></td><td>TAGGED(90)</td><td>✅</td></tr>
          <tr><td><code>frozenset</code></td><td>TAGGED(91)</td><td>✅</td></tr>
        </tbody>
      </table>

      <h2>Architecture</h2>
      <p>
        Crous uses a three-layer architecture:
      </p>
      <ol>
        <li><strong>Python Wrapper</strong> (<code>__init__.py</code>) — Enhanced API with file path support and error handling</li>
        <li><strong>C Extension</strong> (<code>pycrous.c</code>) — N-API bindings that convert between Python objects and the C value tree</li>
        <li><strong>Core C Library</strong> — Arena allocator, value tree, FLUX/CROUT/CROUS encoders, lexer, and parser</li>
      </ol>
      <p>
        Data flows through an intermediate <code>crous_value</code> tree that decouples Python types from the wire format,
        enabling format-agnostic encoding and easy addition of new formats.
      </p>

      <h2>Wire Format Versions</h2>
      <table>
        <thead>
          <tr>
            <th>Format</th>
            <th>Magic Bytes</th>
            <th>Version</th>
            <th>Default</th>
          </tr>
        </thead>
        <tbody>
          <tr>
            <td>FLUX Binary</td>
            <td><code>FLUX</code></td>
            <td>1</td>
            <td>✅ (v2.0.0+)</td>
          </tr>
          <tr>
            <td>CROUT Text</td>
            <td><code>CROUT1</code></td>
            <td>1</td>
            <td>—</td>
          </tr>
          <tr>
            <td>CROUS Binary (Legacy)</td>
            <td><code>CROU</code></td>
            <td>2</td>
            <td>— (auto-detected on decode)</td>
          </tr>
        </tbody>
      </table>
    </>
  );
}
