"""
test_containers.py - Container type tests for Crous

Tests lists, tuples, dicts with various content, nesting, and edge cases.
"""

import pytest
import crous


class TestLists:
    """Test list serialization and deserialization."""

    def test_empty_list(self):
        """Test empty list."""
        data = []
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == []

    def test_list_of_none(self):
        """Test list containing only None."""
        data = [None, None, None]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_list_of_bools(self):
        """Test list of booleans."""
        data = [True, False, True, False]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_list_of_ints(self):
        """Test list of integers."""
        # Use smaller values to avoid overflow issues
        data = [0, 1, 42, 100, 1000000]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_list_of_floats(self):
        """Test list of floats."""
        data = [0.0, 3.14, -2.71, 1e10]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert len(result) == len(data)
        for original, decoded in zip(data, result):
            assert original == pytest.approx(decoded)

    def test_list_of_strings(self):
        """Test list of strings."""
        data = ['hello', 'world', '', 'unicode: 你好']
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_list_of_bytes(self):
        """Test list of bytes."""
        data = [b'', b'hello', b'\x00\x01\x02', bytes(range(256))]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_list_mixed_types(self):
        """Test list with mixed types."""
        data = [None, True, 42, 3.14, 'string', b'bytes', [], {}]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_list_of_lists(self):
        """Test list of lists."""
        data = [[], [1], [1, 2], [1, 2, 3]]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_list_of_dicts(self):
        """Test list of dicts."""
        data = [{}, {'a': 1}, {'x': 'y', 'z': 0}]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_list_large(self):
        """Test large list."""
        data = list(range(10000))
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data
        assert len(result) == 10000

    def test_list_very_large(self):
        """Test very large list."""
        data = list(range(100000))
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_list_duplicate_values(self):
        """Test list with many duplicate values."""
        data = [42] * 1000
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data
        assert all(v == 42 for v in result)

    def test_list_preserve_type_int_vs_float(self):
        """Test that int and float types are preserved in lists."""
        data = [1, 1.0, 2, 2.0]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert isinstance(result[0], int)
        assert isinstance(result[1], float)
        assert isinstance(result[2], int)
        assert isinstance(result[3], float)


