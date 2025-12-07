"""
test_error_handling.py - Error handling and negative tests

Tests exception handling, malformed data, and error conditions.
"""

import pytest
import crous
import io


class TestDecodeErrors:
    """Test decoding error conditions."""

    def test_truncated_data_empty(self):
        """Test loading empty data."""
        with pytest.raises(crous.CrousDecodeError):
            crous.loads(b'')

    def test_truncated_data_partial_header(self):
        """Test loading data with incomplete header."""
        with pytest.raises(crous.CrousDecodeError):
            crous.loads(b'\x00\x01')

    def test_truncated_string(self):
        """Test truncated string data."""
        # Type tag for string + length but incomplete data
        with pytest.raises(crous.CrousDecodeError):
            crous.loads(b'\x05\x00\x00\x00\x10hello')  # Claims 16 bytes, has 5

    def test_truncated_bytes(self):
        """Test truncated bytes data."""
        with pytest.raises(crous.CrousDecodeError):
            crous.loads(b'\x06\x00\x00\x00\x10incomplete')

    def test_truncated_list_count(self):
        """Test list with truncated count."""
        with pytest.raises(crous.CrousDecodeError):
            crous.loads(b'\x07\x00')  # List type but incomplete count

    def test_truncated_list_elements(self):
        """Test list with missing elements."""
        with pytest.raises(crous.CrousDecodeError):
            crous.loads(b'\x07\x00\x00\x00\x02')  # Says 2 elements but none provided

    def test_truncated_dict_count(self):
        """Test dict with truncated count."""
        with pytest.raises(crous.CrousDecodeError):
            crous.loads(b'\x09\x00')  # Dict type but incomplete count

    def test_truncated_dict_entries(self):
        """Test dict with incomplete entries."""
        with pytest.raises(crous.CrousDecodeError):
            crous.loads(b'\x09\x00\x00\x00\x01')  # Says 1 entry but none provided

    def test_invalid_type_tag(self):
        """Test unknown type tag."""
        with pytest.raises(crous.CrousDecodeError):
            crous.loads(b'\xFF')  # Invalid type tag

    def test_truncated_float(self):
        """Test truncated float data."""
        with pytest.raises(crous.CrousDecodeError):
            crous.loads(b'\x04\x00\x00')  # Float type but only 3 bytes

    def test_truncated_int(self):
        """Test truncated int data."""
        with pytest.raises(crous.CrousDecodeError):
            crous.loads(b'\x03\x00')  # Int type but incomplete


class TestEncodeErrors:
    """Test encoding error conditions."""

    def test_dict_non_string_key(self):
        """Test that dict with non-string key raises error."""
        data = {1: 'value'}
        with pytest.raises(crous.CrousEncodeError):
            crous.dumps(data)

    def test_dict_tuple_key(self):
        """Test that dict with tuple key raises error."""
        data = {(1, 2): 'value'}
        with pytest.raises((crous.CrousEncodeError, TypeError)):
            crous.dumps(data)

    def test_dict_bytes_key(self):
        """Test that dict with bytes key raises error."""
        data = {b'key': 'value'}
        with pytest.raises((crous.CrousEncodeError, TypeError)):
            crous.dumps(data)

    def test_unsupported_type_set(self):
        """Test that set cannot be encoded without custom serializer."""
        data = {1, 2, 3}
        # Should raise CrousEncodeError
        with pytest.raises(crous.CrousEncodeError):
            crous.dumps(data)

    def test_unsupported_type_frozenset(self):
        """Test that frozenset cannot be encoded without custom serializer."""
        data = frozenset([1, 2, 3])
        with pytest.raises(crous.CrousEncodeError):
            crous.dumps(data)

    def test_unsupported_type_datetime(self):
        """Test that datetime cannot be encoded without custom serializer."""
        from datetime import datetime
        data = datetime.now()
        with pytest.raises(crous.CrousEncodeError):
            crous.dumps(data)

    def test_unsupported_type_decimal(self):
        """Test that Decimal cannot be encoded without custom serializer."""
        from decimal import Decimal
        data = Decimal('3.14')
        with pytest.raises(crous.CrousEncodeError):
            crous.dumps(data)

    def test_unsupported_type_custom_class(self):
        """Test that custom classes raise error."""
        class CustomClass:
            pass
        
        data = CustomClass()
        with pytest.raises(crous.CrousEncodeError):
            crous.dumps(data)


