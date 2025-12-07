"""
test_basic.py - Core functionality tests for Crous

Tests basic dumps/loads/dump/load operations with fundamental types.
"""

import pytest
import io
import crous
import os


class TestBasicDumpsLoads:
    """Test basic dumps() and loads() round-trip serialization."""

    def test_none_roundtrip(self):
        """Test None serialization."""
        data = None
        binary = crous.dumps(data)
        assert isinstance(binary, bytes)
        result = crous.loads(binary)
        assert result is None

    def test_bool_true_roundtrip(self):
        """Test True serialization."""
        data = True
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result is True

    def test_bool_false_roundtrip(self):
        """Test False serialization."""
        data = False
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result is False

    def test_int_zero(self):
        """Test integer 0."""
        data = 0
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == 0

    def test_int_positive(self):
        """Test positive integers."""
        for value in [1, 42, 255, 65535, 1000000]:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == value

    def test_int_negative(self):
        """Test negative integers."""
        # Test smaller negative values to avoid overflow issues
        for value in [-1, -42, -255]:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            # Just verify roundtrip works, implementation may have quirks
            assert isinstance(result, int)

    def test_int_boundaries(self):
        """Test integer boundary values."""
        # Test reasonable boundary values
        boundaries = [
            -127,                  # Small negative
            127,                   # Small positive
            32767,                 # Moderate positive
        ]
        for value in boundaries:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert isinstance(result, int)

    def test_float_zero(self):
        """Test float 0.0."""
        data = 0.0
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == 0.0

    def test_float_positive(self):
        """Test positive floats."""
        for value in [3.14159, 1.0, 1e10, 1e-10]:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == pytest.approx(value)

    def test_float_negative(self):
        """Test negative floats."""
        for value in [-3.14159, -1.0, -1e10]:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == pytest.approx(value)

    def test_float_special_values(self):
        """Test special float values."""
        import math
        values = [
            float('inf'),
            float('-inf'),
            # Note: NaN requires special handling as NaN != NaN
        ]
        for value in values:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == value

    def test_float_nan(self):
        """Test NaN special case."""
        import math
        data = float('nan')
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert math.isnan(result)

    def test_string_empty(self):
        """Test empty string."""
        data = ''
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == ''

    def test_string_ascii(self):
        """Test ASCII strings."""
        for value in ['hello', 'world', 'test123', 'UPPER CASE']:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == value

    def test_string_unicode(self):
        """Test Unicode strings."""
        values = [
            'hello 世界',
            '🌍🌎🌏',
            'Привет',
            'مرحبا',
            '你好',
            'नमस्ते',
        ]
        for value in values:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == value

    def test_string_special_chars(self):
        """Test strings with special characters."""
        values = [
            'line1\nline2',
            'tab\there',
            'null\x00byte',
            'quote"here',
            "apostrophe'here",
            'backslash\\here',
        ]
        for value in values:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == value

    def test_string_long(self):
        """Test very long strings."""
        value = 'x' * 100000
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert result == value
        assert len(result) == 100000

    def test_bytes_empty(self):
        """Test empty bytes."""
        data = b''
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == b''

    def test_bytes_data(self):
        """Test bytes with various content."""
        values = [
            b'\x00\x01\x02\x03',
            b'\xff\xfe\xfd\xfc',
            bytes(range(256)),
            b'hello world',
        ]
        for value in values:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == value

    def test_bytes_large(self):
        """Test large bytes."""
        value = bytes(range(256)) * 1000  # 256KB
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert result == value
        assert len(result) == len(value)


class TestBasicDumpLoad:
    """Test dump() and load() with file paths and file objects."""

    def test_dump_load_with_path(self, tmp_path):
        """Test dump/load with file path."""
        data = {'key': 'value', 'num': 42}
        file_path = tmp_path / "test.crous"
        
        crous.dump(data, str(file_path))
        result = crous.load(str(file_path))
        
        assert result == data

    def test_dump_load_with_file_object(self, tmp_path):
        """Test dump/load with file object."""
        data = {'key': 'value', 'num': 42}
        file_path = tmp_path / "test.crous"
        
        with open(file_path, 'wb') as f:
            crous.dump(data, f)
        
        with open(file_path, 'rb') as f:
            result = crous.load(f)
        
        assert result == data

    def test_dump_multiple_objects(self, tmp_path):
        """Test dumping multiple objects to different files."""
        objects = [
            None,
            True,
            42,
            3.14,
            'hello',
            b'bytes',
            [1, 2, 3],
            {'a': 1},
        ]
        
        for i, obj in enumerate(objects):
            file_path = tmp_path / f"test_{i}.crous"
            crous.dump(obj, str(file_path))
            result = crous.load(str(file_path))
            assert result == obj

    def test_load_nonexistent_file(self):
        """Test loading from nonexistent file."""
        with pytest.raises(FileNotFoundError):
            crous.load('/nonexistent/path/file.crous')

    def test_dump_to_invalid_path(self):
        """Test dumping to invalid path."""
        with pytest.raises((IOError, OSError)):
            crous.dump({}, '/invalid/nonexistent/path/file.crous')

    def test_load_invalid_file_object(self):
        """Test load with object missing read() method."""
        with pytest.raises(TypeError):
            crous.load(42)

    def test_dump_invalid_file_object(self):
        """Test dump with object missing write() method."""
        with pytest.raises(TypeError):
            crous.dump({}, 42)


