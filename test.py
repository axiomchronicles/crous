import crous

binary = crous.dumps([1, 2, 4])
print(binary)

print(crous.loads(binary))