import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/index";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Node.js SDK Documentation - Crous Binary Serialization" },
    { name: "description", content: "Complete Node.js SDK documentation for Crous. High-performance binary serialization with N-API native bindings. TypeScript support included." },
    { name: "keywords", content: "nodejs sdk, crous nodejs, javascript documentation, binary serialization nodejs, typescript api, napi native addon" },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/nodejs" },
  ];
}

export default function NodejsIntro() {
  return (
    <>
      <h1>Introduction</h1>
      <p>
        The <strong>Crous Node.js SDK</strong> provides high-performance binary serialization
        for Node.js applications via a native N-API addon. It produces wire-compatible output
        with the Python SDK, enabling seamless cross-language data exchange.
      </p>

      <Callout type="info" title="Version 2.0.0">
        The Node.js SDK v2.0.0 shares the same core C library as the Python SDK, ensuring
        identical binary output and full interoperability.
      </Callout>

      <h2>Key Features</h2>
      <ul>
        <li><strong>Native C addon</strong> — N-API bindings for maximum performance</li>
        <li><strong>ABI stable</strong> — No recompilation needed across Node.js versions</li>
        <li><strong>Cross-platform</strong> — Works on Linux, macOS, and Windows</li>
        <li><strong>TypeScript ready</strong> — Full type definitions included</li>
        <li><strong>Binary data</strong> — Native Buffer support (15x faster than JSON for binary)</li>
        <li><strong>Custom serializers</strong> — Register handlers for any JavaScript class</li>
        <li><strong>Set support</strong> — Native <code>Set</code> preservation</li>
        <li><strong>Python compatible</strong> — Identical wire format for polyglot apps</li>
      </ul>

      <h2>Quick Example</h2>
      <CodeBlock
        filename="example.js"
        language="javascript"
        code={`const crous = require('crous');

// Serialize JavaScript data to compact binary
const data = {
    name: 'Alice',
    scores: [98, 95, 100],
    active: true,
    avatar: Buffer.from([0x89, 0x50, 0x4E, 0x47]),
    tags: new Set(['admin', 'developer']),
};

// Encode to binary Buffer
const binary = crous.dumps(data);
console.log(\`Encoded: \${binary.length} bytes\`);

// Decode back to JavaScript
const result = crous.loads(binary);
console.log(result);
// { name: 'Alice', scores: [98, 95, 100], ... }`}
      />

      <h2>Architecture</h2>
      <p>
        The Node.js SDK uses a three-layer architecture:
      </p>
      <ol>
        <li><strong>JavaScript Wrapper</strong> (<code>index.js</code>) — Error classes, file I/O, CrousEncoder/CrousDecoder</li>
        <li><strong>N-API C Bindings</strong> (<code>crous_node.c</code>) — Converts JS ↔ crous_value, custom serializer registry</li>
        <li><strong>Core C Library</strong> (<code>crous_core/</code>) — Shared with Python SDK: binary codec, value tree, arena allocator</li>
      </ol>

      <h2>Type Mapping</h2>
      <table>
        <thead>
          <tr>
            <th>JavaScript</th>
            <th>Crous Binary</th>
            <th>Round-trip</th>
          </tr>
        </thead>
        <tbody>
          <tr><td><code>null</code> / <code>undefined</code></td><td>NULL</td><td>→ <code>null</code></td></tr>
          <tr><td><code>boolean</code></td><td>BOOL</td><td>✅</td></tr>
          <tr><td><code>number</code> (integer)</td><td>INT64</td><td>✅</td></tr>
          <tr><td><code>number</code> (float)</td><td>FLOAT64</td><td>✅</td></tr>
          <tr><td><code>string</code></td><td>STRING</td><td>✅</td></tr>
          <tr><td><code>Buffer</code></td><td>BYTES</td><td>✅</td></tr>
          <tr><td><code>Array</code></td><td>LIST</td><td>✅</td></tr>
          <tr><td><code>Object</code></td><td>DICT</td><td>✅</td></tr>
          <tr><td><code>Set</code></td><td>TAGGED(90)</td><td>✅</td></tr>
          <tr><td>Custom class</td><td>TAGGED(100+)</td><td>Via decoder</td></tr>
        </tbody>
      </table>

      <Callout type="warning" title="Number Handling">
        JavaScript has a single <code>number</code> type. Crous heuristically detects integers:
        if <code>num === Math.floor(num)</code> and falls within the int64 range, it's stored
        as INT64; otherwise as FLOAT64.
      </Callout>

      <h2>Performance</h2>
      <table>
        <thead>
          <tr>
            <th>Metric</th>
            <th>Value</th>
          </tr>
        </thead>
        <tbody>
          <tr><td>Small object encode</td><td>~909,000 ops/sec</td></tr>
          <tr><td>Small object decode</td><td>~1,428,000 ops/sec</td></tr>
          <tr><td>Binary data encode</td><td>15x faster than JSON</td></tr>
          <tr><td>Size reduction (binary data)</td><td>74.9% smaller than JSON</td></tr>
          <tr><td>Size reduction (typical data)</td><td>12-19% smaller than JSON</td></tr>
        </tbody>
      </table>
    </>
  );
}
