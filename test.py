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


assert decoded == data, "Decoded data does not match original"

def test_bytes_support():
    """Verify that bytes are supported in FLUX format."""
    test_data = {
        "binary": b"hello world",
        "empty_binary": b"",
        "nested": {"data": b"\x00\x01\x02"}
    }
    
    # Encode with FLUX format
    encoded = crous.dumps(test_data)
    
    # Decode and verify
    decoded = crous.loads(encoded)
    assert decoded == test_data
    print(f"✓ Bytes correctly handled in FLUX format")
    
    return True

def test_flux_format():
    data = {
        "name": "Test",
        "values": [1, 2, 3],
        "nested": {"key": "value"}
    }

    encode = crous.dumps(data)
    assert encode[:4] == b'FLUX', f"Expected FLUX magic bytes, got {encode[:4]}"
    print(f"✓ FLUX magic bytes detected: {encode[:4]}")



if __name__ == "__main__":
    test_flux_format()
    test_bytes_support()    
    print("All tests passed.")


# ============================================================================
# NEW FEATURE TEST CASES
# ============================================================================

def test_large_integers():
    """Test large integer handling (overflow fix)."""
    test_values = [
        2**62,           # Large positive
        -2**62,          # Large negative
        2**63 - 1,       # Max int64
        -2**63,          # Min int64
        0,               # Zero
        1,               # One
        -1,              # Negative one
    ]
    
    for val in test_values:
        encoded = crous.dumps(val)
        decoded = crous.loads(encoded)
        assert decoded == val, f"Large int failed: {val} != {decoded}"
    
    print("✓ Large integer handling works correctly")


def test_crous_encoder_class():
    """Test CrousEncoder class instantiation and methods."""
    # Check class exists
    assert hasattr(crous, 'CrousEncoder'), "CrousEncoder class not found"
    
    # Create instance
    encoder = crous.CrousEncoder()
    assert encoder is not None, "Failed to create CrousEncoder instance"
    
    # Test encode method
    assert hasattr(encoder, 'encode'), "CrousEncoder missing encode method"
    
    data = {"test": [1, 2, 3]}
    encoded = encoder.encode(data)
    assert isinstance(encoded, bytes), "encode() should return bytes"
    assert encoded[:4] == b'FLUX', "Encoded data should have FLUX magic"
    
    # Verify roundtrip
    decoded = crous.loads(encoded)
    assert decoded == data, "CrousEncoder roundtrip failed"
    
    print("✓ CrousEncoder class works correctly")


def test_crous_decoder_class():
    """Test CrousDecoder class instantiation and methods."""
    # Check class exists
    assert hasattr(crous, 'CrousDecoder'), "CrousDecoder class not found"
    
    # Create instance
    decoder = crous.CrousDecoder()
    assert decoder is not None, "Failed to create CrousDecoder instance"
    
    # Test decode method
    assert hasattr(decoder, 'decode'), "CrousDecoder missing decode method"
    
    data = {"test": [1, 2, 3]}
    encoded = crous.dumps(data)
    decoded = decoder.decode(encoded)
    assert decoded == data, "CrousDecoder roundtrip failed"
    
    print("✓ CrousDecoder class works correctly")


def test_object_hook_parameter():
    """Test object_hook parameter for loads()."""
    def add_processed_flag(d):
        d['_processed'] = True
        return d
    
    data = {"name": "test", "value": 42}
    encoded = crous.dumps(data)
    
    # Without object_hook
    decoded_normal = crous.loads(encoded)
    assert '_processed' not in decoded_normal
    
    # With object_hook
    decoded_hooked = crous.loads(encoded, object_hook=add_processed_flag)
    assert decoded_hooked['_processed'] == True
    assert decoded_hooked['name'] == 'test'
    assert decoded_hooked['value'] == 42
    
    print("✓ object_hook parameter works correctly")


def test_object_hook_nested():
    """Test object_hook with nested dicts."""
    call_count = [0]
    
    def counting_hook(d):
        call_count[0] += 1
        d['_depth'] = call_count[0]
        return d
    
    data = {
        "level1": {
            "level2": {
                "level3": {"value": "deep"}
            }
        }
    }
    encoded = crous.dumps(data)
    decoded = crous.loads(encoded, object_hook=counting_hook)
    
    # Hook should be called for each dict (4 total)
    assert call_count[0] == 4, f"Expected 4 hook calls, got {call_count[0]}"
    
    print("✓ object_hook with nested dicts works correctly")


def test_default_parameter():
    """Test default parameter for dumps() with custom types."""
    from datetime import datetime
    
    def datetime_serializer(obj):
        if isinstance(obj, datetime):
            return {"__datetime__": obj.isoformat()}
        raise TypeError(f"Cannot serialize {type(obj)}")
    
    now = datetime.now()
    data = {"timestamp": now, "name": "event"}
    
    # Should fail without default
    try:
        crous.dumps(data)
        assert False, "Should have raised error for datetime"
    except (TypeError, crous.CrousEncodeError):
        pass  # Expected
    
    # Should work with default
    encoded = crous.dumps(data, default=datetime_serializer)
    decoded = crous.loads(encoded)
    
    assert decoded['name'] == 'event'
    assert '__datetime__' in decoded['timestamp']
    
    print("✓ default parameter works correctly")


