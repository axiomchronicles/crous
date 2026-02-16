import crous

# CROUT text format (human-readable)
data = {"name": "pawan", "email": "lol", "age": 18, "status": True}
crout_text = crous.dumps_text(data)
print("=== CROUT Text ===")
print(crout_text)
print()

# Decode back
result = crous.loads_text(crout_text)
print("=== Decoded ===")
print(result)