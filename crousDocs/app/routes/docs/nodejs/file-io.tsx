import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/file-io";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Node.js File I/O - Read & Write Binary Files - Crous" },
    { name: "description", content: "Read and write Crous binary files in Node.js. Synchronous and asynchronous file operations, streaming support, and best practices." },
    { name: "keywords", content: "nodejs file io, binary file operations, read write files, async file operations, file streaming nodejs, buffer file io" },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/nodejs/file-io" },
  ];
}

export default function FileIO() {
  return (
    <>
      <h1>File I/O</h1>
      <p>
        The Crous Node.js SDK provides convenient functions for reading and writing
        serialized data to files and streams, on top of the in-memory
        <code>dumps</code> / <code>loads</code> API.
      </p>

      <h2>Writing to Files</h2>

      <h3>dump — Write by File Path</h3>
      <p>
        The simplest way to persist data. Pass the data and a file path — Crous handles
        the rest.
      </p>
      <CodeBlock
        filename="write_file.js"
        language="javascript"
        code={`const crous = require('crous');

const config = {
    server: { host: '0.0.0.0', port: 8080 },
    database: { url: 'postgres://localhost/mydb' },
    features: ['auth', 'logging', 'rate-limiting'],
    maxRetries: 3,
    debug: false
};

// Write to file — synchronous
crous.dump(config, 'config.crous');
console.log('✓ Config saved');`}
      />

      <h3>dump — Write to Stream</h3>
      <p>
        Pass a writable stream instead of a file path for integration with Node.js
        stream pipelines.
      </p>
      <CodeBlock
        filename="write_stream.js"
        language="javascript"
        code={`const crous = require('crous');
const fs = require('fs');

const data = { message: 'Hello from a stream!' };

const stream = fs.createWriteStream('output.crous');
crous.dump(data, stream);
stream.end();

// Works with any Writable stream
const { PassThrough } = require('stream');
const passthrough = new PassThrough();
const chunks = [];
passthrough.on('data', (chunk) => chunks.push(chunk));
passthrough.on('end', () => {
    const buffer = Buffer.concat(chunks);
    console.log(\`Captured \${buffer.length} bytes\`);
});
crous.dump(data, passthrough);
passthrough.end();`}
      />

      <h2>Reading from Files</h2>

      <h3>load — Read by File Path</h3>
      <CodeBlock
        filename="read_file.js"
        language="javascript"
        code={`const crous = require('crous');

// Read from file — synchronous
const config = crous.load('config.crous');
console.log(config.server.host); // '0.0.0.0'
console.log(config.features);    // ['auth', 'logging', 'rate-limiting']`}
      />

      <h3>load — Read from Buffer</h3>
      <p>
        When you already have the file contents in memory (e.g., from an HTTP response
        or database), use <code>loads</code> directly.
      </p>
      <CodeBlock
        filename="read_buffer.js"
        language="javascript"
        code={`const crous = require('crous');
const fs = require('fs');

// Read file into buffer manually
const buffer = fs.readFileSync('config.crous');
const config = crous.loads(buffer);

// From an HTTP response
const http = require('http');
http.get('http://api.example.com/data.crous', (res) => {
    const chunks = [];
    res.on('data', (chunk) => chunks.push(chunk));
    res.on('end', () => {
        const data = crous.loads(Buffer.concat(chunks));
        console.log(data);
    });
});`}
      />

      <h2>Multi-Record Files</h2>
      <p>
        You can write multiple records to the same file by concatenating serialized
        buffers. Each call to <code>dumps</code> produces a self-contained record.
      </p>
      <CodeBlock
        filename="multi_record.js"
        language="javascript"
        code={`const crous = require('crous');
const fs = require('fs');

// Write multiple records
const stream = fs.createWriteStream('events.crous');

const events = [
    { type: 'click', x: 100, y: 200, time: Date.now() },
    { type: 'scroll', delta: -50, time: Date.now() },
    { type: 'keypress', key: 'Enter', time: Date.now() },
];

for (const event of events) {
    const buf = crous.dumps(event);
    // Write length prefix + data for framing
    const lenBuf = Buffer.alloc(4);
    lenBuf.writeUInt32BE(buf.length);
    stream.write(lenBuf);
    stream.write(buf);
}
stream.end();

// Read multiple records back
const fileData = fs.readFileSync('events.crous');
let offset = 0;
const decoded = [];

while (offset < fileData.length) {
    const len = fileData.readUInt32BE(offset);
    offset += 4;
    const record = fileData.subarray(offset, offset + len);
    decoded.push(crous.loads(record));
    offset += len;
}

console.log(\`Read \${decoded.length} events\`);
decoded.forEach((e) => console.log(e.type));`}
      />

      <h2>Using with CrousEncoder / CrousDecoder</h2>
      <p>
        The class-based API also supports file I/O with the same <code>dump</code> /
        <code>load</code> methods.
      </p>
      <CodeBlock
        filename="encoder_file_io.js"
        language="javascript"
        code={`const { CrousEncoder, CrousDecoder } = require('crous');

const encoder = new CrousEncoder({
    default: (obj) => {
        if (obj instanceof Date) {
            return { __date__: obj.toISOString() };
        }
        throw new TypeError('Unserializable');
    }
});

const decoder = new CrousDecoder({
    object_hook: (obj) => {
        if (obj.__date__) return new Date(obj.__date__);
        return obj;
    }
});

// Write
encoder.dump({ created: new Date(), name: 'test' }, 'record.crous');

// Read
const record = decoder.load('record.crous');
console.log(record.created instanceof Date); // true`}
      />

      <h2>Error Handling for File I/O</h2>
      <CodeBlock
        filename="error_handling.js"
        language="javascript"
        code={`const crous = require('crous');

// File not found
try {
    crous.load('nonexistent.crous');
} catch (e) {
    console.log(e.code);    // 'ENOENT'
    console.log(e.message); // No such file or directory
}

// Corrupted file
try {
    const fs = require('fs');
    fs.writeFileSync('bad.crous', 'not valid crous data');
    crous.load('bad.crous');
} catch (e) {
    console.log(e.name);    // 'CrousDecodeError'
    console.log(e.message); // decode error details
}

// Permission denied
try {
    crous.dump({ data: 1 }, '/root/protected.crous');
} catch (e) {
    console.log(e.code);    // 'EACCES'
}`}
      />

      <Callout type="info" title="Synchronous I/O">
        File operations in the Crous Node.js SDK are currently <strong>synchronous</strong>.
        For high-throughput applications, consider running file I/O in a worker thread
        or using the in-memory <code>dumps</code> / <code>loads</code> with your own async
        file handling.
      </Callout>

      <h2>Practical Patterns</h2>

      <h3>Configuration Files</h3>
      <CodeBlock
        filename="config_pattern.js"
        language="javascript"
        code={`const crous = require('crous');
const fs = require('fs');
const path = require('path');

function loadConfig(configPath) {
    if (!fs.existsSync(configPath)) {
        // Return defaults
        return {
            port: 3000,
            debug: false,
            allowedOrigins: ['http://localhost:3000']
        };
    }
    return crous.load(configPath);
}

function saveConfig(config, configPath) {
    // Atomic write: write to temp, then rename
    const tmpPath = configPath + '.tmp';
    crous.dump(config, tmpPath);
    fs.renameSync(tmpPath, configPath);
}

const config = loadConfig('app.crous');
config.debug = true;
saveConfig(config, 'app.crous');`}
      />

      <h3>Data Caching</h3>
      <CodeBlock
        filename="cache_pattern.js"
        language="javascript"
        code={`const crous = require('crous');
const fs = require('fs');

class CrousCache {
    constructor(cacheDir) {
        this.cacheDir = cacheDir;
        if (!fs.existsSync(cacheDir)) {
            fs.mkdirSync(cacheDir, { recursive: true });
        }
    }

    _keyPath(key) {
        // Simple hash for filename
        const hash = Buffer.from(key).toString('hex').slice(0, 16);
        return \`\${this.cacheDir}/\${hash}.crous\`;
    }

    set(key, value, ttlMs = 60_000) {
        const entry = {
            value,
            expires: Date.now() + ttlMs
        };
        crous.dump(entry, this._keyPath(key));
    }

    get(key) {
        const filePath = this._keyPath(key);
        if (!fs.existsSync(filePath)) return null;

        const entry = crous.load(filePath);
        if (Date.now() > entry.expires) {
            fs.unlinkSync(filePath);
            return null;
        }
        return entry.value;
    }
}

const cache = new CrousCache('./cache');
cache.set('users', [{ id: 1, name: 'Alice' }], 30_000);
console.log(cache.get('users')); // [{ id: 1, name: 'Alice' }]`}
      />
    </>
  );
}
