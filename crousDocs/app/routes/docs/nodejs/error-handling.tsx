import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/error-handling";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Node.js Error Handling - Exceptions & Debugging - Crous" },
    { name: "description", content: "Handle errors in Crous Node.js SDK. CrousError class, TypeScript error types, validation, debugging, and error recovery patterns." },
    { name: "keywords", content: "nodejs error handling, javascript exceptions, typescript errors, error types nodejs, validation errors, exception handling" },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/nodejs/error-handling" },
  ];
}

export default function ErrorHandling() {
  return (
    <>
      <h1>Error Handling</h1>
      <p>
        Crous provides a structured error hierarchy that maps C-level error codes to
        JavaScript exceptions. Every error includes a descriptive message to help you
        diagnose issues quickly.
      </p>

      <h2>Exception Hierarchy</h2>
      <div className="my-6 p-5 bg-tubox-card border border-tubox-border rounded-xl font-mono text-sm leading-loose">
        <div className="text-zinc-300">Error</div>
        <div className="pl-6 text-crous-400">
          └── CrousError
          <div className="pl-6 text-crous-300">
            ├── CrousEncodeError
            <br />
            └── CrousDecodeError
          </div>
        </div>
      </div>

      <ul>
        <li><strong>CrousError</strong> — Base class for all Crous errors</li>
        <li><strong>CrousEncodeError</strong> — Thrown during serialization (dumps, dump)</li>
        <li><strong>CrousDecodeError</strong> — Thrown during deserialization (loads, load)</li>
      </ul>

      <h2>Catching Errors</h2>
      <CodeBlock
        filename="catch_errors.js"
        language="javascript"
        code={`const crous = require('crous');

// ─── Encoding Errors ────────────────────────────

try {
    // Circular reference
    const obj = {};
    obj.self = obj;
    crous.dumps(obj);
} catch (e) {
    if (e instanceof crous.CrousEncodeError) {
        console.log('Encode error:', e.message);
    }
}

try {
    // Unsupported type (without default handler)
    crous.dumps(new Map([['a', 1]]));
} catch (e) {
    console.log(e.name);    // 'CrousEncodeError'
    console.log(e.message); // unsupported type details
}

// ─── Decoding Errors ────────────────────────────

try {
    // Invalid data
    crous.loads(Buffer.from([0xFF, 0x00, 0x00]));
} catch (e) {
    if (e instanceof crous.CrousDecodeError) {
        console.log('Decode error:', e.message);
    }
}

try {
    // Truncated data
    const valid = crous.dumps({ key: 'value' });
    crous.loads(valid.subarray(0, 5));
} catch (e) {
    console.log(e.name);    // 'CrousDecodeError'
}

// ─── Generic catch ──────────────────────────────

try {
    crous.loads(Buffer.from('garbage'));
} catch (e) {
    if (e instanceof crous.CrousError) {
        // Catches both encode and decode errors
        console.log('Crous error:', e.message);
    }
}`}
      />

      <h2>C Error Codes</h2>
      <p>
        Under the hood, the C core returns numeric error codes that the Node.js binding
        translates to JavaScript exceptions. Understanding these codes helps when debugging
        low-level issues.
      </p>
      <table>
        <thead>
          <tr>
            <th>Code</th>
            <th>Name</th>
            <th>Description</th>
          </tr>
        </thead>
        <tbody>
          <tr><td>0</td><td><code>CROUS_OK</code></td><td>No error — success</td></tr>
          <tr><td>1</td><td><code>CROUS_ERR_MEMORY</code></td><td>Memory allocation failed</td></tr>
          <tr><td>2</td><td><code>CROUS_ERR_ENCODE</code></td><td>General encoding error</td></tr>
          <tr><td>3</td><td><code>CROUS_ERR_DECODE</code></td><td>General decoding error</td></tr>
          <tr><td>4</td><td><code>CROUS_ERR_OVERFLOW</code></td><td>Buffer or integer overflow</td></tr>
          <tr><td>5</td><td><code>CROUS_ERR_INVALID_TYPE</code></td><td>Unsupported type encountered</td></tr>
          <tr><td>6</td><td><code>CROUS_ERR_INVALID_DATA</code></td><td>Malformed or corrupted data</td></tr>
          <tr><td>7</td><td><code>CROUS_ERR_IO</code></td><td>File or stream I/O error</td></tr>
          <tr><td>8</td><td><code>CROUS_ERR_KEY_TYPE</code></td><td>Non-string dictionary key</td></tr>
          <tr><td>9</td><td><code>CROUS_ERR_NULL_INPUT</code></td><td>Null pointer passed to API</td></tr>
          <tr><td>10</td><td><code>CROUS_ERR_VERSION</code></td><td>Format version mismatch</td></tr>
          <tr><td>11</td><td><code>CROUS_ERR_DEPTH_EXCEEDED</code></td><td>Maximum nesting depth exceeded</td></tr>
        </tbody>
      </table>

      <h2>Common Error Scenarios</h2>

      <h3>Unsupported Types</h3>
      <CodeBlock
        filename="unsupported_type.js"
        language="javascript"
        code={`const crous = require('crous');

// Map is not supported natively
try {
    crous.dumps(new Map([['key', 'value']]));
} catch (e) {
    console.log(e.message);
    // Fix: convert to plain object
    const map = new Map([['key', 'value']]);
    crous.dumps(Object.fromEntries(map)); // ✓ works
}

// Functions cannot be serialized
try {
    crous.dumps({ handler: () => {} });
} catch (e) {
    console.log(e.message); // function type not supported
}

// Symbol keys
try {
    crous.dumps({ [Symbol('id')]: 42 });
} catch (e) {
    console.log(e.message); // invalid key type
}`}
      />

      <h3>Circular References</h3>
      <CodeBlock
        filename="circular.js"
        language="javascript"
        code={`const crous = require('crous');

// Direct circular reference
const a = {};
a.self = a;

try {
    crous.dumps(a);
} catch (e) {
    console.log(e.name); // CrousEncodeError
    // Max depth exceeded due to infinite recursion
}

// Indirect circular reference
const x = { name: 'x' };
const y = { name: 'y', ref: x };
x.ref = y;

try {
    crous.dumps(x);
} catch (e) {
    console.log(e.message); // depth exceeded
}`}
      />

      <Callout type="info" title="Depth Limit">
        Crous enforces a maximum nesting depth to protect against circular references
        and stack overflow. Deeply nested but non-circular structures may also trigger
        this limit.
      </Callout>

      <h3>Corrupted Data</h3>
      <CodeBlock
        filename="corrupted.js"
        language="javascript"
        code={`const crous = require('crous');

// Random bytes
try {
    crous.loads(Buffer.from([0xDE, 0xAD, 0xBE, 0xEF]));
} catch (e) {
    console.log(e.name); // CrousDecodeError
}

// Truncated valid data
const valid = crous.dumps({ big: 'data'.repeat(1000) });
try {
    crous.loads(valid.subarray(0, 10));
} catch (e) {
    console.log(e.name); // CrousDecodeError
}

// Empty buffer
try {
    crous.loads(Buffer.alloc(0));
} catch (e) {
    console.log(e.name); // CrousDecodeError
}`}
      />

      <h2>Best Practices</h2>

      <h3>1. Use Specific Error Classes</h3>
      <CodeBlock
        filename="best_specific.js"
        language="javascript"
        code={`const crous = require('crous');

function safeSerialize(data) {
    try {
        return crous.dumps(data);
    } catch (e) {
        if (e instanceof crous.CrousEncodeError) {
            console.error('Serialization failed:', e.message);
            return null;
        }
        throw e; // Re-throw unexpected errors
    }
}

function safeDeserialize(buffer) {
    try {
        return crous.loads(buffer);
    } catch (e) {
        if (e instanceof crous.CrousDecodeError) {
            console.error('Deserialization failed:', e.message);
            return null;
        }
        throw e;
    }
}`}
      />

      <h3>2. Validate Before Serializing</h3>
      <CodeBlock
        filename="best_validate.js"
        language="javascript"
        code={`function validateData(data) {
    if (data === undefined) {
        throw new Error('Data cannot be undefined at top level');
    }
    if (typeof data === 'function') {
        throw new Error('Functions cannot be serialized');
    }
    if (data instanceof Map) {
        throw new Error('Convert Map to Object first');
    }
    return true;
}`}
      />

      <h3>3. Use default for Graceful Fallbacks</h3>
      <CodeBlock
        filename="best_default.js"
        language="javascript"
        code={`const crous = require('crous');

const encoder = new crous.CrousEncoder({
    default: (obj) => {
        // Handle Dates
        if (obj instanceof Date) {
            return { __type: 'Date', value: obj.toISOString() };
        }
        // Handle Maps
        if (obj instanceof Map) {
            return { __type: 'Map', entries: [...obj.entries()] };
        }
        // Handle RegExp
        if (obj instanceof RegExp) {
            return { __type: 'RegExp', source: obj.source, flags: obj.flags };
        }
        // Unknown → log and skip
        console.warn(\`Skipping unserializable: \${obj?.constructor?.name}\`);
        return null;
    }
});`}
      />
    </>
  );
}
