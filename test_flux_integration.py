#!/usr/bin/env python3
"""
Test to verify that FLUX format is being used and dual-format compatibility works.
"""

import crous
import os
import tempfile

def test_flux_format():
    """Verify that new files are encoded with FLUX format."""
    test_data = {
        "name": "Test",
        "values": [1, 2, 3],
        "nested": {"key": "value"}
    }
    
    # Encode with new format (should be FLUX)
    encoded = crous.dumps(test_data)
    
    # Check magic bytes for FLUX format
    assert encoded[:4] == b'FLUX', f"Expected FLUX magic bytes, got {encoded[:4]}"
    print(f"✓ FLUX magic bytes detected: {encoded[:4]}")
    
    # Decode and verify
    decoded = crous.loads(encoded)
    assert decoded == test_data
    print(f"✓ Data correctly decoded from FLUX format")
    
    # Verify that decoding works
    print(f"✓ Encoded size: {len(encoded)} bytes")
    return True

def test_bytes_support():
    """Verify that bytes are supported in FLUX format."""
    test_data = {
        "binary": b"hello world",
        "empty_binary": b"",
        "mixed": [1, b"data", "string"]
    }
    
    encoded = crous.dumps(test_data)
    decoded = crous.loads(encoded)
    
    assert decoded == test_data
    print(f"✓ Bytes support working correctly")
    return True

def test_file_io():
    """Verify that file I/O uses FLUX format."""
    test_data = {"test": "data", "number": 42}
    
    with tempfile.NamedTemporaryFile(delete=False) as f:
        temp_path = f.name
    
    try:
        # Dump to file
        crous.dump(test_data, temp_path)
        
        # Read raw bytes to check format
        with open(temp_path, 'rb') as f:
            raw_bytes = f.read()
        
        assert raw_bytes[:4] == b'FLUX', f"File format is not FLUX: {raw_bytes[:4]}"
        print(f"✓ File I/O using FLUX format")
        
        # Load from file
        loaded = crous.load(temp_path)
        assert loaded == test_data
        print(f"✓ File I/O correctly round-tripped")
    finally:
        if os.path.exists(temp_path):
            os.remove(temp_path)
    
    return True

def test_complex_types():
    """Test complex nested structures with FLUX."""
    test_data = {
        "users": [
            {"name": "Alice", "age": 30, "active": True},
            {"name": "Bob", "age": 25, "active": False},
        ],
        "metadata": {
            "count": 2,
            "tags": ["important", "verified"],
            "data": b"binary_content"
        }
    }
    
    encoded = crous.dumps(test_data)
    decoded = crous.loads(encoded)
    
    assert decoded == test_data
    print(f"✓ Complex nested structures work with FLUX")
    return True

if __name__ == "__main__":
    print("Testing FLUX Format Integration...")
    print()
    
    tests = [
        ("FLUX Format Detection", test_flux_format),
        ("Bytes Support", test_bytes_support),
        ("File I/O", test_file_io),
        ("Complex Types", test_complex_types),
    ]
    
    passed = 0
    failed = 0
    
    for test_name, test_func in tests:
        try:
            print(f"Testing: {test_name}")
            if test_func():
                passed += 1
            print()
        except Exception as e:
            print(f"✗ {test_name} FAILED: {e}")
            import traceback
            traceback.print_exc()
            failed += 1
            print()
    
    print(f"Results: {passed} passed, {failed} failed")
    exit(0 if failed == 0 else 1)