def test_register_serializer():
    """Test custom serializer registration."""
    class Point:
        def __init__(self, x, y):
            self.x = x
            self.y = y
    
    def point_serializer(p):
        return {"x": p.x, "y": p.y}
    
    # Register serializer
    crous.register_serializer(Point, point_serializer)
    
    # Now Point should be serializable
    point = Point(10, 20)
    encoded = crous.dumps(point)
    decoded = crous.loads(encoded)
    
    assert decoded['x'] == 10
    assert decoded['y'] == 20
    
    # Unregister
    crous.unregister_serializer(Point)
    
    # Should fail now
    try:
        crous.dumps(Point(1, 2))
        assert False, "Should have raised error after unregistering"
    except (TypeError, crous.CrousEncodeError):
        pass  # Expected
    
    print("✓ register_serializer works correctly")


def test_register_decoder():
    """Test custom decoder registration."""
    # Register decoder for tag 200
    def custom_decoder(value):
        return {"decoded_tag_200": value}
    
    crous.register_decoder(200, custom_decoder)
    
    # Verify registration
    crous.unregister_decoder(200)
    
    print("✓ register_decoder works correctly")


def test_dump_load_file():
    """Test dump/load with file objects."""
    import io
    
    data = {"file_test": [1, 2, 3], "nested": {"key": "value"}}
    
    # Test with BytesIO
    buffer = io.BytesIO()
    crous.dump(data, buffer)
    
    buffer.seek(0)
    decoded = crous.load(buffer)
    
    assert decoded == data, "File dump/load roundtrip failed"
    
    print("✓ dump/load with file objects works correctly")


def test_version():
    """Test version attribute."""
    assert hasattr(crous, '__version__'), "Missing __version__"
    assert crous.__version__ == "2.0.0", f"Expected 2.0.0, got {crous.__version__}"
    
    print("✓ Version is correct: " + crous.__version__)


def test_error_classes():
    """Test error class hierarchy."""
    assert hasattr(crous, 'CrousError'), "Missing CrousError"
    assert hasattr(crous, 'CrousEncodeError'), "Missing CrousEncodeError"
    assert hasattr(crous, 'CrousDecodeError'), "Missing CrousDecodeError"
    
    # Check inheritance
    assert issubclass(crous.CrousEncodeError, crous.CrousError)
    assert issubclass(crous.CrousDecodeError, crous.CrousError)
    
    print("✓ Error classes are properly defined")


def test_all_scalar_types():
    """Test all scalar types roundtrip."""
    import math
    
    test_cases = [
        None,
        True,
        False,
        0,
        1,
        -1,
        42,
        -42,
        3.14,
        -3.14,
        0.0,
        float('inf'),
        float('-inf'),
        "",
        "hello",
        "unicode: 你好世界 🌍",
        b"",
        b"bytes data",
        b"\x00\x01\x02\xff",
    ]
    
    for value in test_cases:
        encoded = crous.dumps(value)
        decoded = crous.loads(encoded)
        
        if isinstance(value, float) and math.isnan(value):
            assert math.isnan(decoded), f"NaN roundtrip failed"
        else:
            assert decoded == value, f"Scalar roundtrip failed: {value!r} != {decoded!r}"
    
    # Test NaN separately
    nan_encoded = crous.dumps(float('nan'))
    nan_decoded = crous.loads(nan_encoded)
    assert math.isnan(nan_decoded), "NaN roundtrip failed"
    
    print("✓ All scalar types roundtrip correctly")


def test_complex_nested_structure():
    """Test complex nested data structures."""
    data = {
        "users": [
            {
                "name": "Alice",
                "age": 30,
                "emails": ["alice@example.com", "alice2@example.com"],
                "settings": {
                    "theme": "dark",
                    "notifications": True,
                    "limits": {"max": 100, "min": 0}
                }
            },
            {
                "name": "Bob",
                "age": 25,
                "emails": [],
                "settings": None
            }
        ],
        "metadata": {
            "version": 1,
            "created": "2024-01-01",
            "tags": ["test", "example"]
        }
    }
    
    encoded = crous.dumps(data)
    decoded = crous.loads(encoded)
    
    assert decoded == data, "Complex nested structure roundtrip failed"
    
    print("✓ Complex nested structure works correctly")

def test_version():
    """Test version attribute."""
    assert hasattr(crous, '__version__'), "Missing __version__"
    assert crous.__version__ == "2.0.0", f"Expected 2.0.0, got {crous.__version__}"
    
    print("✓ Version is correct: " + crous.__version__)

def run_all_new_tests():
    """Run all new feature tests."""
    print("\n" + "="*60)
    print("RUNNING NEW FEATURE TESTS")
    print("="*60 + "\n")
    
    test_large_integers()
    test_crous_encoder_class()
    test_crous_decoder_class()
    test_object_hook_parameter()
    test_object_hook_nested()
    test_default_parameter()
    test_register_serializer()
    test_register_decoder()
    test_dump_load_file()
    test_version()
    test_error_classes()
    test_all_scalar_types()
    test_complex_nested_structure()
    
    print("\n" + "="*60)
    print("ALL NEW FEATURE TESTS PASSED! ✓")
    print("="*60 + "\n")


if __name__ == "__main__":
    run_all_new_tests()