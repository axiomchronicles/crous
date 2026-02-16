import { CodeBlock } from "~/components/CodeBlock";
import { Callout } from "~/components/Callout";
import type { Route } from "./+types/streaming";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Python Streaming API - Large Data Serialization - Crous" },
    { name: "description", content: "Stream-based binary serialization in Python with Crous. Encode and decode large datasets incrementally with dump() and load() for memory efficiency." },
    { name: "keywords", content: "python streaming, memory efficient serialization, incremental encoding, large data python, stream api, file streaming" },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs/python/streaming" },
  ];
}

export default function Streaming() {
  return (
    <>
      <h1>Streaming</h1>
      <p>
        Crous supports stream-based I/O for writing to and reading from any file-like object,
        including files, sockets, <code>io.BytesIO</code>, and network streams.
      </p>

      <h2>Stream Writing with dump_to_stream</h2>
      <p>
        Write serialized binary data to any writable stream (any object with a <code>write()</code> method).
      </p>
      <CodeBlock
        filename="stream_write.py"
        code={`import crous
import io

data = {"name": "Alice", "scores": [98, 95, 100]}

# Write to BytesIO
buffer = io.BytesIO()
crous.dump_to_stream(data, buffer)

# Check the result
buffer.seek(0)
print(f"Written {len(buffer.getvalue())} bytes")`}
      />

      <h2>Stream Reading with load_from_stream</h2>
      <p>
        Read and deserialize from any readable stream (any object with a <code>read()</code> method).
      </p>
      <CodeBlock
        filename="stream_read.py"
        code={`import crous
import io

# Write data
data = {"key": "value"}
buffer = io.BytesIO()
crous.dump_to_stream(data, buffer)

# Read it back
buffer.seek(0)
result = crous.load_from_stream(buffer)
assert result == data`}
      />

      <h2>File Streams</h2>
      <p>
        The <code>dump</code> and <code>load</code> functions accept both file paths and file objects:
      </p>
      <CodeBlock
        filename="file_streams.py"
        code={`import crous

data = {"users": [{"name": "Alice"}, {"name": "Bob"}]}

# String path — Crous handles opening/closing
crous.dump(data, "output.crous")
result = crous.load("output.crous")

# File object — you manage the lifecycle
with open("output.crous", "wb") as f:
    crous.dump(data, f)

with open("output.crous", "rb") as f:
    result = crous.load(f)`}
      />

      <h2>Multi-Record Streaming</h2>
      <p>
        You can write multiple records to a single stream. Each record includes its own
        header, so they can be read back sequentially.
      </p>
      <CodeBlock
        filename="multi_record.py"
        code={`import crous
import io

buffer = io.BytesIO()

# Write multiple records
records = [
    {"id": 1, "name": "Alice"},
    {"id": 2, "name": "Bob"},
    {"id": 3, "name": "Charlie"},
]

for record in records:
    crous.dump_to_stream(record, buffer)

# Read them back
buffer.seek(0)
for _ in range(len(records)):
    record = crous.load_from_stream(buffer)
    print(record)`}
      />

      <Callout type="info" title="Stream Position">
        When reading multiple records, the stream position advances after each
        <code> load_from_stream</code> call. Make sure to seek to position 0 if you want
        to re-read from the beginning.
      </Callout>

      <h2>Network Sockets</h2>
      <p>
        Since Crous works with any file-like object, you can use it with sockets wrapped
        in <code>socket.makefile()</code>:
      </p>
      <CodeBlock
        filename="socket_example.py"
        code={`import crous
import socket

# Server side
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(('localhost', 9999))
server.listen(1)
conn, addr = server.accept()

# Wrap socket as file object
stream = conn.makefile('rwb')

# Read a message
data = crous.load_from_stream(stream)
print(f"Received: {data}")

# Send a response
crous.dump_to_stream({"status": "ok"}, stream)
stream.flush()  # important for sockets!`}
      />

      <Callout type="warning" title="Buffering">
        When using sockets, remember to call <code>flush()</code> after writing to ensure
        data is actually sent. Crous does not automatically flush the stream.
      </Callout>

      <h2>BytesIO Convenience</h2>
      <p>
        For in-memory operations, <code>io.BytesIO</code> works seamlessly:
      </p>
      <CodeBlock
        code={`import crous
import io

# dumps/loads is simpler for pure in-memory use
binary = crous.dumps(data)        # returns bytes directly
result = crous.loads(binary)      # accepts bytes directly

# dump_to_stream/load_from_stream for stream-based use
buf = io.BytesIO()
crous.dump_to_stream(data, buf)   # writes to stream
buf.seek(0)
result = crous.load_from_stream(buf)  # reads from stream`}
      />
    </>
  );
}
