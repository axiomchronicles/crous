import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/types";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Node.js Type System - Supported Data Types - Crous" },
    { 
      name: "description", 
      content: "Complete guide to JavaScript and TypeScript types in Crous: null, boolean, number, string, Buffer, Array, Object, and tagged types with BigInt support." 
    },
    { 
      name: "keywords", 
      content: "javascript data types, typescript types, crous nodejs types, binary encoding types, javascript type mapping, buffer encoding" 
    },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/nodejs/types" },
  ];
}

export default function Types() {
  return (
    <>
      <h1>Type System</h1>
      <p>
        Crous maps JavaScript types to its internal binary type system. Understanding
        these mappings is essential for predictable serialization, especially for edge
        cases around numbers and types that don't exist natively in JavaScript.
      </p>

      <h2>Type Mapping Table</h2>
      <table>
        <thead>
          <tr>
            <th>JavaScript Type</th>
            <th>Crous Wire Type</th>
            <th>Decoded As</th>
            <th>Notes</th>
          </tr>
        </thead>
        <tbody>
          <tr><td><code>null</code></td><td>NULL (0x01)</td><td><code>null</code></td><td></td></tr>
          <tr><td><code>undefined</code></td><td>NULL (0x01)</td><td><code>null</code></td><td>Lossy — becomes null</td></tr>
          <tr><td><code>true</code> / <code>false</code></td><td>BOOL (0x02)</td><td><code>boolean</code></td><td></td></tr>
          <tr><td><code>number</code> (integer)</td><td>INT64 (0x03)</td><td><code>number</code></td><td>Heuristic applied</td></tr>
          <tr><td><code>number</code> (float)</td><td>FLOAT64 (0x04)</td><td><code>number</code></td><td></td></tr>
          <tr><td><code>string</code></td><td>STRING (0x05)</td><td><code>string</code></td><td>UTF-8 encoded</td></tr>
          <tr><td><code>Buffer</code></td><td>BYTES (0x06)</td><td><code>Buffer</code></td><td></td></tr>
          <tr><td><code>Uint8Array</code></td><td>BYTES (0x06)</td><td><code>Buffer</code></td><td>Decoded as Buffer</td></tr>
          <tr><td><code>Array</code></td><td>LIST (0x07)</td><td><code>Array</code></td><td>Heterogeneous OK</td></tr>
          <tr><td><code>Object</code> (plain)</td><td>DICT (0x09)</td><td><code>Object</code></td><td>String keys only</td></tr>
          <tr><td><code>Set</code></td><td>TAGGED(90) → LIST</td><td><code>Set</code></td><td>Via tagged type</td></tr>
        </tbody>
      </table>

      <h2>Number Encoding Heuristic</h2>
      <p>
        JavaScript has a single <code>number</code> type (IEEE 754 double). Crous uses a
        heuristic to determine whether a number should be encoded as an integer or float:
      </p>

      <CodeBlock
        filename="number_heuristic.c"
        language="c"
        code={`// Internal C logic (simplified)
double num = napi_get_value_double(value);
int64_t as_int = (int64_t)num;

if ((double)as_int == num &&
    as_int >= INT64_MIN &&
    as_int <= INT64_MAX) {
    // Encode as INT64
} else {
    // Encode as FLOAT64
}`}
      />

      <p>In practice, this means:</p>

      <CodeBlock
        filename="numbers.js"
        language="javascript"
        code={`const crous = require('crous');

// These encode as INT64
crous.dumps(42);           // integer
crous.dumps(0);            // zero
crous.dumps(-100);         // negative integer
crous.dumps(2 ** 53 - 1);  // max safe integer

// These encode as FLOAT64
crous.dumps(3.14);         // has decimal
crous.dumps(NaN);          // NaN
crous.dumps(Infinity);     // Infinity
crous.dumps(-Infinity);    // -Infinity
crous.dumps(1.0);          // ⚠️ This is INT64! (1.0 === 1 in JS)`}
      />

      <Callout type="warning" title="1.0 is an Integer">
        In JavaScript, <code>1.0 === 1</code> is <code>true</code>. There's no way to distinguish
        them. Crous encodes <code>1.0</code> as INT64 because the heuristic sees a whole number.
        This is generally not a problem, but be aware of it.
      </Callout>

      <h2>Null and Undefined</h2>
      <p>
        Both <code>null</code> and <code>undefined</code> map to the same wire type
        (<code>NULL</code>). On decode, they always come back as <code>null</code>.
      </p>

      <CodeBlock
        filename="null_undefined.js"
        language="javascript"
        code={`const crous = require('crous');

const data = {
    a: null,
    b: undefined
};

const result = crous.loads(crous.dumps(data));
console.log(result);
// { a: null, b: null }
// undefined was converted to null!`}
      />

      <Callout type="warning" title="Lossy Mapping">
        <code>undefined</code> → <code>null</code> is a lossy conversion. If you need to
        preserve the distinction, filter out undefined values before serialization or use
        a sentinel value.
      </Callout>

      <h2>Strings</h2>
      <p>
        Strings are encoded as UTF-8 with a 4-byte length prefix. There is no practical
        length limit beyond available memory.
      </p>
      <CodeBlock
        filename="strings.js"
        language="javascript"
        code={`const crous = require('crous');

// Regular strings
crous.dumps('Hello, World!');

// Unicode — full support
crous.dumps('こんにちは 🌍');

// Empty strings
crous.dumps('');

// Very long strings — works fine
crous.dumps('x'.repeat(1_000_000));`}
      />

      <h2>Binary Data (Buffer)</h2>
      <p>
        Crous natively supports binary data through <code>Buffer</code>. No base64
        encoding needed — bytes go straight into the wire format.
      </p>
      <CodeBlock
        filename="binary.js"
        language="javascript"
        code={`const crous = require('crous');
const fs = require('fs');

// Buffer from array
const buf = Buffer.from([0x89, 0x50, 0x4E, 0x47]);
crous.dumps(buf);

// Read a file as binary
const image = fs.readFileSync('photo.jpg');
const encoded = crous.dumps({ image });
// The image bytes are stored efficiently — no base64 bloat!

// Uint8Array also works
const arr = new Uint8Array([1, 2, 3, 4]);
crous.dumps(arr);  // encoded as BYTES`}
      />

      <h2>Arrays</h2>
      <p>
        JavaScript arrays map to Crous LIST. They can contain any mix of types.
      </p>
      <CodeBlock
        filename="arrays.js"
        language="javascript"
        code={`const crous = require('crous');

// Homogeneous
crous.dumps([1, 2, 3, 4, 5]);

// Heterogeneous — no problem
crous.dumps([1, 'two', true, null, Buffer.from([3])]);

// Nested
crous.dumps([[1, 2], [3, 4], [5, 6]]);

// Empty
crous.dumps([]);`}
      />

      <Callout type="info" title="No Tuples">
        JavaScript doesn't have a tuple type. Python tuples deserialized by the Node.js
        SDK will become Arrays. This is a one-way lossy mapping when interoperating with
        the Python SDK.
      </Callout>

      <h2>Objects (Dictionaries)</h2>
      <p>
        Plain objects are encoded as Crous DICT. Keys must be strings.
      </p>
      <CodeBlock
        filename="objects.js"
        language="javascript"
        code={`const crous = require('crous');

// Plain objects
crous.dumps({ name: 'Alice', age: 30 });

// Nested objects
crous.dumps({
    user: {
        profile: {
            bio: 'Hello!'
        }
    }
});

// Empty object
crous.dumps({});

// ⚠️ Non-string keys will throw CrousEncodeError
// crous.dumps({ [Symbol()]: 'value' }); // Error!`}
      />

      <h2>Sets</h2>
      <p>
        JavaScript <code>Set</code> objects are serialized using the tagged type mechanism
        with tag <code>90</code>. The set elements are stored as a list internally.
      </p>
      <CodeBlock
        filename="sets.js"
        language="javascript"
        code={`const crous = require('crous');

const data = {
    tags: new Set(['admin', 'user', 'moderator']),
    ids: new Set([1, 2, 3])
};

const result = crous.loads(crous.dumps(data));
console.log(result.tags instanceof Set); // true
console.log(result.tags.has('admin'));    // true`}
      />

      <h2>Cross-SDK Type Compatibility</h2>
      <p>
        When exchanging Crous data between the Python and Node.js SDKs, be aware of these
        type differences:
      </p>
      <table>
        <thead>
          <tr>
            <th>Python Type</th>
            <th>Wire Type</th>
            <th>JavaScript Type</th>
            <th>Lossy?</th>
          </tr>
        </thead>
        <tbody>
          <tr><td><code>None</code></td><td>NULL</td><td><code>null</code></td><td>No</td></tr>
          <tr><td><code>bool</code></td><td>BOOL</td><td><code>boolean</code></td><td>No</td></tr>
          <tr><td><code>int</code></td><td>INT64</td><td><code>number</code></td><td>⚠️ &gt; 2⁵³</td></tr>
          <tr><td><code>float</code></td><td>FLOAT64</td><td><code>number</code></td><td>No</td></tr>
          <tr><td><code>str</code></td><td>STRING</td><td><code>string</code></td><td>No</td></tr>
          <tr><td><code>bytes</code></td><td>BYTES</td><td><code>Buffer</code></td><td>No</td></tr>
          <tr><td><code>list</code></td><td>LIST</td><td><code>Array</code></td><td>No</td></tr>
          <tr><td><code>tuple</code></td><td>TUPLE</td><td><code>Array</code></td><td>Yes</td></tr>
          <tr><td><code>dict</code></td><td>DICT</td><td><code>Object</code></td><td>No</td></tr>
          <tr><td><code>set</code></td><td>SET</td><td><code>Set</code></td><td>No</td></tr>
          <tr><td><code>frozenset</code></td><td>FROZENSET</td><td><code>Set</code></td><td>Yes</td></tr>
        </tbody>
      </table>

      <Callout type="warning" title="Large Integers">
        Python can serialize integers beyond JavaScript's safe integer range (2⁵³ - 1).
        These will lose precision when decoded by the Node.js SDK. Consider using strings
        for very large integers in cross-language scenarios.
      </Callout>
    </>
  );
}
