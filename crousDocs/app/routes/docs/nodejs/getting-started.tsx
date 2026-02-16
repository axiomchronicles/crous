import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/getting-started";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Node.js Getting Started Guide - Crous Binary Serialization" },
    { 
      name: "description", 
      content: "Install Crous for Node.js with npm. Quick start guide for binary serialization in JavaScript and TypeScript with N-API native bindings." 
    },
    { 
      name: "keywords", 
      content: "nodejs binary serialization, crous npm, javascript encoding, typescript serialization, napi binary format, nodejs native addon" 
    },
    { property: "og:title", content: "Node.js Getting Started - Crous" },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/nodejs/getting-started" },
  ];
}

export default function GettingStarted() {
  return (
    <>
      <h1>Getting Started</h1>
      <p>
        This guide walks you through installing the Crous Node.js SDK and running
        your first serialization.
      </p>

      <h2>Requirements</h2>
      <ul>
        <li>Node.js 14.0.0 or later</li>
        <li>A C compiler (GCC, Clang, or MSVC)</li>
        <li>Python 3.x (required by node-gyp for building)</li>
        <li>node-gyp build tools</li>
      </ul>

      <h2>Installation</h2>
      <CodeBlock
        code="npm install crous"
        language="bash"
      />

      <Callout type="info" title="Native Compilation">
        The package includes C source code that is compiled during <code>npm install</code> via
        node-gyp. Make sure you have a C compiler available. On macOS, install Xcode Command Line
        Tools. On Ubuntu, install <code>build-essential</code>.
      </Callout>

      <h3>Build from Source</h3>
      <CodeBlock
        code={`git clone https://github.com/axiomchronicles/crous.git
cd crous/nodejs
npm install
npm run build`}
        language="bash"
      />

      <h2>Verify Installation</h2>
      <CodeBlock
        filename="verify.js"
        language="javascript"
        code={`const crous = require('crous');

// Check version
console.log(\`Crous version: \${crous.version}\`);
// Output: Crous version: 2.0.0

// Quick round-trip test
const data = { hello: 'world', numbers: [1, 2, 3] };
const encoded = crous.dumps(data);
const decoded = crous.loads(encoded);
console.log(decoded);
console.log('✓ Crous is working correctly!');`}
      />

      <h2>Your First Serialization</h2>
      <CodeBlock
        filename="first_example.js"
        language="javascript"
        code={`const crous = require('crous');

// Create some data
const user = {
    name: 'Alice',
    age: 30,
    email: 'alice@example.com',
    scores: [98.5, 95.0, 100.0],
    verified: true,
    avatar: Buffer.from([0x89, 0x50, 0x4E, 0x47]),  // binary data!
    tags: new Set(['admin', 'developer']),             // sets too!
};

// Serialize to binary
const binaryData = crous.dumps(user);
console.log(\`Serialized to \${binaryData.length} bytes\`);

// Save to file
crous.dump(user, 'user.crous');

// Load from file
const loaded = crous.load('user.crous');
console.log('✓ File round-trip successful!');
console.log(loaded);`}
      />

      <h2>TypeScript Support</h2>
      <p>
        Full TypeScript definitions are included. No additional <code>@types</code> package needed.
      </p>
      <CodeBlock
        filename="example.ts"
        language="typescript"
        code={`import * as crous from 'crous';

interface User {
    name: string;
    age: number;
}

const user: User = { name: 'Alice', age: 30 };
const binary: Buffer = crous.dumps(user);
const result = crous.loads(binary) as User;`}
      />

      <h2>Platform Support</h2>
      <table>
        <thead>
          <tr>
            <th>Platform</th>
            <th>Architecture</th>
            <th>Status</th>
          </tr>
        </thead>
        <tbody>
          <tr><td>Linux</td><td>x64, ARM64</td><td>✅</td></tr>
          <tr><td>macOS</td><td>x64, ARM64 (Apple Silicon)</td><td>✅</td></tr>
          <tr><td>Windows</td><td>x64</td><td>✅</td></tr>
        </tbody>
      </table>

      <Callout type="success" title="ABI Stability">
        Crous uses N-API (not V8 directly), which is ABI-stable across Node.js versions.
        You won't need to recompile when upgrading Node.js.
      </Callout>
    </>
  );
}
