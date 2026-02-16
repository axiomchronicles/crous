import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/error-handling";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Python Error Handling - Exceptions & Debugging - Crous" },
    { name: "description", content: "Complete guide to error handling in Crous Python SDK. CrousError exceptions, validation, debugging tips, and best practices for robust applications." },
    { name: "keywords", content: "python error handling, exceptions python, validation errors, debugging serialization, error types python, exception handling" },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/python/error-handling" },
  ];
}

export default function ErrorHandling() {
  return (
    <>
      <h1>Error Handling</h1>
      <p>
        Crous uses a hierarchical exception system. All Crous exceptions inherit from
        <code>CrousError</code>, with specialized subclasses for encoding and decoding errors.
      </p>

      <h2>Exception Hierarchy</h2>
      <CodeBlock
        code={`Exception
└── CrousError                # Base exception for all Crous errors
    ├── CrousEncodeError      # Raised during serialization
    └── CrousDecodeError      # Raised during deserialization`}
      />

      <h2>CrousError</h2>
      <p>
        The base exception class. Catch this to handle all Crous-related errors.
      </p>
      <CodeBlock
        filename="catch_all.py"
        code={`import crous

try:
    result = crous.loads(b"invalid data")
except crous.CrousError as e:
    print(f"Crous error: {e}")  # catches both encode and decode errors`}
      />

      <h2>CrousEncodeError</h2>
      <p>
        Raised when serialization fails. Common causes:
      </p>
      <ul>
        <li><strong>Unsupported type</strong> — object type is not supported and no <code>default</code> handler is provided</li>
        <li><strong>Non-string dict keys</strong> — dictionaries with integer, tuple, or other non-string keys</li>
        <li><strong>Integer overflow</strong> — integer value exceeds 64-bit signed range</li>
        <li><strong>Nesting too deep</strong> — structure nesting exceeds 256 levels</li>
        <li><strong>Size limit</strong> — string or bytes exceeds 64 MB</li>
      </ul>
      <CodeBlock
        filename="encode_errors.py"
        code={`import crous

# Unsupported type
try:
    crous.dumps(object())
except crous.CrousEncodeError as e:
    print(f"Encode error: {e}")

# Non-string dict keys
try:
    crous.dumps({1: "value"})
except crous.CrousEncodeError as e:
    print(f"Key error: {e}")

# Integer overflow
try:
    crous.dumps(2**64)
except (crous.CrousEncodeError, OverflowError) as e:
    print(f"Overflow: {e}")`}
      />

      <h2>CrousDecodeError</h2>
      <p>
        Raised when deserialization fails. Common causes:
      </p>
      <ul>
        <li><strong>Truncated data</strong> — binary data is incomplete</li>
        <li><strong>Invalid magic bytes</strong> — data doesn't start with valid format header</li>
        <li><strong>Corrupted data</strong> — bytes are malformed or tampered with</li>
        <li><strong>Invalid UTF-8</strong> — string data contains invalid UTF-8 sequences</li>
        <li><strong>Version mismatch</strong> — wire format version is unsupported</li>
      </ul>
      <CodeBlock
        filename="decode_errors.py"
        code={`import crous

# Truncated data
try:
    crous.loads(b"FLUX\\x01")  # incomplete
except crous.CrousDecodeError as e:
    print(f"Decode error: {e}")

# Invalid data
try:
    crous.loads(b"not crous data")
except crous.CrousDecodeError as e:
    print(f"Invalid: {e}")

# File not found
try:
    crous.load("nonexistent.crous")
except (crous.CrousDecodeError, FileNotFoundError) as e:
    print(f"File error: {e}")`}
      />

      <h2>C-Level Error Codes</h2>
      <p>
        Internally, the C library uses numeric error codes that are mapped to Python exceptions:
      </p>
      <table>
        <thead>
          <tr>
            <th>Code</th>
            <th>Name</th>
            <th>Description</th>
            <th>Critical</th>
          </tr>
        </thead>
        <tbody>
          <tr><td>0</td><td>CROUS_OK</td><td>Success</td><td>—</td></tr>
          <tr><td>1</td><td>CROUS_ERR_OOM</td><td>Out of memory</td><td>✅</td></tr>
          <tr><td>2</td><td>CROUS_ERR_OVERFLOW</td><td>Buffer/integer overflow</td><td>✅</td></tr>
          <tr><td>3</td><td>CROUS_ERR_TYPE</td><td>Unknown/unsupported type</td><td></td></tr>
          <tr><td>4</td><td>CROUS_ERR_INVALID_DATA</td><td>Malformed or corrupt data</td><td></td></tr>
          <tr><td>5</td><td>CROUS_ERR_IO</td><td>I/O failure</td><td></td></tr>
          <tr><td>6</td><td>CROUS_ERR_DEPTH</td><td>Max nesting depth exceeded</td><td></td></tr>
          <tr><td>7</td><td>CROUS_ERR_SIZE</td><td>Size limit exceeded</td><td></td></tr>
          <tr><td>8</td><td>CROUS_ERR_UTF8</td><td>Invalid UTF-8 encoding</td><td></td></tr>
          <tr><td>9</td><td>CROUS_ERR_VERSION</td><td>Incompatible version</td><td></td></tr>
          <tr><td>10</td><td>CROUS_ERR_UNSUPPORTED</td><td>Unsupported operation</td><td></td></tr>
          <tr><td>11</td><td>CROUS_ERR_INTERNAL</td><td>Internal error</td><td>✅</td></tr>
          <tr><td>12</td><td>CROUS_ERR_PARSE</td><td>Parse error</td><td></td></tr>
        </tbody>
      </table>

      <Callout type="danger" title="Critical Errors">
        Error codes marked as "Critical" (OOM, OVERFLOW, INTERNAL) indicate severe failures
        that may leave the library in an inconsistent state. These should be treated as
        unrecoverable.
      </Callout>

      <h2>Best Practices</h2>
      <CodeBlock
        filename="best_practices.py"
        code={`import crous

def safe_serialize(data, filepath=None):
    """Serialize data with comprehensive error handling."""
    try:
        if filepath:
            crous.dump(data, filepath)
        else:
            return crous.dumps(data)
    except crous.CrousEncodeError as e:
        # Handle serialization-specific errors
        print(f"Cannot serialize: {e}")
        raise
    except crous.CrousError as e:
        # Handle any other Crous error
        print(f"Crous error: {e}")
        raise

def safe_deserialize(source):
    """Deserialize data with comprehensive error handling."""
    try:
        if isinstance(source, (str, bytes)) and not isinstance(source, bytes):
            return crous.load(source)
        else:
            return crous.loads(source)
    except crous.CrousDecodeError as e:
        print(f"Cannot deserialize: {e}")
        raise
    except FileNotFoundError:
        print(f"File not found: {source}")
        raise
    except crous.CrousError as e:
        print(f"Crous error: {e}")
        raise`}
      />

      <h2>Validating Data</h2>
      <p>
        Use the version module to validate binary data before decoding:
      </p>
      <CodeBlock
        filename="validation.py"
        code={`from crous.version import check_compatibility, Header

binary_data = b"FLUX\\x01\\x00..."

# Parse and validate header
header = Header.parse(binary_data)
if header.is_valid:
    print(f"Valid FLUX data, version {header.wire_version}")

# Check compatibility
result = check_compatibility(binary_data)
if result.is_compatible:
    data = crous.loads(binary_data)
elif result.is_warning:
    print(f"Warning: {result.message}")
    data = crous.loads(binary_data)  # try anyway
else:
    print(f"Incompatible: {result.message}")`}
      />
    </>
  );
}
