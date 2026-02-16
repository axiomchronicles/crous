import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/custom-types";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Node.js Custom Types - Extend Serialization - Crous" },
    { name: "description", content: "Extend Crous with custom serializers and decoders in JavaScript and TypeScript. Support Date, Map, Set, and domain-specific types." },
    { name: "keywords", content: "nodejs custom types, javascript serializers, typescript encoders, custom encoding nodejs, date serialization, map encoding" },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/nodejs/custom-types" },
  ];
}

export default function CustomTypes() {
  return (
    <>
      <h1>Custom Types</h1>
      <p>
        The Crous tagged type system lets you register custom serializers and decoders
        for your own classes and objects. This enables type-preserving round-trips for
        application-specific types.
      </p>

      <h2>How It Works</h2>
      <p>
        Crous uses numeric <strong>tags</strong> (integers ≥ 100) to identify custom types
        in the binary stream. You register a serializer that converts your object to a
        Crous-native type, and a decoder that reconstructs it.
      </p>
      <div className="my-6 p-5 bg-tubox-card border border-tubox-border rounded-xl font-mono text-sm">
        <div className="text-zinc-400 mb-2">Wire format for custom types:</div>
        <div className="text-crous-400">
          TAGGED → [tag: varint] [inner_type] [inner_data]
        </div>
      </div>

      <h2>Registering a Serializer</h2>
      <CodeBlock
        filename="register_serializer.js"
        language="javascript"
        code={`const crous = require('crous');

class Vector3 {
    constructor(x, y, z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }
}

// Register a serializer for Vector3
// The tag is auto-assigned starting from 100
crous.registerSerializer(
    Vector3,                         // constructor to match
    (vec) => [vec.x, vec.y, vec.z]   // serialize to array
);

// Now Vector3 instances can be serialized!
const data = { position: new Vector3(1, 2, 3) };
const buffer = crous.dumps(data);
console.log(\`Encoded: \${buffer.length} bytes\`);`}
      />

      <Callout type="info" title="Auto-Tag Assignment">
        Tags are automatically assigned starting from 100, incrementing for each new
        registration. The same tag must be used for both serializer and decoder registration
        to ensure correct round-trips.
      </Callout>

      <h2>Registering a Decoder</h2>
      <CodeBlock
        filename="register_decoder.js"
        language="javascript"
        code={`const crous = require('crous');

class Vector3 {
    constructor(x, y, z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }
    magnitude() {
        return Math.sqrt(this.x ** 2 + this.y ** 2 + this.z ** 2);
    }
}

// Register decoder for tag 100 (the first custom type)
crous.registerDecoder(
    100,                                       // tag number
    (arr) => new Vector3(arr[0], arr[1], arr[2])  // reconstruct from array
);

// Full round-trip
crous.registerSerializer(Vector3, (v) => [v.x, v.y, v.z]);

const original = new Vector3(3, 4, 0);
const buffer = crous.dumps(original);
const restored = crous.loads(buffer);

console.log(restored instanceof Vector3); // true
console.log(restored.magnitude());        // 5`}
      />

      <h2>Complete Round-Trip Example</h2>
      <CodeBlock
        filename="roundtrip.js"
        language="javascript"
        code={`const crous = require('crous');

// ─── Define custom types ─────────────────────────

class Color {
    constructor(r, g, b, a = 1.0) {
        this.r = r; this.g = g;
        this.b = b; this.a = a;
    }
    toHex() {
        const hex = (v) => Math.round(v * 255).toString(16).padStart(2, '0');
        return \`#\${hex(this.r)}\${hex(this.g)}\${hex(this.b)}\`;
    }
}

class Transform {
    constructor(position, rotation, scale) {
        this.position = position;
        this.rotation = rotation;
        this.scale = scale;
    }
}

// ─── Register serializers ────────────────────────

crous.registerSerializer(Color, (c) => ({
    r: c.r, g: c.g, b: c.b, a: c.a
}));

crous.registerSerializer(Transform, (t) => ({
    pos: t.position,
    rot: t.rotation,
    scl: t.scale
}));

// ─── Register decoders ──────────────────────────

crous.registerDecoder(100, (d) =>
    new Color(d.r, d.g, d.b, d.a)
);

crous.registerDecoder(101, (d) =>
    new Transform(d.pos, d.rot, d.scl)
);

// ─── Use them! ──────────────────────────────────

const scene = {
    background: new Color(0.1, 0.1, 0.2),
    player: {
        color: new Color(1, 0, 0),
        transform: new Transform(
            [0, 1, 0],    // position
            [0, 0, 0, 1], // rotation (quaternion)
            [1, 1, 1]     // scale
        )
    }
};

const buffer = crous.dumps(scene);
const loaded = crous.loads(buffer);

console.log(loaded.background.toHex());           // #1a1a33
console.log(loaded.player.color instanceof Color); // true
console.log(loaded.player.transform.position);     // [0, 1, 0]`}
      />

      <h2>Unregistering Types</h2>
      <CodeBlock
        filename="unregister.js"
        language="javascript"
        code={`const crous = require('crous');

class Temporary {
    constructor(val) { this.val = val; }
}

// Register
crous.registerSerializer(Temporary, (t) => t.val);
crous.registerDecoder(100, (v) => new Temporary(v));

// Later, unregister
crous.unregisterSerializer(Temporary);
crous.unregisterDecoder(100);

// Now Temporary objects will throw CrousEncodeError
// and tag 100 will throw CrousDecodeError`}
      />

      <h2>Constructor-Based Lookup</h2>
      <p>
        Serializer registration uses the object's <code>constructor</code> for matching.
        This means subclasses need their own registration unless you want them to use
        the parent's serializer.
      </p>

      <CodeBlock
        filename="inheritance.js"
        language="javascript"
        code={`const crous = require('crous');

class Shape {
    constructor(type) { this.type = type; }
}

class Circle extends Shape {
    constructor(radius) {
        super('circle');
        this.radius = radius;
    }
}

// Only registering Shape
crous.registerSerializer(Shape, (s) => ({ type: s.type }));

// Circle won't match — it has a different constructor!
try {
    crous.dumps(new Circle(5));
} catch (e) {
    console.log(e.message); // encode error
}

// Register Circle separately
crous.registerSerializer(Circle, (c) => ({
    type: c.type, radius: c.radius
}));

crous.dumps(new Circle(5)); // ✓ works now`}
      />

      <Callout type="warning" title="No MRO Lookup">
        Unlike the Python SDK, the Node.js SDK does <strong>not</strong> walk the prototype
        chain (MRO) to find a matching serializer. Each constructor must be registered
        individually.
      </Callout>

      <h2>Tag Numbering</h2>
      <table>
        <thead>
          <tr>
            <th>Tag Range</th>
            <th>Purpose</th>
          </tr>
        </thead>
        <tbody>
          <tr><td>0–89</td><td>Reserved — core Crous types</td></tr>
          <tr><td>90</td><td>Set (built-in tagged type)</td></tr>
          <tr><td>91–99</td><td>Reserved for future built-in types</td></tr>
          <tr><td>100+</td><td>User-defined custom types</td></tr>
        </tbody>
      </table>

      <Callout type="danger" title="Tag Consistency">
        When exchanging custom-typed data between processes or services, ensure the same
        tag numbers map to the same types on both sides. Tag assignments are per-process
        and not stored in the binary format's header.
      </Callout>

      <h2>Cross-SDK Custom Types</h2>
      <p>
        Custom types are fully interoperable between the Python and Node.js SDKs. As
        long as both sides register the same tag number with compatible serialization
        formats, data flows seamlessly.
      </p>

      <CodeBlock
        filename="cross_sdk.js"
        language="javascript"
        code={`// Node.js side — reading data written by Python
const crous = require('crous');

// Python registered: crous.register_serializer(
//     datetime, 100, lambda dt: dt.isoformat()
// )

// Node.js decoder for the same tag
crous.registerDecoder(100, (isoString) => new Date(isoString));

// Load a file written by Python
const data = crous.load('timestamps.crous');
console.log(data.created instanceof Date); // true`}
      />
    </>
  );
}