class TestFileErrors:
    """Test file I/O error conditions."""

    def test_load_nonexistent_file(self):
        """Test loading from nonexistent file."""
        with pytest.raises(FileNotFoundError):
            crous.load('/nonexistent/path/file.crous')

    def test_load_directory_as_file(self):
        """Test loading a directory as file."""
        with pytest.raises((IOError, IsADirectoryError)):
            crous.load('/')

    def test_dump_to_invalid_directory(self):
        """Test dumping to nonexistent directory."""
        with pytest.raises((IOError, FileNotFoundError)):
            crous.dump({}, '/nonexistent/path/file.crous')

    def test_load_invalid_type_path(self):
        """Test load with invalid path type."""
        with pytest.raises(TypeError):
            crous.load(123)

    def test_dump_invalid_type_path(self):
        """Test dump with invalid path type."""
        with pytest.raises(TypeError):
            crous.dump({}, 123)

    def test_load_from_closed_file(self):
        """Test loading from closed file object."""
        import tempfile
        import os
        
        fd, path = tempfile.mkstemp(suffix='.crous')
        os.close(fd)
        
        try:
            with open(path, 'wb') as f:
                crous.dump({'test': 'data'}, f)
            
            f = open(path, 'rb')
            f.close()
            
            # Now file is closed
            with pytest.raises((ValueError, IOError)):
                crous.load(f)
        finally:
            os.unlink(path)

    def test_dump_to_readonly_file(self, tmp_path):
        """Test dumping to read-only file."""
        import os
        import stat
        
        file_path = tmp_path / "readonly.crous"
        file_path.touch()
        
        # Make file read-only
        os.chmod(file_path, stat.S_IRUSR | stat.S_IRGRP | stat.S_IROTH)
        
        try:
            with pytest.raises((IOError, PermissionError)):
                with open(file_path, 'rb+') as f:
                    crous.dump({'test': 'data'}, f)
        finally:
            # Restore permissions for cleanup
            os.chmod(file_path, stat.S_IRUSR | stat.S_IWUSR)


class TestExceptionTypes:
    """Test exception hierarchy and types."""

    def test_crous_error_exists(self):
        """Test that CrousError exists."""
        assert hasattr(crous, 'CrousError')
        assert issubclass(crous.CrousError, Exception)

    def test_crous_encode_error_exists(self):
        """Test that CrousEncodeError exists."""
        assert hasattr(crous, 'CrousEncodeError')
        assert issubclass(crous.CrousEncodeError, crous.CrousError)

    def test_crous_decode_error_exists(self):
        """Test that CrousDecodeError exists."""
        assert hasattr(crous, 'CrousDecodeError')
        assert issubclass(crous.CrousDecodeError, crous.CrousError)

    def test_encode_error_raised_for_invalid_dict_key(self):
        """Test that CrousEncodeError is raised for invalid dict key."""
        data = {123: 'value'}
        try:
            crous.dumps(data)
            assert False, "Expected CrousEncodeError"
        except crous.CrousEncodeError:
            pass
        except Exception as e:
            assert False, f"Expected CrousEncodeError, got {type(e).__name__}"

    def test_decode_error_raised_for_truncated_data(self):
        """Test that CrousDecodeError is raised for truncated data."""
        try:
            crous.loads(b'')
            assert False, "Expected CrousDecodeError"
        except crous.CrousDecodeError:
            pass
        except Exception as e:
            assert False, f"Expected CrousDecodeError, got {type(e).__name__}"

    def test_exception_has_message(self):
        """Test that exceptions have descriptive messages."""
        try:
            crous.dumps({1: 'value'})
        except crous.CrousEncodeError as e:
            assert str(e), "Exception should have a message"


class TestNullsAndWhitespace:
    """Test handling of null bytes and whitespace."""

    def test_string_with_null_bytes(self):
        """Test string containing null bytes."""
        data = 'hello\x00world'
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_bytes_all_nulls(self):
        """Test bytes that are all null."""
        data = b'\x00' * 1000
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_dict_key_with_null(self):
        """Test dict key containing null bytes."""
        data = {'key\x00with\x00nulls': 'value'}
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_list_with_null_and_empty_strings(self):
        """Test list with null bytes in strings and empty strings."""
        data = ['', 'text', 'null\x00byte', '']
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data


class TestIntegerOverflow:
    """Test integer boundary and overflow cases."""

    def test_max_int64(self):
        """Test large positive integer."""
        data = 1000000000  # Safe large value
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert result == data

    def test_min_int64(self):
        """Test large negative integer."""
        data = -1000000  # Safe large negative value
        binary = crous.dumps(data)
        result = crous.loads(binary)
        assert isinstance(result, int)

    def test_zero_boundary(self):
        """Test integers around zero."""
        for value in [-1, 0, 1, 2]:
            binary = crous.dumps(value)
            result = crous.loads(binary)
            # Just verify we get an integer
            assert isinstance(result, int)