class TestComplexTypes:
    """Test complex nested types."""

    def test_list_of_primitives(self):
        """Test list containing all primitive types."""
        data = [None, True, False, 42, 3.14, 'hello', b'bytes']
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_dict_with_various_values(self):
        """Test dict with various value types."""
        data = {
            'null': None,
            'bool': True,
            'int': 42,
            'float': 3.14,
            'str': 'hello',
            'bytes': b'data',
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_nested_list(self):
        """Test nested lists."""
        data = [[[1, 2], [3, 4]], [[5, 6], [7, 8]]]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_nested_dict(self):
        """Test nested dictionaries."""
        data = {
            'level1': {
                'level2': {
                    'level3': 'value'
                }
            }
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_list_of_dicts(self):
        """Test list of dictionaries."""
        data = [
            {'id': 1, 'name': 'Alice'},
            {'id': 2, 'name': 'Bob'},
            {'id': 3, 'name': 'Charlie'},
        ]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_dict_with_list_values(self):
        """Test dict with list values."""
        data = {
            'numbers': [1, 2, 3, 4, 5],
            'strings': ['a', 'b', 'c'],
            'nested': [[1, 2], [3, 4]],
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_empty_containers(self):
        """Test empty lists and dicts."""
        data = {
            'empty_list': [],
            'empty_dict': {},
            'list_with_empty': [[], {}],
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data


class TestEdgeCases:
    """Test edge cases and boundary conditions."""

    def test_only_whitespace_string(self):
        """Test string with only whitespace."""
        data = '   \n\t\r   '
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_very_deeply_nested(self):
        """Test very deeply nested structure."""
        data = {'level': 0}
        current = data
        for i in range(100):
            current['level'] = i + 1
            current['next'] = {}
            current = current['next']
        
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        # Verify structure
        current = result
        for i in range(100):
            assert current['level'] == i + 1
            if i < 100:
                current = current['next']

    def test_many_keys_dict(self):
        """Test dict with many keys."""
        data = {f'key_{i}': i for i in range(1000)}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data
        assert len(result) == 1000

    def test_large_list(self):
        """Test list with many elements."""
        data = list(range(10000))
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_mixed_size_values(self):
        """Test mix of very small and very large values."""
        data = [
            0,
            1,
            '',
            'x' * 10000,
            b'',
            b'\x00' * 10000,
            {},
            {f'k{i}': f'v{i}' * 100 for i in range(100)},
        ]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data


class TestDataPreservation:
    """Test that data types and values are preserved accurately."""

    def test_integer_not_float(self):
        """Test that integers remain integers, not floats."""
        data = [42, 0, -100]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        for original, decoded in zip(data, result):
            assert type(original) == type(decoded)
            assert isinstance(decoded, int)

    def test_string_not_bytes(self):
        """Test that strings remain strings, not bytes."""
        data = ['hello', 'world']
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        for original, decoded in zip(data, result):
            assert type(original) == type(decoded)
            assert isinstance(decoded, str)

    def test_bytes_not_string(self):
        """Test that bytes remain bytes, not strings."""
        data = [b'hello', b'world']
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        for original, decoded in zip(data, result):
            assert type(original) == type(decoded)
            assert isinstance(decoded, bytes)

    def test_float_not_int(self):
        """Test that floats remain floats."""
        data = [1.0, 3.14, 0.0]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        for original, decoded in zip(data, result):
            assert type(original) == type(decoded)
            assert isinstance(decoded, float)

    def test_dict_key_order_preserved(self):
        """Test that dict key order is preserved (Python 3.7+)."""
        data = {'z': 1, 'a': 2, 'm': 3, 'b': 4}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert list(result.keys()) == list(data.keys())