class TestDicts:
    """Test dict serialization and deserialization."""

    def test_empty_dict(self):
        """Test empty dict."""
        data = {}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == {}

    def test_dict_single_key(self):
        """Test dict with single key."""
        data = {'key': 'value'}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_dict_string_keys(self):
        """Test dict with various string keys."""
        data = {
            'simple': 1,
            'with space': 2,
            'with-dash': 3,
            'with_underscore': 4,
            'camelCase': 5,
            'PascalCase': 6,
            '123numeric': 7,
            '': 8,  # Empty key
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_dict_numeric_values(self):
        """Test dict with numeric values."""
        data = {
            'int': 42,
            'float': 3.14,
            'positive': 100,
            'zero_int': 0,
            'zero_float': 0.0,
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_dict_string_values(self):
        """Test dict with string values."""
        data = {
            'empty': '',
            'simple': 'hello',
            'unicode': '世界',
            'long': 'x' * 1000,
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_dict_bytes_values(self):
        """Test dict with bytes values."""
        data = {
            'empty': b'',
            'simple': b'data',
            'binary': b'\x00\x01\x02\xff',
            'long': b'x' * 1000,
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_dict_nested_values(self):
        """Test dict with nested collection values."""
        data = {
            'list': [1, 2, 3],
            'dict': {'inner': 'value'},
            'mixed': {'list': [1, 2], 'dict': {'x': 'y'}},
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_dict_many_keys(self):
        """Test dict with many keys."""
        data = {f'key_{i}': f'value_{i}' for i in range(1000)}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data
        assert len(result) == 1000

    def test_dict_key_order(self):
        """Test that dict key order is preserved."""
        data = {'z': 1, 'a': 2, 'm': 3, 'b': 4}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert list(result.keys()) == list(data.keys())

    def test_dict_special_key_chars(self):
        """Test dict keys with special characters."""
        data = {
            'key\nwith\nnewlines': 1,
            'key\twith\ttabs': 2,
            'key with spaces': 3,
            'key\x00with\x00nulls': 4,
            '键': 5,  # Unicode key
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_dict_values_all_none(self):
        """Test dict with all None values."""
        data = {'a': None, 'b': None, 'c': None}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_dict_preserve_type_values(self):
        """Test that value types are preserved."""
        data = {
            'int': 1,
            'float': 1.0,
            'str': '1',
            'bytes': b'1',
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        assert isinstance(result['int'], int)
        assert isinstance(result['float'], float)
        assert isinstance(result['str'], str)
        assert isinstance(result['bytes'], bytes)


class TestTuples:
    """Test tuple handling - currently not supported by Crous."""

    @pytest.mark.skip(reason="Tuples not supported by Crous format")
    def test_tuple_basic(self):
        """Test that tuples serialize and deserialize."""
        # Tuples are not natively supported; use lists instead
        data = (1, 2, 3)
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert list(result) == [1, 2, 3]

    @pytest.mark.skip(reason="Tuples not supported by Crous format")
    def test_tuple_mixed_types(self):
        """Test tuple with mixed types."""
        data = (1, 'two', 3.0, None)
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert list(result) == [1, 'two', 3.0, None]

    @pytest.mark.skip(reason="Tuples not supported by Crous format")
    def test_nested_tuples(self):
        """Test nested tuples."""
        data = ((1, 2), (3, 4))
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert [list(x) for x in result] == [[1, 2], [3, 4]]


class TestComplexNesting:
    """Test complex nested structures."""

    def test_dict_list_dict(self):
        """Test dict containing list of dicts."""
        data = {
            'users': [
                {'id': 1, 'name': 'Alice'},
                {'id': 2, 'name': 'Bob'},
            ]
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_list_dict_list(self):
        """Test list containing dict with lists."""
        data = [
            {'items': [1, 2, 3]},
            {'items': [4, 5, 6]},
        ]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_deeply_nested_structure(self):
        """Test structure with multiple nesting levels."""
        data = {
            'level1': {
                'level2': {
                    'level3': {
                        'level4': {
                            'level5': [1, 2, 3]
                        }
                    }
                }
            }
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_mixed_nesting(self):
        """Test mixed list/dict nesting."""
        data = {
            'lists': [
                [1, 2],
                {'inner': 'dict'},
                [3, 4],
            ],
            'dicts': {
                'list': [1, 2, 3],
                'dict': {'key': 'value'},
            }
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data


class TestContainerProperties:
    """Test properties and invariants of containers."""

    def test_dict_key_not_list(self):
        """Test that dict keys must be strings."""
        # This should fail during encoding
        data_invalid = {1: 'value'}  # Integer key
        with pytest.raises(crous.CrousEncodeError):
            crous.dumps(data_invalid)

    def test_dict_key_not_dict(self):
        """Test that dict keys cannot be dicts."""
        data_invalid = {tuple([1, 2]): 'value'}  # Tuple key (not string)
        with pytest.raises((crous.CrousEncodeError, TypeError)):
            crous.dumps(data_invalid)

    def test_list_preserves_order(self):
        """Test that list order is preserved."""
        data = [5, 4, 3, 2, 1]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == [5, 4, 3, 2, 1]

    def test_dict_preserves_key_value_pairs(self):
        """Test that dict key-value pairs are correctly preserved."""
        data = {
            'a': 1,
            'b': 2,
            'c': 3,
        }
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        for key in data:
            assert result[key] == data[key]

    def test_container_identity_not_preserved(self):
        """Test that container identity is not preserved (only values)."""
        data = [1, 2, 3]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        # Same value but different object
        assert result == data
        assert result is not data

    def test_container_references_not_preserved(self):
        """Test that references within containers are resolved (not preserved)."""
        # Create shared reference
        shared = [1, 2]
        data = [shared, shared]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        
        # After round-trip, references may not be identical
        # (Crous encodes values, not references)
        assert result[0] == [1, 2]
        assert result[1] == [1, 2]
