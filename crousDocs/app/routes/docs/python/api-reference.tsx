import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/api-reference";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Python API Reference - Crous Binary Serialization" },
    { 
      name: "description", 
      content: "Complete Python API reference for Crous. Functions, classes, and methods for encoding, decoding, streaming, and error handling with type hints." 
    },
    { 
      name: "keywords", 
      content: "python api reference, crous api docs, python serialization api, encoding functions python, binary format api, python type hints" 
    },
    { property: "og:title", content: "Python API Reference - Crous" },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/python/api-reference" },
  ];
}

export default function ApiReference() {
  return (
    <>
      <h1>API Reference</h1>
      <p>
        Complete reference for all public functions, classes, and constants in the
        <code> crous</code> Python module.
      </p>

      <h2>Core Functions</h2>

      <h3>crous.dumps(obj, default=None)</h3>
      <p>Serialize a Python object to FLUX binary bytes.</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>obj</code></td><td><code>Any</code></td><td>The Python object to serialize</td></tr>
          <tr><td><code>default</code></td><td><code>Callable[[Any], Any] | None</code></td><td>Fallback handler for unsupported types</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>bytes</code> — FLUX binary data</p>
      <p><strong>Raises:</strong> <code>CrousEncodeError</code> — if serialization fails</p>

      <h3>crous.loads(data, object_hook=None)</h3>
      <p>Deserialize FLUX/CROUS binary bytes to a Python object.</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>data</code></td><td><code>bytes</code></td><td>Binary data to deserialize</td></tr>
          <tr><td><code>object_hook</code></td><td><code>Callable[[dict], Any] | None</code></td><td>Transform decoded dictionaries</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>Any</code> — Deserialized Python object</p>
      <p><strong>Raises:</strong> <code>CrousDecodeError</code> — if deserialization fails</p>

      <h3>crous.dump(obj, file, default=None)</h3>
      <p>Serialize and write to a file path or file object.</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>obj</code></td><td><code>Any</code></td><td>The Python object to serialize</td></tr>
          <tr><td><code>file</code></td><td><code>str | IO[bytes]</code></td><td>File path or writable binary file object</td></tr>
          <tr><td><code>default</code></td><td><code>Callable | None</code></td><td>Fallback handler for unsupported types</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>None</code></p>
      <p><strong>Raises:</strong> <code>CrousEncodeError</code>, <code>IOError</code></p>

      <h3>crous.load(file, object_hook=None)</h3>
      <p>Read and deserialize from a file path or file object.</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>file</code></td><td><code>str | IO[bytes]</code></td><td>File path or readable binary file object</td></tr>
          <tr><td><code>object_hook</code></td><td><code>Callable[[dict], Any] | None</code></td><td>Transform decoded dictionaries</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>Any</code> — Deserialized Python object</p>
      <p><strong>Raises:</strong> <code>CrousDecodeError</code>, <code>FileNotFoundError</code></p>

      <h2>Stream Functions</h2>

      <h3>crous.dump_to_stream(obj, stream, default=None)</h3>
      <p>Serialize and write to a writable stream.</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>obj</code></td><td><code>Any</code></td><td>Object to serialize</td></tr>
          <tr><td><code>stream</code></td><td><code>IO[bytes]</code></td><td>Any object with a <code>write()</code> method</td></tr>
          <tr><td><code>default</code></td><td><code>Callable | None</code></td><td>Fallback handler</td></tr>
        </tbody>
      </table>

      <h3>crous.load_from_stream(stream, object_hook=None)</h3>
      <p>Read and deserialize from a readable stream.</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>stream</code></td><td><code>IO[bytes]</code></td><td>Any object with a <code>read()</code> method</td></tr>
          <tr><td><code>object_hook</code></td><td><code>Callable | None</code></td><td>Transform decoded dictionaries</td></tr>
        </tbody>
      </table>

      <h2>CROUT Functions</h2>

      <h3>crous.to_crout(obj)</h3>
      <p>Serialize a Python object to CROUT text format.</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>obj</code></td><td><code>Any</code></td><td>Object to serialize</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>str</code> — CROUT text representation</p>

      <h3>crous.from_crout(text)</h3>
      <p>Deserialize CROUT text to a Python object.</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>text</code></td><td><code>str</code></td><td>CROUT text data</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>Any</code> — Deserialized Python object</p>

      <h3>crous.crout_to_flux(text)</h3>
      <p>Convert CROUT text directly to FLUX binary (no Python intermediary).</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>text</code></td><td><code>str</code></td><td>CROUT text data</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>bytes</code> — FLUX binary data</p>

      <h3>crous.flux_to_crout(data)</h3>
      <p>Convert FLUX binary directly to CROUT text (no Python intermediary).</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>data</code></td><td><code>bytes</code></td><td>FLUX binary data</td></tr>
        </tbody>
      </table>
      <p><strong>Returns:</strong> <code>str</code> — CROUT text representation</p>

      <h2>Custom Serializer Registry</h2>

      <h3>crous.register_serializer(type, serializer, tag=None)</h3>
      <p>Register a custom serializer for a Python type.</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>type</code></td><td><code>type</code></td><td>The Python class to register</td></tr>
          <tr><td><code>serializer</code></td><td><code>Callable[[Any], Any]</code></td><td>Function that converts instance to serializable value</td></tr>
          <tr><td><code>tag</code></td><td><code>int | None</code></td><td>Optional explicit tag number (auto-assigned from 100 if omitted)</td></tr>
        </tbody>
      </table>

      <h3>crous.unregister_serializer(type)</h3>
      <p>Remove a previously registered serializer.</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>type</code></td><td><code>type</code></td><td>The Python class to unregister</td></tr>
        </tbody>
      </table>

      <h3>crous.register_decoder(tag, decoder)</h3>
      <p>Register a custom decoder for a tagged value.</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>tag</code></td><td><code>int</code></td><td>The tag number to decode</td></tr>
          <tr><td><code>decoder</code></td><td><code>Callable[[Any], Any]</code></td><td>Function that reconstructs the custom type</td></tr>
        </tbody>
      </table>

      <h3>crous.unregister_decoder(tag)</h3>
      <p>Remove a previously registered decoder.</p>
      <table>
        <thead><tr><th>Parameter</th><th>Type</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>tag</code></td><td><code>int</code></td><td>The tag number to unregister</td></tr>
        </tbody>
      </table>

      <h2>Classes</h2>

      <h3>crous.CrousEncoder</h3>
      <p>Reusable encoder with persistent options.</p>
      <CodeBlock
        code={`class CrousEncoder:
    def __init__(self, default=None, allow_custom=True):
        """
        Args:
            default: Fallback handler for unsupported types
            allow_custom: Whether to use registered custom serializers
        """

    def encode(self, obj) -> bytes:
        """Serialize obj to FLUX binary bytes."""`}
      />

      <h3>crous.CrousDecoder</h3>
      <p>Reusable decoder with persistent options.</p>
      <CodeBlock
        code={`class CrousDecoder:
    def __init__(self, object_hook=None):
        """
        Args:
            object_hook: Transform function for decoded dictionaries
        """

    def decode(self, data: bytes) -> Any:
        """Deserialize FLUX binary bytes to Python object."""`}
      />

      <h2>Exceptions</h2>

      <h3>crous.CrousError</h3>
      <p>Base exception for all Crous errors. Inherits from <code>Exception</code>.</p>

      <h3>crous.CrousEncodeError</h3>
      <p>Raised during serialization. Inherits from <code>CrousError</code>.</p>

      <h3>crous.CrousDecodeError</h3>
      <p>Raised during deserialization. Inherits from <code>CrousError</code>.</p>

      <h2>Module Attributes</h2>

      <table>
        <thead>
          <tr><th>Attribute</th><th>Type</th><th>Value</th><th>Description</th></tr>
        </thead>
        <tbody>
          <tr><td><code>crous.__version__</code></td><td><code>str</code></td><td><code>"2.0.0"</code></td><td>Library version string</td></tr>
          <tr><td><code>crous.__version_tuple__</code></td><td><code>tuple</code></td><td><code>(2, 0, 0)</code></td><td>Version as tuple</td></tr>
        </tbody>
      </table>

      <h2>crous.version Module</h2>
      <p>
        The <code>crous.version</code> submodule provides detailed version and compatibility utilities.
      </p>

      <h3>Constants</h3>
      <table>
        <thead><tr><th>Constant</th><th>Value</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>VERSION_MAJOR</code></td><td><code>2</code></td><td>Major version</td></tr>
          <tr><td><code>VERSION_MINOR</code></td><td><code>0</code></td><td>Minor version</td></tr>
          <tr><td><code>VERSION_PATCH</code></td><td><code>0</code></td><td>Patch version</td></tr>
          <tr><td><code>VERSION_STRING</code></td><td><code>"2.0.0"</code></td><td>Version string</td></tr>
          <tr><td><code>VERSION_HEX</code></td><td><code>0x020000</code></td><td>Version as hex</td></tr>
          <tr><td><code>WIRE_VERSION_CURRENT</code></td><td><code>2</code></td><td>Current wire format version</td></tr>
          <tr><td><code>WIRE_VERSION_MIN_READ</code></td><td><code>1</code></td><td>Minimum readable wire version</td></tr>
          <tr><td><code>WIRE_VERSION_MAX_READ</code></td><td><code>2</code></td><td>Maximum readable wire version</td></tr>
        </tbody>
      </table>

      <h3>Feature Flags (IntFlag)</h3>
      <table>
        <thead><tr><th>Flag</th><th>Bit</th><th>Description</th></tr></thead>
        <tbody>
          <tr><td><code>Feature.NONE</code></td><td>0</td><td>No features</td></tr>
          <tr><td><code>Feature.TAGGED</code></td><td>1</td><td>Tagged values</td></tr>
          <tr><td><code>Feature.TUPLE</code></td><td>2</td><td>Native tuple support</td></tr>
          <tr><td><code>Feature.SET</code></td><td>4</td><td>Set type</td></tr>
          <tr><td><code>Feature.FROZENSET</code></td><td>8</td><td>Frozenset type</td></tr>
          <tr><td><code>Feature.COMPRESSION</code></td><td>16</td><td>Compression</td></tr>
          <tr><td><code>Feature.STREAMING</code></td><td>32</td><td>Streaming mode</td></tr>
          <tr><td><code>Feature.SCHEMA</code></td><td>64</td><td>Schema validation</td></tr>
          <tr><td><code>Feature.ENCRYPTION</code></td><td>128</td><td>Encryption</td></tr>
          <tr><td><code>Feature.DATETIME</code></td><td>256</td><td>Datetime support</td></tr>
          <tr><td><code>Feature.DECIMAL</code></td><td>512</td><td>Decimal support</td></tr>
          <tr><td><code>Feature.UUID</code></td><td>1024</td><td>UUID support</td></tr>
          <tr><td><code>Feature.PATH</code></td><td>2048</td><td>Path support</td></tr>
          <tr><td><code>Feature.COMMENTS</code></td><td>4096</td><td>Comments</td></tr>
          <tr><td><code>Feature.METADATA</code></td><td>8192</td><td>Metadata</td></tr>
          <tr><td><code>Feature.CHECKSUM</code></td><td>16384</td><td>Checksums</td></tr>
        </tbody>
      </table>

      <h3>SemanticVersion</h3>
      <CodeBlock
        code={`from crous.version import SemanticVersion

# Create from string
v = SemanticVersion.parse("2.0.0")

# Get current library version
v = SemanticVersion.current()

# Comparison operators
v1 = SemanticVersion.parse("1.0.0")
v2 = SemanticVersion.parse("2.0.0")
assert v1 < v2

# Constraint checking (supports >=, >, <=, <, ==, !=, ^, ~)
assert v2.satisfies(">=1.0.0")
assert v2.satisfies("^2.0.0")
assert v2.satisfies("~2.0")

# Bump versions
v3 = v2.bump_minor()   # 2.1.0
v4 = v2.bump_major()   # 3.0.0`}
      />

      <h3>Header</h3>
      <CodeBlock
        code={`from crous.version import Header

header = Header.parse(binary_data)
header.magic         # b'FLUX'
header.wire_version  # 1
header.flags         # 0
header.is_valid      # True if magic == b'FLUX'`}
      />

      <h3>check_compatibility(data)</h3>
      <CodeBlock
        code={`from crous.version import check_compatibility

result = check_compatibility(binary_data)
result.status         # Compatibility enum value
result.is_compatible  # True/False
result.is_warning     # True if warnings
result.is_error       # True if incompatible
result.message        # Human-readable description`}
      />

      <h2>Serializable Types</h2>
      <CodeBlock
        code={`# Type alias for all natively serializable types
CrousSerializable = Union[
    None,
    bool,
    int,
    float,
    str,
    bytes,
    List['CrousSerializable'],
    Tuple['CrousSerializable', ...],
    Dict[str, 'CrousSerializable'],
    Set['CrousSerializable'],
    FrozenSet['CrousSerializable'],
]`}
      />
    </>
  );
}
