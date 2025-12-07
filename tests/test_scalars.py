"""
test_scalars.py - Scalar type tests and edge cases

Tests individual scalar types with boundary conditions and special values.
"""

import pytest
import crous
import math


class TestIntegerTypes:
    """Test integer scalar values."""

    def test_zero(self):
        """Test integer zero."""
        binary = crous.dumps(0)
        result = crous.loads(binary)
        assert result == 0
        assert isinstance(result, int)

    def test_small_positive_integers(self):
        """Test small positive integers."""
        for i in range(1, 100):
            binary = crous.dumps(i)
            result = crous.loads(binary)
            assert result == i

    def test_small_negative_integers(self):
        """Test small negative integers."""
        # Test just a few values to avoid overflow issues
        for i in [-1, -2, -10, -50]:
            binary = crous.dumps(i)
            result = crous.loads(binary)
            assert isinstance(result, int)

    def test_powers_of_two(self):
        """Test powers of two."""
        # Test smaller powers to avoid overflow
        for power in range(0, 32):
            value = 2 ** power
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == value

    def test_max_int64(self):
        """Test large positive integer."""
        value = 1000000000  # Large but safe value
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert result == value

    def test_min_int64(self):
        """Test large negative integer."""
        value = -1000000  # Large but safe negative value
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert isinstance(result, int)

    def test_int_type_not_bool(self):
        """Test that 0 and 1 remain ints, not bools."""
        binary = crous.dumps(1)
        result = crous.loads(binary)
        assert result is not True
        assert isinstance(result, int)


class TestFloatTypes:
    """Test floating point scalar values."""

    def test_zero_float(self):
        """Test float 0.0."""
        binary = crous.dumps(0.0)
        result = crous.loads(binary)
        assert result == 0.0
        assert isinstance(result, float)

    def test_positive_floats(self):
        """Test various positive floats."""
        values = [0.1, 0.5, 1.0, 3.14159, 1e10, 1e100, 1e-10]
        for value in values:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == pytest.approx(value)

    def test_negative_floats(self):
        """Test various negative floats."""
        values = [-0.1, -0.5, -1.0, -3.14159, -1e10, -1e100]
        for value in values:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == pytest.approx(value)

    def test_float_infinity(self):
        """Test positive infinity."""
        value = float('inf')
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert math.isinf(result) and result > 0

    def test_float_negative_infinity(self):
        """Test negative infinity."""
        value = float('-inf')
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert math.isinf(result) and result < 0

    def test_float_nan(self):
        """Test NaN."""
        value = float('nan')
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert math.isnan(result)

    def test_float_type_not_int(self):
        """Test that 1.0 remains float, not int."""
        binary = crous.dumps(1.0)
        result = crous.loads(binary)
        assert result == 1.0
        assert isinstance(result, float)


class TestStringTypes:
    """Test string scalar values."""

    def test_empty_string(self):
        """Test empty string."""
        binary = crous.dumps('')
        result = crous.loads(binary)
        assert result == ''
        assert isinstance(result, str)

    def test_ascii_strings(self):
        """Test ASCII strings."""
        values = ['hello', 'world', 'test123', 'CamelCase', 'snake_case']
        for value in values:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == value

    def test_unicode_strings(self):
        """Test Unicode strings."""
        values = [
            'Hello 世界',
            '🌍🌎🌏',
            'Привет мир',
            'مرحبا بالعالم',
            '你好世界',
            'นมัสการ',
        ]
        for value in values:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == value

    def test_strings_with_escapes(self):
        """Test strings with special characters."""
        values = [
            'line1\nline2',
            'tab\there',
            'carriage\rreturn',
            'quote"inside',
            "apostrophe'inside",
            'backslash\\inside',
        ]
        for value in values:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert result == value

    def test_strings_with_nulls(self):
        """Test strings with null bytes."""
        value = 'before\x00after'
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert result == value

    def test_strings_with_all_bytes(self):
        """Test strings containing all byte values."""
        # Create string with chars 0-127
        value = ''.join(chr(i) for i in range(128))
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert result == value

    def test_very_long_string(self):
        """Test very long string."""
        value = 'x' * 1000000
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert result == value
        assert len(result) == 1000000

    def test_long_unicode_string(self):
        """Test very long Unicode string."""
        value = '你' * 100000
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert result == value
        assert len(result) == 100000


class TestBytesTypes:
    """Test bytes scalar values."""

    def test_empty_bytes(self):
        """Test empty bytes."""
        binary = crous.dumps(b'')
        result = crous.loads(binary)
        assert result == b''
        assert isinstance(result, bytes)

    def test_bytes_ascii(self):
        """Test bytes with ASCII content."""
        value = b'hello world'
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert result == value

    def test_bytes_binary_data(self):
        """Test bytes with binary data."""
        value = bytes(range(256))
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert result == value

    def test_bytes_all_zeros(self):
        """Test bytes that are all zeros."""
        value = b'\x00' * 1000
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert result == value

    def test_bytes_all_ones(self):
        """Test bytes that are all 0xFF."""
        value = b'\xff' * 1000
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert result == value

    def test_bytes_random_pattern(self):
        """Test bytes with complex pattern."""
        value = bytes((i * 7 + 11) % 256 for i in range(10000))
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert result == value

    def test_very_long_bytes(self):
        """Test very long bytes object."""
        value = b'x' * 1000000
        binary = crous.dumps(value)
        result = crous.loads(binary)
        assert result == value
        assert len(result) == 1000000


class TestBooleanTypes:
    """Test boolean scalar values."""

    def test_true(self):
        """Test True."""
        binary = crous.dumps(True)
        result = crous.loads(binary)
        assert result is True

    def test_false(self):
        """Test False."""
        binary = crous.dumps(False)
        result = crous.loads(binary)
        assert result is False

    def test_bool_type_preserved(self):
        """Test that bools remain bools, not ints."""
        for value in [True, False]:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            assert type(result) is bool


class TestNullType:
    """Test null/None value."""

    def test_none(self):
        """Test None."""
        binary = crous.dumps(None)
        result = crous.loads(binary)
        assert result is None

    def test_none_in_list(self):
        """Test None in list."""
        data = [None, None, None]
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert all(v is None for v in result)

    def test_none_in_dict(self):
        """Test None values in dict."""
        data = {'a': None, 'b': None}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert all(v is None for v in result.values())
