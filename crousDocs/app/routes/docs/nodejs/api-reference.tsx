import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/api-reference";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Node.js API Reference - Crous Binary Serialization" },
    { 
      name: "description", 
      content: "Complete Node.js API reference for Crous. Functions, TypeScript types, and methods for encoding, decoding, file I/O, and error handling." 
    },
    { 
      name: "keywords", 
      content: "nodejs api reference, crous typescript, javascript serialization api, encoding functions nodejs, binary format api, typescript definitions" 
    },
    { property: "og:title", content: "Node.js API Reference - Crous" },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/nodejs/api-reference" },
  ];
}

export default function ApiReference() {
  return (
    <>
      <h1>API Reference</h1>
      <p>
        Complete reference for every function, class, and type exported by the
        Crous Node.js SDK.
      </p>

      {/* ─── Module Functions ───────────────────────── */}
      <h2>Module Functions</h2>

      <h3 id="dumps">
        <code>dumps(data, options?)</code>
      </h3>
      <p>Serialize a JavaScript value to a Crous binary Buffer.</p>
      <table>
        <thead>
          <tr><th>Parameter</th><th>Type</th><th>Description</th></tr>
        </thead>
        <tbody>
          <tr><td><code>data</code></td><td><code>any</code></td><td>The value to serialize</td></tr>
          <tr><td><code>options.default</code></td><td><code>DefaultFunction?</code></td><td>Fallback for unsupported types</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>Buffer</code></p>
      <p><strong>Throws:</strong> <code>CrousEncodeError</code></p>
      <CodeBlock
        language="javascript"
        code={`const buf = crous.dumps({ key: 'value' });
const buf2 = crous.dumps(data, {
    default: (obj) => obj.toJSON()
});`}
      />

      <h3 id="loads">
        <code>loads(buffer, options?)</code>
      </h3>
      <p>Deserialize a Crous binary Buffer to a JavaScript value.</p>
      <table>
        <thead>
          <tr><th>Parameter</th><th>Type</th><th>Description</th></tr>
        </thead>
        <tbody>
          <tr><td><code>buffer</code></td><td><code>Buffer | Uint8Array</code></td><td>Binary data to decode</td></tr>
          <tr><td><code>options.object_hook</code></td><td><code>ObjectHook?</code></td><td>Transform decoded objects</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>any</code></p>
      <p><strong>Throws:</strong> <code>CrousDecodeError</code></p>
      <CodeBlock
        language="javascript"
        code={`const data = crous.loads(buffer);
const data2 = crous.loads(buffer, {
    object_hook: (obj) => revive(obj)
});`}
      />

      <h3 id="dump">
        <code>dump(data, target, options?)</code>
      </h3>
      <p>Serialize data and write to a file path or writable stream.</p>
      <table>
        <thead>
          <tr><th>Parameter</th><th>Type</th><th>Description</th></tr>
        </thead>
        <tbody>
          <tr><td><code>data</code></td><td><code>any</code></td><td>The value to serialize</td></tr>
          <tr><td><code>target</code></td><td><code>string | Writable</code></td><td>File path or writable stream</td></tr>
          <tr><td><code>options.default</code></td><td><code>DefaultFunction?</code></td><td>Fallback for unsupported types</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>void</code></p>
      <p><strong>Throws:</strong> <code>CrousEncodeError</code>, file system errors</p>
      <CodeBlock
        language="javascript"
        code={`crous.dump({ key: 'value' }, 'output.crous');
crous.dump(data, writableStream);`}
      />

      <h3 id="load">
        <code>load(source, options?)</code>
      </h3>
      <p>Read and deserialize data from a file path.</p>
      <table>
        <thead>
          <tr><th>Parameter</th><th>Type</th><th>Description</th></tr>
        </thead>
        <tbody>
          <tr><td><code>source</code></td><td><code>string</code></td><td>File path to read from</td></tr>
          <tr><td><code>options.object_hook</code></td><td><code>ObjectHook?</code></td><td>Transform decoded objects</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>any</code></p>
      <p><strong>Throws:</strong> <code>CrousDecodeError</code>, file system errors</p>
      <CodeBlock
        language="javascript"
        code={`const data = crous.load('data.crous');`}
      />

      {/* ─── Custom Type Registration ──────────────── */}
      <h2>Custom Type Registration</h2>

      <h3 id="registerSerializer">
        <code>registerSerializer(constructor, serializer)</code>
      </h3>
      <p>Register a serializer function for a custom type.</p>
      <table>
        <thead>
          <tr><th>Parameter</th><th>Type</th><th>Description</th></tr>
        </thead>
        <tbody>
          <tr><td><code>constructor</code></td><td><code>Function</code></td><td>The class/constructor to match</td></tr>
          <tr><td><code>serializer</code></td><td><code>SerializerFunction</code></td><td>Converts instance to serializable form</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>void</code></p>
      <CodeBlock
        language="javascript"
        code={`crous.registerSerializer(MyClass, (obj) => ({
    field1: obj.field1,
    field2: obj.field2
}));`}
      />

      <h3 id="unregisterSerializer">
        <code>unregisterSerializer(constructor)</code>
      </h3>
      <p>Remove a previously registered serializer.</p>
      <table>
        <thead>
          <tr><th>Parameter</th><th>Type</th><th>Description</th></tr>
        </thead>
        <tbody>
          <tr><td><code>constructor</code></td><td><code>Function</code></td><td>The class/constructor to unregister</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>void</code></p>

      <h3 id="registerDecoder">
        <code>registerDecoder(tag, decoder)</code>
      </h3>
      <p>Register a decoder function for a tagged custom type.</p>
      <table>
        <thead>
          <tr><th>Parameter</th><th>Type</th><th>Description</th></tr>
        </thead>
        <tbody>
          <tr><td><code>tag</code></td><td><code>number</code></td><td>Tag number (≥ 100 for user types)</td></tr>
          <tr><td><code>decoder</code></td><td><code>DecoderFunction</code></td><td>Reconstructs instance from decoded data</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>void</code></p>
      <CodeBlock
        language="javascript"
        code={`crous.registerDecoder(100, (data) => new MyClass(
    data.field1, data.field2
));`}
      />

      <h3 id="unregisterDecoder">
        <code>unregisterDecoder(tag)</code>
      </h3>
      <p>Remove a previously registered decoder.</p>
      <table>
        <thead>
          <tr><th>Parameter</th><th>Type</th><th>Description</th></tr>
        </thead>
        <tbody>
          <tr><td><code>tag</code></td><td><code>number</code></td><td>The tag number to unregister</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>void</code></p>

      {/* ─── Classes ─────────────────────────────── */}
      <h2>Classes</h2>

      <h3 id="CrousEncoder">
        <code>CrousEncoder</code>
      </h3>
      <p>Stateful encoder with pre-configured options.</p>
      <CodeBlock
        language="typescript"
        code={`class CrousEncoder {
    constructor(options?: {
        default?: DefaultFunction;
    });
    dumps(data: any): Buffer;
    dump(data: any, target: string | Writable): void;
}`}
      />

      <h3 id="CrousDecoder">
        <code>CrousDecoder</code>
      </h3>
      <p>Stateful decoder with pre-configured options.</p>
      <CodeBlock
        language="typescript"
        code={`class CrousDecoder {
    constructor(options?: {
        object_hook?: ObjectHook;
    });
    loads(buffer: Buffer | Uint8Array): any;
    load(source: string): any;
}`}
      />

      {/* ─── Error Classes ──────────────────────── */}
      <h2>Error Classes</h2>

      <h3 id="CrousError">
        <code>CrousError</code>
      </h3>
      <p>Base error class. All Crous-specific errors extend this.</p>
      <CodeBlock
        language="typescript"
        code={`class CrousError extends Error {
    name: 'CrousError';
    message: string;
}`}
      />

      <h3 id="CrousEncodeError">
        <code>CrousEncodeError</code>
      </h3>
      <p>Thrown when serialization fails.</p>
      <CodeBlock
        language="typescript"
        code={`class CrousEncodeError extends CrousError {
    name: 'CrousEncodeError';
}`}
      />
      <p><strong>Common causes:</strong></p>
      <ul>
        <li>Unsupported type (Map, Function, Symbol, etc.)</li>
        <li>Circular reference / max depth exceeded</li>
        <li>Non-string dictionary key</li>
        <li>Memory allocation failure</li>
      </ul>

      <h3 id="CrousDecodeError">
        <code>CrousDecodeError</code>
      </h3>
      <p>Thrown when deserialization fails.</p>
      <CodeBlock
        language="typescript"
        code={`class CrousDecodeError extends CrousError {
    name: 'CrousDecodeError';
}`}
      />
      <p><strong>Common causes:</strong></p>
      <ul>
        <li>Corrupted or malformed data</li>
        <li>Truncated buffer</li>
        <li>Unknown tagged type (no decoder registered)</li>
        <li>Version mismatch</li>
      </ul>

      {/* ─── Version ─────────────────────────────── */}
      <h2>Version Information</h2>

      <h3 id="version">
        <code>version</code>
      </h3>
      <p>The SDK version as a string.</p>
      <CodeBlock
        language="javascript"
        code={`console.log(crous.version); // '2.0.0'`}
      />

      <h3 id="versionInfo">
        <code>versionInfo</code>
      </h3>
      <p>Structured version information from the C core.</p>
      <CodeBlock
        language="typescript"
        code={`interface VersionInfo {
    major: number;
    minor: number;
    patch: number;
    full: string;       // e.g., '2.0.0'
    build_date: string; // compilation date
    compiler: string;   // compiler used
}

console.log(crous.versionInfo);
// {
//   major: 2, minor: 0, patch: 0,
//   full: '2.0.0',
//   build_date: '2025-01-15',
//   compiler: 'Apple clang 15.0.0'
// }`}
      />

      <h3 id="versionTuple">
        <code>versionTuple</code>
      </h3>
      <p>The version as a <code>[major, minor, patch]</code> array.</p>
      <CodeBlock
        language="javascript"
        code={`console.log(crous.versionTuple); // [2, 0, 0]`}
      />

      {/* ─── TypeScript Types ─────────────────────── */}
      <h2>TypeScript Types</h2>

      <CodeBlock
        filename="index.d.ts"
        language="typescript"
        code={`// Callback that converts unsupported objects to serializable form
type DefaultFunction = (obj: any) => any;

// Callback that transforms every decoded object
type ObjectHook = (obj: Record<string, any>) => any;

// Converts a custom type instance to a serializable value
type SerializerFunction = (obj: any) => any;

// Reconstructs a custom type from decoded data
type DecoderFunction = (data: any) => any;

// Options for dumps / dump
interface EncodeOptions {
    default?: DefaultFunction;
}

// Options for loads / load
interface DecodeOptions {
    object_hook?: ObjectHook;
}

// Version info structure
interface VersionInfo {
    major: number;
    minor: number;
    patch: number;
    full: string;
    build_date: string;
    compiler: string;
}

// Module exports
export function dumps(data: any, options?: EncodeOptions): Buffer;
export function loads(buffer: Buffer | Uint8Array, options?: DecodeOptions): any;
export function dump(data: any, target: string | NodeJS.WritableStream, options?: EncodeOptions): void;
export function load(source: string, options?: DecodeOptions): any;

export function registerSerializer(constructor: Function, serializer: SerializerFunction): void;
export function unregisterSerializer(constructor: Function): void;
export function registerDecoder(tag: number, decoder: DecoderFunction): void;
export function unregisterDecoder(tag: number): void;

export class CrousEncoder {
    constructor(options?: EncodeOptions);
    dumps(data: any): Buffer;
    dump(data: any, target: string | NodeJS.WritableStream): void;
}

export class CrousDecoder {
    constructor(options?: DecodeOptions);
    loads(buffer: Buffer | Uint8Array): any;
    load(source: string): any;
}

export class CrousError extends Error {}
export class CrousEncodeError extends CrousError {}
export class CrousDecodeError extends CrousError {}

export const version: string;
export const versionInfo: VersionInfo;
export const versionTuple: [number, number, number];`}
      />

      {/* ─── Quick Reference ──────────────────────── */}
      <h2>Quick Reference</h2>
      <table>
        <thead>
          <tr><th>Task</th><th>Function</th></tr>
        </thead>
        <tbody>
          <tr><td>Serialize to Buffer</td><td><code>dumps(data)</code></td></tr>
          <tr><td>Deserialize from Buffer</td><td><code>loads(buffer)</code></td></tr>
          <tr><td>Write to file</td><td><code>dump(data, path)</code></td></tr>
          <tr><td>Read from file</td><td><code>load(path)</code></td></tr>
          <tr><td>Custom type → binary</td><td><code>registerSerializer(ctor, fn)</code></td></tr>
          <tr><td>Binary → custom type</td><td><code>registerDecoder(tag, fn)</code></td></tr>
          <tr><td>Handle unknown types</td><td><code>dumps(data, {'{'} default: fn {'}'})</code></td></tr>
          <tr><td>Revive objects</td><td><code>loads(buf, {'{'} object_hook: fn {'}'})</code></td></tr>
          <tr><td>Check version</td><td><code>crous.version</code></td></tr>
        </tbody>
      </table>
    </>
  );
}
