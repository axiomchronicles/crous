import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/serialization";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Node.js Serialization Guide - Encoding & Decoding - Crous" },
    { 
      name: "description", 
      content: "Learn JavaScript binary serialization with Crous. Encode and decode objects with encode(), decode(), FLUX and CROUT formats. TypeScript examples included." 
    },
    { 
      name: "keywords", 
      content: "nodejs serialization, javascript binary encoding, typescript encoding, decode javascript objects, buffer encoding, nodejs data format" 
    },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/nodejs/serialization" },
  ];
}

export default function Serialization() {
  return (
    <>
      <h1>Serialization</h1>
      <p>
        Crous provides a simple and familiar API for serializing JavaScript objects into
        compact binary format and deserializing them back. If you've used <code>JSON.stringify</code> /
        <code>JSON.parse</code>, you already know the pattern.
      </p>

      <h2>In-Memory Serialization</h2>

      <h3>dumps — Serialize to Buffer</h3>
      <CodeBlock
        filename="dumps.js"
        language="javascript"
        code={`const crous = require('crous');

const data = {
    name: 'Alice',
    scores: [95.5, 88.0, 100.0],
    active: true
};

// Serialize to Buffer
const buffer = crous.dumps(data);
console.log(buffer);        // <Buffer ...>
console.log(buffer.length); // compact binary representation`}
      />

      <h3>loads — Deserialize from Buffer</h3>
      <CodeBlock
        filename="loads.js"
        language="javascript"
        code={`const crous = require('crous');

// Deserialize from Buffer
const data = crous.loads(buffer);
console.log(data.name);   // 'Alice'
console.log(data.scores); // [95.5, 88, 100]`}
      />

      <Callout type="info" title="Buffer Input">
        <code>loads()</code> accepts both <code>Buffer</code> and <code>Uint8Array</code> objects.
      </Callout>

      <h2>File Serialization</h2>

      <h3>dump — Write to File</h3>
      <CodeBlock
        filename="file_write.js"
        language="javascript"
        code={`const crous = require('crous');

const config = {
    database: {
        host: 'localhost',
        port: 5432,
        name: 'myapp'
    },
    features: ['auth', 'logging', 'cache'],
    debug: false
};

// Write directly to a file path
crous.dump(config, 'config.crous');`}
      />

      <h3>load — Read from File</h3>
      <CodeBlock
        filename="file_read.js"
        language="javascript"
        code={`const crous = require('crous');

// Read from file path
const config = crous.load('config.crous');
console.log(config.database.host); // 'localhost'
console.log(config.features);      // ['auth', 'logging', 'cache']`}
      />

      <h2>Stream Serialization</h2>
      <p>
        <code>dump</code> and <code>load</code> also accept Node.js streams for
        integration with pipelines.
      </p>

      <CodeBlock
        filename="stream_io.js"
        language="javascript"
        code={`const crous = require('crous');
const fs = require('fs');

// Write to a writable stream
const wstream = fs.createWriteStream('output.crous');
crous.dump({ key: 'value' }, wstream);
wstream.end();

// Read from a readable stream (buffered)
const rstream = fs.createReadStream('output.crous');
const chunks = [];
rstream.on('data', (chunk) => chunks.push(chunk));
rstream.on('end', () => {
    const buffer = Buffer.concat(chunks);
    const data = crous.loads(buffer);
    console.log(data); // { key: 'value' }
});`}
      />

      <h2>The default Function</h2>
      <p>
        When Crous encounters an object it doesn't know how to serialize, you can
        provide a <code>default</code> function that converts it to a serializable form.
      </p>

      <CodeBlock
        filename="default_fn.js"
        language="javascript"
        code={`const crous = require('crous');

class Point {
    constructor(x, y) {
        this.x = x;
        this.y = y;
    }
}

const data = { origin: new Point(0, 0), cursor: new Point(10, 20) };

// Without default → CrousEncodeError
// With default → works!
const buffer = crous.dumps(data, {
    default: (obj) => {
        if (obj instanceof Point) {
            return { __point__: true, x: obj.x, y: obj.y };
        }
        throw new Error(\`Cannot serialize \${typeof obj}\`);
    }
});

console.log(crous.loads(buffer));
// { origin: { __point__: true, x: 0, y: 0 },
//   cursor: { __point__: true, x: 10, y: 20 } }`}
      />

      <h2>The object_hook Function</h2>
      <p>
        On the decoding side, <code>object_hook</code> lets you intercept every decoded
        object and transform it — perfect for reviving class instances.
      </p>

      <CodeBlock
        filename="object_hook.js"
        language="javascript"
        code={`const crous = require('crous');

class Point {
    constructor(x, y) {
        this.x = x;
        this.y = y;
    }
    toString() {
        return \`Point(\${this.x}, \${this.y})\`;
    }
}

const buffer = crous.dumps({
    origin: { __point__: true, x: 0, y: 0 },
    cursor: { __point__: true, x: 10, y: 20 }
});

const data = crous.loads(buffer, {
    object_hook: (obj) => {
        if (obj.__point__) {
            return new Point(obj.x, obj.y);
        }
        return obj;
    }
});

console.log(data.origin.toString());  // Point(0, 0)
console.log(data.cursor.toString());  // Point(10, 20)`}
      />

      <h2>CrousEncoder &amp; CrousDecoder Classes</h2>
      <p>
        For repeated serialization with the same options, the class-based API avoids
        passing options on every call.
      </p>

      <CodeBlock
        filename="encoder_decoder.js"
        language="javascript"
        code={`const { CrousEncoder, CrousDecoder } = require('crous');

// Create an encoder with a default handler
const encoder = new CrousEncoder({
    default: (obj) => {
        if (obj instanceof Date) {
            return { __date__: obj.toISOString() };
        }
        throw new TypeError(\`Unserializable: \${typeof obj}\`);
    }
});

// Create a decoder with an object hook
const decoder = new CrousDecoder({
    object_hook: (obj) => {
        if (obj.__date__) {
            return new Date(obj.__date__);
        }
        return obj;
    }
});

// Use them repeatedly
const buf = encoder.dumps({ created: new Date() });
const result = decoder.loads(buf);
console.log(result.created instanceof Date); // true`}
      />

      <h2>Comparison with JSON</h2>
      <table>
        <thead>
          <tr>
            <th>Feature</th>
            <th>JSON</th>
            <th>Crous</th>
          </tr>
        </thead>
        <tbody>
          <tr><td>Format</td><td>Text (UTF-8)</td><td>Binary (compact)</td></tr>
          <tr><td>Buffer / Bytes</td><td>❌ Base64 workaround</td><td>✅ Native</td></tr>
          <tr><td>Set</td><td>❌ No</td><td>✅ Tagged type (90)</td></tr>
          <tr><td>Integer precision</td><td>⚠️ 53-bit</td><td>✅ Full 64-bit</td></tr>
          <tr><td>NaN / Infinity</td><td>❌ No</td><td>✅ Yes</td></tr>
          <tr><td>default / replacer</td><td>✅ Yes</td><td>✅ Yes</td></tr>
          <tr><td>object_hook / reviver</td><td>✅ Yes</td><td>✅ Yes</td></tr>
          <tr><td>Human-readable</td><td>✅ Yes</td><td>❌ No</td></tr>
        </tbody>
      </table>
    </>
  );
}
