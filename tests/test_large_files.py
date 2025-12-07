"""
test_large_files.py - Large file and memory tests

Tests handling of large data structures and file I/O.
"""

import pytest
import crous
import io
import tempfile
import os


class TestLargeFileSerialization:
    """Test serialization of large files."""

    def test_large_dict_to_file(self, tmp_path):
        """Test dumping large dict to file."""
        data = {f'key_{i}': f'value_{i}' * 100 for i in range(10000)}
        file_path = tmp_path / "large.crous"
        
        crous.dump(data, str(file_path))
        result = crous.load(str(file_path))
        
        assert result == data

    def test_large_list_to_file(self, tmp_path):
        """Test dumping large list to file."""
        data = list(range(100000))
        file_path = tmp_path / "large_list.crous"
        
        crous.dump(data, str(file_path))
        result = crous.load(str(file_path))
        
        assert result == data

    def test_large_bytes_to_file(self, tmp_path):
        """Test dumping large bytes object to file."""
        data = b'x' * 10000000  # 10MB
        file_path = tmp_path / "large_bytes.crous"
        
        crous.dump(data, str(file_path))
        result = crous.load(str(file_path))
        
        assert result == data
        assert len(result) == 10000000

    def test_large_string_to_file(self, tmp_path):
        """Test dumping large string to file."""
        data = 'x' * 10000000  # 10MB
        file_path = tmp_path / "large_string.crous"
        
        crous.dump(data, str(file_path))
        result = crous.load(str(file_path))
        
        assert result == data

    def test_complex_large_structure(self, tmp_path):
        """Test complex large nested structure."""
        data = {
            'users': [
                {
                    'id': i,
                    'name': f'user_{i}',
                    'data': 'x' * 1000,
                    'items': list(range(100)),
                }
                for i in range(100)
            ]
        }
        file_path = tmp_path / "complex.crous"
        
        crous.dump(data, str(file_path))
        result = crous.load(str(file_path))
        
        assert result == data


class TestMemoryEfficiency:
    """Test memory-efficient handling of large data."""

    def test_repeated_load_no_leak(self, tmp_path):
        """Test repeated load cycles don't accumulate memory."""
        data = {'key': 'value' * 1000}
        file_path = tmp_path / "test.crous"
        crous.dump(data, str(file_path))
        
        # Repeat loading many times
        for _ in range(100):
            result = crous.load(str(file_path))
            assert result == data

    def test_repeated_dumps_no_leak(self):
        """Test repeated dumps don't accumulate memory."""
        data = {'key': 'value' * 1000}
        
        for _ in range(100):
            binary = crous.dumps(data)
            result = crous.loads(binary)
            assert result == data
