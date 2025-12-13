import crous

# Example: Serialize and deserialize a Python list
data = [1, 2, 4]
print(f"Original: {data}")

# Serialize to FLUX binary format
binary = crous.dumps(data)
print(f"Encoded (FLUX): {binary}")
print(f"Encoded size: {len(binary)} bytes")
print(f"Magic bytes: {binary[:4]}")  # Should be b'FLUX'

# Deserialize back to Python object
decoded = crous.loads(binary)
print(f"Decoded: {decoded}")
print(f"Match: {data == decoded}")
